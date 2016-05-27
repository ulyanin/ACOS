#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


char map[2][MAX_FIELD_SIZE][MAX_FIELD_SIZE];

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

void get_data(Node &server, int &rows_amount, int &step)
{
    char * data = nullptr;
    int pieces_amount = -1;
    int pieces_received = 0;
    int phrase_len = strlen(PHRASE_SERVER_DONE_STEP);
    while (pieces_amount != pieces_received) {
        int size = server.get_data(data, 0);
        if (size >= phrase_len + (int)sizeof(int) &&
            strncmp(data, PHRASE_SERVER_DONE_STEP, phrase_len) == 0) {
            char * tmp  = deserialize_int(&step, data + phrase_len);
            tmp         = deserialize_int(&pieces_amount, tmp);
            tmp         = deserialize_int(&field_size, tmp);
            deserialize_int(&rows_amount, tmp);
            continue;
        }
        int server_step, row, column, row_length;
        char * buf = deserialize_int(&server_step, data);
        buf = deserialize_int(&row, buf);
        buf = deserialize_int(&column, buf);
        buf = deserialize_int(&row_length, buf);
        if (server_step != step) {
            printf("ERR: received msg with step which is different from our; (serv=%d) != %d\n", server_step, step);
            step = server_step;
//            continue;
        }
        if (4 * (int)sizeof(int) + row_length > size) {
            fprintf(stderr, "received msg: truncated length\n");
        } else {
            memcpy(&map[step & 1][row][column], buf, row_length);
            ++pieces_received;
        }
    }
    printf("received %d/%d pieces;\n", pieces_received, pieces_amount);
    free(data);
}

void calculate_step(int step, int rows_amount)
{
    for (int i = 1; i < rows_amount - 1; i++) {
        for (int j = 0; j < field_size; j++)
        {
            recalc(step & 1, i, j);
        }
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

void send_data(Node &server, int step, int rows_amount)
{
    int pieces_amount = 0;
    for (int i = 1; i < rows_amount - 1; ++i) {
        char * row = get_row_data(step, i, field_size);
        if (!server.is_data_to_send_empty()) {
            fprintf(stderr, "WARN! data_to_send was not empty\n");
            server.reset_data();
        }
        for (int column = 0; column < field_size; ) {
            int rest = server.get_rest_data_size() - 4 * sizeof(int);
            if (column + rest > field_size)
                rest = field_size - column;
            server.push_data_int(step);  // step
            server.push_data_int(i - 1);  // row [0..rows_amount)
            server.push_data_int(column);
            server.push_data_int(rest);  // row length
            server.push_data_str(row + column, rest);
            server.send_pushed_data();
            ++pieces_amount;
            column += rest;
        }
#ifdef DEBUG_LOG
        printf("sent row%d\n", i);
#endif
    }
    server.push_data_str(PHRASE_NODE_DONE_STEP, -1);
    server.push_data_int(pieces_amount);
    server.push_data_int(step);
    server.push_data_int(rows_amount - 2);
    server.send_pushed_data();
    printf("summary pieces sent amount=%d\n", pieces_amount);
}

void client(char * server_name, int server_port)
{
    struct sockaddr_in server_address;
    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_aton(server_name , &server_address.sin_addr) == 0)
    {
        die("inet_aton() failed\n");
    }
    Node server(server_address);
    server.set_up_socket();
    server.register_as_client_node();
    if (server.receive_and_accept_server_ans() < 0)
        return;
    for (int step = 0; ; step++) {
        int rows_amount;
        printf("getting data\n");
        get_data(server, rows_amount, step);
        printf("calculating step data; fsize=%d, rows=%d\n", field_size, rows_amount);
        calculate_step(step, rows_amount);
        printf("sending data\n");
        send_data(server, step, rows_amount);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Wrong params amount(=%d); using: ./life_game_client server_address server_port\n", argc);
    } else {
        printf("%s %s\n", argv[1], argv[2]);
        client(argv[1], std::stoi(argv[2]));
    }
    return 0;
}