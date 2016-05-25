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
#include "life_game_common.h"

const int DEFAULT_FIELD_SIZE = 666;
const int MAX_FIELD_SIZE = 5000;
const int DEFAULT_STEPS_AMOUNT = 200;
char map[2][MAX_FIELD_SIZE][MAX_FIELD_SIZE];

inline char gen_rnd_char()
{
    return (char)(rand() % (127 - 32) + 32);
}

struct Node
{
    struct sockaddr_in node_addr;
    char secret_passphrase[SECRET_PASSPHRASE_LENGTH + 1];
    /* Node() {
        memset((char *)&node_addr, 0, sizeof(node_addr));
        memset(secret_passphrase, 0, sizeof(secret_passphrase));
    } */
    Node(const sockaddr_in &addr) {
        memcpy(&node_addr, &addr, sizeof(addr));
        for (int i = 0; i < SECRET_PASSPHRASE_LENGTH; ++i)
            secret_passphrase[i] = gen_rnd_char();
        secret_passphrase[SECRET_PASSPHRASE_LENGTH] = 0;
    }
};

std::vector<Node> nodes;

void die(const char *s)
{
    perror(s);
    exit(1);
}

void print_bytes(void *object, size_t size)
{
    unsigned char * bytes = (unsigned char *)object;
    size_t i;

    printf("[ ");
    for(i = 0; i < size; i++)
    {
        printf("%02x ", bytes[i]);
    }
    printf("]\n");
}

void add_nodes(int fd, std::vector<Node>::iterator first, std::vector<Node>::iterator last)
{
    nodes.insert(nodes.end(), first, last);
}

bool step_completed(const std::vector<bool> &mask)
{
    for (auto node_completed : mask) {
        if (node_completed == 0)
            return false;
    }
    return true;
}

void send_row(const char * data, int node_num, int field_size, int socket_fd, int row_number)
{
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    char * send_data = new char[2048];
    char * current = nullptr;
    int rest_len = 1024 - 2 * sizeof(int);
    for (int i = 0; i < field_size; i += rest_len) {
        size_t piece_size = (size_t)(i + rest_len < field_size ? rest_len : field_size - i);
        current = send_data;
        current = serialize_int(row_number, current);
        current = serialize_int(i, current);
        memcpy(current, data + i, piece_size);
        if (sendto(socket_fd, data + i, piece_size, 0, (struct sockaddr *) &nodes[node_num], addrlen) == -1) {
            die("sendto()");
        }
    }
    delete [] send_data;
}


int main(int argc, char **argv)
{
    struct sockaddr_in myaddr;                  /* our address */
    struct sockaddr_in remaddr;                 /* remote address */
    socklen_t addrlen = sizeof(remaddr);        /* length of addresses */
    ssize_t recvlen;                                /* # bytes received */
    int fd;                                     /* our socket */
    char buf[BUFSIZE];     /* receive buffer */

    int port = DEFAULT_PORT,
        field_size = DEFAULT_FIELD_SIZE,
        steps_amount = DEFAULT_STEPS_AMOUNT;
    if (argc > 5) {
        printf("wrong params: using:\n");
        printf("./life_game_server port field_size field_size_columns\n");
        return 0;
    }
    if (argc >= 2) {
        port = std::stoi(argv[1]);
    }
    if (argc >= 3) {
        field_size = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        steps_amount = std::stoi(argv[4]);
    }
    printf("using params: port=%d, filed_size=%dx%d generations=%d\n",
           port, field_size, field_size, steps_amount);

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        return 0;
    }

    /* bind the socket to any valid IP address and a specific port */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }
    for (int step = 0; step < steps_amount; ++step) {
        std::vector <Node> new_nodes;
        int row_to_each_node = field_size / (int)nodes.size();
        int rest = field_size % (int)nodes.size();
        std::vector <std::pair <int, int> > tasks_for_nodes(nodes.size());
        for (int i = 0, last = 0; i < (int)nodes.size(); ++i) {
            int rows = row_to_each_node + (rest < i);
            tasks_for_nodes[i] = std::make_pair(last, last + rows);
            for (int t = -1; t <= rows; ++t) {
                int r = (t + last >= 0 ? t + last : field_size - 1);
                send_row(map[(step + 1) & 1][r], i, field_size, fd, t + 1);
            }
            last += row_to_each_node + (rest < i);
        }
        std::vector<bool> completed_nodes(nodes.size(), 0);
        while (!step_completed(completed_nodes)) {
            bzero(buf, sizeof(buf));
            recvlen = recvfrom(fd, buf, BUFSIZE, 0, (sockaddr *)&remaddr, &addrlen);
            if (recvlen < 0) {
                printf("sin_addr=%s, sin_family=%d, sin_port=%d, sin_zero='%s'\n",
                       inet_ntoa(remaddr.sin_addr), remaddr.sin_family, ntohs(remaddr.sin_port), remaddr.sin_zero);
                perror("recvfrom function:");
                continue;
            }
            buf[recvlen] = 0;
            if (recvlen < 10) {
                printf("too small input msg, may be spam?\n");
                printf("\tspam='%s'\n", buf);
                continue;
            }
            if (strncmp(buf, PHRASE_BE_NODE, SECRET_PASSPHRASE_LENGTH) == 0) {
                new_nodes.push_back(Node(remaddr));
            } else {
                if (recvlen < MINIMAL_DATA_SIZE) {
                    printf("too small input for a node, may be spam?\n");
                    printf("\tspam='%s'\n", buf);
                    continue;
                }
                for (int i = 0; i < (int)nodes.size(); ++i) {
                    /* check which node sent this msg */
                    if (strncmp(buf, nodes[i].secret_passphrase, SECRET_PASSPHRASE_LENGTH) == 0) {
                        /* found remote node */
                        const char * buf_data = buf + SECRET_PASSPHRASE_LENGTH;
                        if (strlen(buf_data) >= SECRET_PASSPHRASE_LENGTH &&
                                strncmp(buf_data, PHRASE_NODE_DONE_STEP, SECRET_PASSPHRASE_LENGTH) == 0) {
                            /* check if node completed step */
                            completed_nodes[i] = true;
                            continue;
                        }
                        int row, column, len, node_step;
                        buf_data = deserialize(&row, &column, &len, &node_step, buf_data);
                        if (node_step != step) {
                            printf("found old package with old step\n");
                            printf("\tstep='%s'\n", buf);
                            break;
                        }
                        row += tasks_for_nodes[i].first;
                        if (row >= field_size || column >= field_size || len + column > field_size) {
                            printf("data out of filed's range\n");
                            printf("row=%d, column=%d, given_len=%d, rest_len=%d",
                                   row, column, len, field_size - column);
                            break;
                        }
                        memcpy(&map[step & 1][row][column], buf_data, len * sizeof(int));
                    }
                }
            }

        }
        add_nodes(fd, new_nodes.begin(), new_nodes.end());
    }
//    while (1) {
//        if (nodes.size() == 0) {
//            printf("wating for nodes connect\n");
//            usleep(1e6);
//        }
//        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (sockaddr *)&remaddr, &addrlen);
//        printf("received %d bytes\n", recvlen);
//        if (recvlen > 0) {
//            buf[recvlen] = 0;
//            printf("received message: \"%s\"\n", buf);
//        }
        //printf("sin_addr=%d, sin_family=%d, sin_port=%d, sin_zero='%s'\n", remaddr.sin_addr, remaddr.sin_family, ntohs(remaddr.sin_port), remaddr.sin_zero);
        //print_bytes(&remaddr, sizeof(remaddr));
//    }
//    return 0;
    /* create a UDP socket */

//    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//        perror("cannot create socket\n");
//        return 0;
//    }

    /* bind the socket to any valid IP address and a specific port */

//    memset((char *)&myaddr, 0, sizeof(myaddr));
//    myaddr.sin_family = AF_INET;
//    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    myaddr.sin_port = htons(port);
//
//    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
//        perror("bind failed");
//        return 0;
//    }

    /* now loop, receiving data and printing what we received */
//    for (;;) {
//        printf("waiting on port %d\n", port);
//        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
//        printf("received %d bytes\n", recvlen);
//        if (recvlen > 0) {
//            buf[recvlen] = 0;
//            printf("received message: \"%s\"\n", buf);
//        }
//        if (sendto(fd, buf, recvlen, 0, (struct sockaddr*) &remaddr, addrlen) == -1)
//        {
//            /*die("sendto()");*/
//        }
//    }
    /* never exits */
    return 0;
}

