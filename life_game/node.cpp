//
// Created by ulyanin on 25.05.16.
//

#include "node.h"
#include <random>
#include <ctime>
#include <iostream>
#include <malloc.h>

inline char gen_rnd_char()
{
    static std::default_random_engine generator((size_t)time(0));
    static std::uniform_int_distribution<char> rand_char(32, 127);
    return rand_char(generator);
}

Node::Node(const sockaddr_in &address_to_connect)
{
    memcpy(&node_address_, &address_to_connect, sizeof(address_to_connect));
//    for (int i = 0; i < SECRET_PASSPHRASE_LENGTH; ++i)
//        secret_pass_phrase_[i] = gen_rnd_char();
//    secret_pass_phrase_[SECRET_PASSPHRASE_LENGTH] = 0;
    set_up_connection_();
    memset(data_to_send_, 0, sizeof(data_to_send_));
    data_to_send_size_ = 0;
}

Node::~Node()
{

}

socklen_t Node::address_len_()
{
    return sizeof(node_address_);
}

int Node::set_up_connection_()
{
    ssize_t recvlen;
    if ((socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot set up socket");
        return socket_fd_;
    }
    if (connect(socket_fd_, (struct sockaddr *)&node_address_, sizeof(node_address_)) < 0) {
        perror("cannot set up connection");
        return -1;
    }
}

void Node::send_buffer(const char *buf, int buf_len)
{
    size_t len = (buf_len == -1 ? strlen(buf) : buf_len);
    ssize_t amount = send(socket_fd_, buf, len, 0);
    if (amount == -1) {
        perror("send_buffer: send()");
    }
    if (amount != buf_len) {
        fprintf(stderr, "WARNING!! MESSAGE MAY BE TRUNC\n");
    }
}

int Node::get_data(char * &data, int timeout)
{
    memset(buf_, 0, sizeof(buf_));
    size_t amount = (size_t)recv(socket_fd_, buf_, BUFFER_SIZE, 0);
    if (BUFFER_SIZE > MAX_DATA_SIZE) {
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
    ssize_t sent = send(socket_fd_, data_to_send_, data_to_send_size_, 0);
    if (sent < 0) {
        perror("error while sending data");
    }
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