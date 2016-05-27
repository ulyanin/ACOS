//
// Created by ulyanin on 25.05.16.
//

#ifndef LIFE_GAME_NODE_H
#define LIFE_GAME_NODE_H

#include <cstdlib>
#include <cstring>
#include "life_game_common.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

class Node
{
private:
    int set_up_connection_();
    struct sockaddr_in node_address_;

//    struct sockaddr_in server_address_;
    static socklen_t address_len_();
    int socket_fd_;
//    char secret_pass_phrase_[SECRET_PASSPHRASE_LENGTH + 1];
    char data_to_send_[BUFFER_SIZE];
    char buf_[BUFFER_SIZE];
    int data_to_send_size_;
public:
    Node(const sockaddr_in &address_to_connect);
    ~Node();
    void send_buffer(const char *buf, int buf_len = -1);
    int get_data(char * &data, int timeout=TIMEOUT_WAIT_NODES_MILLISECONDS);
    int push_data_str(const char * data, int size);  // return rest size or -1 if overflow
    int push_data_int(int num);                      // return rest size or -1 if overflow
    ssize_t send_pushed_data();
    int get_rest_data_size() const;                  // return rest size or -1 if overflow
    void reset_data();
    bool is_data_to_send_empty() const;
    int get_socket_fd() const;
};


char * deserialize_int(int * number, char * data);


#endif //LIFE_GAME_NODE_H
