#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <bitset>
#include <malloc.h>
#include <poll.h>
#include "node.h"

#define SIZE 48
#define NUM_OF_THREADS 8

int num = 0;
pthread_t thread_init;
//int global_step = 0;
char map[2][SIZE][SIZE];
int was_printed = 0;
int was_initialized = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER,
                mutex_init = PTHREAD_MUTEX_INITIALIZER,
                mutex_nodes_vector = PTHREAD_MUTEX_INITIALIZER;
int temp[NUM_OF_THREADS];
pthread_barrier_t barrier_distribute_tasks,
                  barrier_life_game_begin_step,
                  barrier_life_game_end_step;
std::vector<Node> nodes;
std::vector <std::pair <int, int> > tasks_for_nodes;


void print_map(int step)
{
    system("clear");
    printf("GENERETION #%d\n", step);
    int i, j;
    for (i = 0; i < SIZE; i++){
        for (j = 0; j < SIZE; j++)
            printf("%s ", map[(step & 1) ^ 1][i][j] ? "■" : ".");
        printf("\n");
    }
}


char * get_row_data(int step, int r, int field_size)
{
    if (r < 0)
        r += field_size;
    if (r >= field_size)
        r -= field_size;
    return map[step & 1][r];
}

void send_rows_to_node(int step, int r_first, int r_last, Node &node, int field_size)
{
    int pieces_amount = 0;
    for (int i = r_first; i < r_last; ++i) {
        char * row = get_row_data(step, i, field_size);
        if (node.is_data_to_send_empty()) {
            fprintf(stderr, "WARN! data_to_send was not empty\n");
            node.reset_data();
        }
        for (int column = 0; column < field_size; ) {
            int rest = node.get_rest_data_size() - 4 * sizeof(int);
            if (column + rest > field_size)
                rest = field_size - column;
            node.push_data_int(step);  // step
            node.push_data_int(i - r_first);  // row 0..n
            node.push_data_int(column);       // column 0..field_size
            node.push_data_int(rest);  // row length
            node.push_data_str(row + column, rest);
            node.send_pushed_data();
            ++pieces_amount;
            column += rest;
        }
    }
    node.push_data_str(PHRASE_NODE_DONE_STEP, -1);
    node.push_data_int(pieces_amount);    // pieces of data transmitted
    node.push_data_int(field_size);       // width of field
    node.push_data_int(r_last - r_first); // rows amount
    node.send_pushed_data();
}

bool all_completed(const std::vector<bool> &mask)
{
    for (auto node_completed : mask) {
        if (node_completed == 0) {
            return false;
        }
    }
    return true;
}

void read_data_from_node(int &pieces_amount, int &received, int step, int node_num)
{
    Node &node = nodes[node_num];
    char * data = nullptr;
    int size = node.get_data(data, 0);
    int phrase_len = strlen(PHRASE_NODE_DONE_STEP);
    if (size >= phrase_len + (int)sizeof(int) &&
        strncmp(data, PHRASE_NODE_DONE_STEP, phrase_len) == 0) {
        char * tmp = deserialize_int(&pieces_amount, data + phrase_len);
        int rows_amount;
        deserialize_int(&rows_amount, tmp);
        if (rows_amount != tasks_for_nodes[node_num].second - tasks_for_nodes[node_num].first) {
            fprintf(stderr, "WARN! received rows amount not same as server");
        }
        return;
    }
    int dst_step, row, column, row_length;
    char * buf = deserialize_int(&dst_step, data);
    buf = deserialize_int(&row, buf);
    buf = deserialize_int(&column, buf);
    buf = deserialize_int(&row_length, buf);
    if (dst_step != step) {
        fprintf(stderr, "WARN! dst_step not the same as server step; skipped");
    }
    else if (4 * (int)(sizeof(int)) + row_length > size) {
        fprintf(stderr, "received msg: truncated length\n");
    } else {
        memcpy(&map[(step & 1) ^ 1][row + tasks_for_nodes[node_num].first][column], buf, row_length);
        ++received;
    }
    free(data);
}

int server(int * argv) {
    int port = argv[0];
    int field_size = argv[1];
    int steps_amount = argv[2];
    char buffer[BUFFER_SIZE];
    struct sockaddr_in my_address;      /* our address */
    int fd;                             /* our socket */

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return fd;
    }

    memset((char *)&my_address, 0, sizeof(my_address));
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(port);

    /* bind the socket to any valid IP address and a specific port */
    if (bind(fd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
        perror("bind failed");
        return -1;
    }
    pollfd pollfd_server;
    pollfd_server.events = POLLIN;
    pollfd_server.fd = fd;
    printf("Waiting clients\n");
    poll(&pollfd_server, 1, -1);  /* waiting for first connect */
    for (int step = 0; step < steps_amount; ++step) {
        poll(&pollfd_server, 1, 0);
        while (pollfd_server.revents & POLLIN) {
            struct sockaddr_in remote;
            socklen_t remote_address_len;
            recvfrom(fd, buffer, BUFFER_SIZE, 0, (sockaddr *)&remote, &remote_address_len);
            nodes.push_back(Node(remote));
            poll(&pollfd_server, 1, 0);
        }
        int rest = field_size % (int) nodes.size();
        int row_to_each_node = field_size / (int) nodes.size();
        tasks_for_nodes.resize(nodes.size());
        for (int i = 0, last = 0; i < (int) nodes.size(); ++i) {
            int rows = row_to_each_node + (rest < i);
            tasks_for_nodes[i] = std::make_pair(last, last + rows);
            send_rows_to_node(step, last - 1, last + rows + 1, nodes[i], field_size);
        }
        std::vector<bool> mask_completed_nodes(nodes.size());
        std::vector<pollfd> socket_descriptors(nodes.size());
        for (int i = 0; i < (int)nodes.size(); ++i) {
            socket_descriptors[i].fd = nodes[i].get_socket_fd();
            socket_descriptors[i].events = POLLIN;
        }
        std::vector<int> expected_pieces_amount(nodes.size(), -1);
        std::vector<int> received_pieces_amount(nodes.size(), 0);
        while (!all_completed(mask_completed_nodes)) {
            int status = poll(socket_descriptors.data(), socket_descriptors.size(), TIMEOUT_WAIT_NODES_MILLISECONDS);
            if (status < 0) {
                perror("poll nodes");
                continue;
            }
            if (status == 0) {
                fprintf(stderr, "TIMEOUT NODES CONNECTION");
                --step;
                break;
            }
            for (int i = 0; i < (int)nodes.size(); ++i) {
                if (!mask_completed_nodes[i] && (socket_descriptors[i].revents & POLLIN)) {
                    read_data_from_node(expected_pieces_amount[i], received_pieces_amount[i], step, i);
                    if (received_pieces_amount[i] == expected_pieces_amount[i]) {
                        mask_completed_nodes[i] = 1;
                    }
                }
            }
        }
    }
    return 0;
}



int main(int argc, char ** argv) {

    int port = DEFAULT_PORT,
        field_size = DEFAULT_FIELD_SIZE,
        steps_amount = DEFAULT_STEPS_AMOUNT;
    if (argc != 4) {
        printf("wrong params: using:\n");
        printf("./life_game_server port field_size steps_amount\n");
        return 0;
    }
    if (argc == 4) {
        port = std::stoi(argv[1]);
        field_size = std::stoi(argv[2]);
        steps_amount = std::stoi(argv[3]);
    }
    int argv_parsed[] = {port, field_size, steps_amount};

    printf("using params: port=%d, filed_size=%dx%d generations_amount=%d\n",
           port, field_size, field_size, steps_amount);

    return server(argv_parsed);
}