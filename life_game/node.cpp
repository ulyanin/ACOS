//
// Created by ulyanin on 25.05.16.
//

#include "node.h"
#include <random>
#include <ctime>
#include <iostream>
#include <malloc.h>
#include <poll.h>

inline char gen_rnd_char()
{
    static std::default_random_engine generator((size_t)time(0));
    static std::uniform_int_distribution<char> rand_char(32, 127);
    return rand_char(generator);
}

char * deserialize_int(int * number, char * data)
{
    memcpy(number, data, sizeof(int));
    return data + sizeof(int);
}

Node::Node(const sockaddr_in &address_to_connect)
{
    memcpy(&node_address_, &address_to_connect, sizeof(address_to_connect));
//    for (int i = 0; i < SECRET_PASSPHRASE_LENGTH; ++i)
//        secret_pass_phrase_[i] = gen_rnd_char();
//    secret_pass_phrase_[SECRET_PASSPHRASE_LENGTH] = 0;
    memset(data_to_send_, 0, sizeof(data_to_send_));
    data_to_send_size_ = 0;
}

Node::~Node()
{ }

socklen_t Node::address_len_()
{
    return sizeof(node_address_);
}

int Node::set_up_socket()
{
    if ((socket_fd_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("cannot set up socket");
    }
    return socket_fd_;
}

int Node::set_up_connection()
{
    int rc = 0;
    connect(socket_fd_, (struct sockaddr *)&node_address_, sizeof(node_address_));
    if (rc < 0) {
        perror("cannot set up connection");
        return -1;
    } else {
        printf("connection is set up ok\n");
    }
    return rc;
}

int Node::send_buffer(const char *buf, int buf_len)
{
    ssize_t len = (buf_len == -1 ? strlen(buf) : buf_len);
//    pollfd wait_output;
//    wait_output.events = POLLOUT | POLL;
//    wait_output.fd = socket_fd_;
//    poll(&wait_output, 1, -1);
//    if (!(wait_output.revents & POLLOUT))
//        fprintf(stderr, "cannot send\n");
    usleep(100);
    ssize_t amount = sendto(socket_fd_, buf, len, 0, (sockaddr *)&node_address_, sizeof(node_address_));
    if (amount == -1) {
        perror("send_buffer: send()");
    }
    if (amount != len) {
        fprintf(stderr, "WARNING!! MESSAGE MAY BE TRUNC %d %d\n", (int)amount, (int)len);
    }
    return amount;
}

int Node::get_data(char * &data, int timeout)
{
    memset(buf_, 0, sizeof(buf_));
    size_t amount = (size_t)recv(socket_fd_, buf_, BUFFER_SIZE, 0);
    if (amount > MAX_DATA_SIZE) {
        fprintf(stderr, "WARN. Too much data size; may be spam?\n");
    }
    data = (char *)realloc(data, amount + 1);
    data[amount] = 0;
    memcpy(data, buf_, amount);
    return amount;
}

int Node::push_data_int(int num)
{
    return push_data_str((char *)&num, sizeof(num));
}

int Node::push_data_str(const char * data, int size)
{
    if (size == -1) {
        size = strlen(data);
    }
    if (data_to_send_size_ + size > MAX_DATA_SIZE)
        return -1;
    memcpy(data_to_send_ + data_to_send_size_, data, size);
    data_to_send_size_ += size;
    return MAX_DATA_SIZE - data_to_send_size_;
}

ssize_t Node::send_pushed_data()
{
    if (data_to_send_size_ == 0)
        return 0;
    ssize_t sent = send_buffer(data_to_send_, data_to_send_size_);
    reset_data();
    return sent;
}

void Node::reset_data()
{
    data_to_send_size_ = 0;
    memset(data_to_send_, 0, sizeof(data_to_send_));
}

int Node::get_rest_data_size() const
{
    return MAX_DATA_SIZE - data_to_send_size_;
}

bool Node::is_data_to_send_empty() const
{
    return data_to_send_size_ == 0;
}

int Node::get_socket_fd() const
{
    return socket_fd_;
}

int Node::receive_and_accept_server_ans()
{
    sockaddr_in new_sock_address;
    socklen_t new_sock_address_len = sizeof(new_sock_address);
    memset(buf_, 0, sizeof(buf_));
    ssize_t receive_len = recvfrom(socket_fd_, buf_, MAX_DATA_SIZE, 0,
                                   (sockaddr *)&new_sock_address, &new_sock_address_len);
    printf("received : msg=%s %s %d\n", buf_,
           inet_ntoa(new_sock_address.sin_addr), (int) ntohs(new_sock_address.sin_port));
    if (receive_len < (int)strlen(PHRASE_SERVER_ACCEPT_NODE)) {
        fprintf(stderr, "strange data received: %s; spam?\n", buf_);
        return -1;
    } else if (strncmp(buf_, PHRASE_SERVER_ACCEPT_NODE, strlen(PHRASE_SERVER_ACCEPT_NODE)) == 0) {
        printf("Accept server answer\n");
        memcpy(&node_address_, &new_sock_address, new_sock_address_len);
        return set_up_connection();
    } else {
        return -1;
    }
}

int Node::register_as_client_node()
{
    return send_buffer(PHRASE_BE_NODE, -1);
}