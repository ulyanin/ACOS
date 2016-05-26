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
#include <malloc.h>
#include "life_game_common.h"
#include "node.h"


int map[2][MAX_FIELD_SIZE][MAX_FIELD_SIZE];

int field_size;

int add(int x)
{
    if (x == field_size - 1)
        return 0;
    else
        return x + 1;
}

int sub(int x)
{
    if (x == 0)
        return field_size - 1;
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


void die(const char *s)
{
    perror(s);
    exit(1);
}

void get_data(Node &server, int step)
{
    char * data = nullptr;
    int pieces_amount = -1;
    int pieces_received = -1;
    while (pieces_amount != pieces_received) {
        int size = server.get_data(data, 0);
        if (size >= strlen(PHRASE_NODE_DONE_STEP) + sizeof(int) &&
            strncmp(data, PHRASE_NODE_DONE_STEP, strlen(PHRASE_NODE_DONE_STEP)) == 0) {
            pieces_amount = *(int *)(data + strlen(PHRASE_NODE_DONE_STEP));
            return;
        }
        int server_step, row, column, row_length;
        char * buf = deserialize_int(&server_step, data);
        buf = deserialize_int(&row, buf);
        buf = deserialize_int(&column, buf);
        buf = deserialize_int(&row_length, buf);
        if (4 * sizeof(int) + row_length > size) {
            fprintf(stderr, "received msg: truncated length\n");
        } else {
            memcpy(&map[step][row][column], buf, row_length);
            ++pieces_received;
        }
    }
    free(data);
}

void client(int server_port, char * server_name)
{
    struct sockaddr_in server_address;
    int sock_fd;
    socklen_t address_len = sizeof(server_address);
    if ((sock_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &server_address, 0, sizeof(server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_aton(server_name , &server_address.sin_addr) == 0)
    {
        die("inet_aton() failed\n");
    }
    Node server(server_address);

    for (int step = 0; ; step++) {
        get_data(server, step);
        calculate_step(server, step);
        send_data(server, step);
    }
}

int main(int argc, char **argv)
{



    if (sendto(sock_fd, PHRASE_BE_NODE, strlen(PHRASE_BE_NODE), 0, (struct sockaddr *)&si_other, addr_len) == -1)
    {
        die("sendto()");
    }

    while(1) {
        ssize_t msg_len;
        while (1) {
            if ((msg_len = recvfrom(sock_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &addr_len)) == -1) {
                die("recvfrom()");
            }
            if (strlen(buf) < SECRET_PASSPHRASE_LENGTH) {
                die("after recieve: too small msg");
            }
            if (strncmp(buf, PHRASE_NODE_DONE_STEP, strlen(PHRASE_NODE_DONE_STEP)) == 0) {
                break;
            }
            int row, col;
            char * next_data = deserialize_int(&row, buf);
            next_data = deserialize_int(&col, next_data);
            memcpy(&map[0][row][col], next_data, msg_len - 2 * sizeof(int));
        }
        for (int i = 0; i < field_size; ++i) {
            for (int j = 0; j < field_size; ++j) {

            }
        }
        if (sendto(sock_fd, message, strlen(message), 0, (struct sockaddr *)&si_other, addr_len) == -1)
        {
            die("sendto()");
            return 0;
        }
    }
    /*
     * receive a reply and print it
     * clear the buffer by filling null, it might have previously received data
     */
    memset(buf, 0, BUFLEN);
    /* try to receive some data, this is a blocking call */
    if (recvfrom(sock_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &addr_len) == -1)
    {
        die("recvfrom()");
    }
    printf("server answer=\"%s\"\n", buf);
    close(sock_fd);
    return 0;
}