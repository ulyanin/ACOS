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
#include "life_game_common.h"
#include "node.h"
#include "poll.h"

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

int add(int x)
{
    if (x == SIZE - 1)
        return 0;
    else
        return x + 1;
}

int sub(int x)
{
    if (x == 0)
        return SIZE - 1;
    else
        return x - 1;
}

int alive(int step, int x, int y)
{
    int al = 0;
    if (map[step][sub(x)][sub(y)] == 1)
        ++al;
    if (map[step][sub(x)][y] == 1)
        ++al;
    if (map[step][sub(x)][add(y)] == 1)
        ++al;
    if (map[step][x][sub(y)] == 1)
        ++al;
    if (map[step][x][add(y)] == 1)
        ++al;
    if (map[step][add(x)][sub(y)] == 1)
        ++al;
    if (map[step][add(x)][y] == 1)
        ++al;
    if (map[step][add(x)][add(y)] == 1)
        ++al;
    return al;
}

void recalc(int step, int x, int y)
{
    if (map[step][x][y] == 0)
    {
        if (alive(step, x, y) == 3)
            map[1 ^ step][x][y] = 1;
        else
            map[1 ^ step][x][y] = 0;
    }
    if (map[step][x][y] == 1)
    {
        if (alive(step, x, y) == 3 || alive(step, x, y) == 2)
            map[1 ^ step][x][y] = 1;
        else
            map[1 ^ step][x][y] = 0;
    }
}

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

int check_if_game_over()
{
    return 0;
}

void try_print_map(int step)
{
    pthread_mutex_lock(&lock);
    if (!was_printed) {
        was_initialized = 0;
        print_map(step);
        was_printed = 1;
    }
    pthread_mutex_unlock(&lock);
}

void initizlize_variables()
{
    pthread_mutex_lock(&mutex_init);
    if (!was_initialized) {
        was_printed = 0;
        was_initialized = 1;
    }
    pthread_mutex_unlock(&mutex_init);
}


//void* life_1(void* arg)
//{
//    int life_game_step;
//    for (life_game_step = 0; ;life_game_step++) {
//        pthread_barrier_wait(&barrier_distribute_tasks);
//        pthread_barrier_wait(&barrier_life_game_begin_step);
//        int i, j;
//        int id = *(int*)arg;
//        for (i = id; i < id + (SIZE / NUM_OF_THREADS); i++) {
//            for (j = 0; j < SIZE; j++)
//            {
//                recalc(life_game_step & 1, i, j);
//                //printf("%d\n",     end[(id * NUM_OF_THREADS) / SIZE]);
//            }
//        }
//        pthread_barrier_wait(&barrier_life_game_step);
//        try_print_map(life_game_step);
//        if (0 & check_if_game_over()) {
//            printf("GAME OVER DETECTED\n");
//            return NULL;
//        }
//        pthread_barrier_wait(&barrier_life_game_end_step);
//        usleep(10000);
//    }
//    return NULL;
//}

//void * server_thread(void * argv) {
//    int port = ((int *)(argv))[0];
//    int field_size = ((int *)(argv))[0];
//    int steps_amount = ((int *)(argv))[0];
//    struct sockaddr_in my_address;                  /* our address */
//    struct sockaddr_in remote_address;              /* remote address */
//    socklen_t address_len = sizeof(remote_address); /* length of addresses */
//    ssize_t recvlen;                                /* # bytes received */
//    int fd;                                         /* our socket */
//    char buf[BUFFER_SIZE];                          /* receive buffer */
//
//    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//        perror("cannot create socket\n");
//        return 0;
//    }
//
//    memset((char *)&my_address, 0, sizeof(my_address));
//    my_address.sin_family = AF_INET;
//    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
//    my_address.sin_port = htons(port);
//
//    /* bind the socket to any valid IP address and a specific port */
//    if (bind(fd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
//        perror("bind failed");
//        return nullptr;
//    }
//    pthread_barrier_init(&barrier_distribute_tasks, NULL, 0);
//    pthread_barrier_init(&barrier_life_game_begin_step, NULL, 0);
//    pthread_barrier_init(&barrier_life_game_end_step, NULL, 0);
//    for (int step = 0; step < steps_amount; ++step) {
//        pthread_barrier_wait(&barrier_distribute_tasks);
//        std::vector <Node> new_nodes;
//        int rest = field_size % (int)nodes.size();
//        int row_to_each_node = field_size / (int)nodes.size();
//        tasks_for_nodes.resize(nodes.size());
//        for (int i = 0, last = 0; i < (int)nodes.size(); ++i) {
//            int rows = row_to_each_node + (rest < i);
//            tasks_for_nodes[i] = std::make_pair(last, last + rows);
//        }
//        pthread_barrier_wait(&barrier_life_game_begin_step);
//
//        pthread_barrier_wait(&barrier_life_game_end_step);
//        pthread_barrier_wait(&barrier_register_nodes);
//}

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
            node.push_data_int(column);
            node.push_data_int(rest);  // row length
            node.push_data_str(row + column, rest);
            node.send_pushed_data();
            ++pieces_amount;
            column += rest;
        }
    }
    node.push_data_str(PHRASE_NODE_DONE_STEP, -1);
    node.push_data_int(pieces_amount);
    node.push_data_int(field_size);
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
    if (size >= strlen(PHRASE_NODE_DONE_STEP) + sizeof(int) &&
            strncmp(data, PHRASE_NODE_DONE_STEP, strlen(PHRASE_NODE_DONE_STEP)) == 0) {
        pieces_amount = *(int *)(data + strlen(PHRASE_NODE_DONE_STEP));
        return;
    }
    int dst_step, row, column, row_length;
    char * buf = deserialize_int(&dst_step, data);
    buf = deserialize_int(&row, buf);
    buf = deserialize_int(&column, buf);
    buf = deserialize_int(&row_length, buf);
    if (4 * sizeof(int) + row_length > size) {
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
    struct sockaddr_in my_address;                  /* our address */
    int fd;                                         /* our socket */

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
    pollfd fd_wait;
    fd_wait.events = POLLIN;
    fd_wait.fd = fd;
    poll(&fd_wait, 1, -1);  /* waiting for first connect */
    for (int step = 0; step < steps_amount; ++step) {
        std::vector<Node> new_nodes;
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
        nodes.reserve(nodes.size() + new_nodes.size());
        for (Node &node : new_nodes) {
            nodes.push_back(node);
        }
    }
    return 0;
}



int main(int argc, char ** argv) {

    int port = DEFAULT_PORT,
        field_size = DEFAULT_FIELD_SIZE,
        steps_amount = DEFAULT_STEPS_AMOUNT;
    if (argc > 4) {
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
    pthread_t server_thread;

    printf("using params: port=%d, filed_size=%dx%d generations_amount=%d\n",
           port, field_size, field_size, steps_amount);

    return server(argv_parsed);
}