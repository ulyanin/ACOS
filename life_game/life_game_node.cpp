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
#include "life_game_common.h"

#define PORT 1153
#define BUFSIZE 1048576

#define BUFLEN 1048576
#define SERVER "127.0.0.1"

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    struct sockaddr_in si_other;
    int s;
    socklen_t addr_len = sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    char * server_name = SERVER;
    int server_port = PORT;
    printf("argc=%d\n", argc);
    if (argc == 3) {
        server_name = argv[1];
        server_port = atoi(argv[2]);
    }
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(server_port);
    if (inet_aton(server_name , &si_other.sin_addr) == 0)
    {
        die("inet_aton() failed\n");
    }

    while(!feof(stdin)) {
        /*printf("Enter message : ");*/
        if (fgets(message, BUFLEN, stdin) == NULL)
            break;
        /* fputs(message, stdout); */
        memset(message, 0, sizeof(message));
        for (int i = 0; i < (1 << 15); ++i)
            message[i] = 'a';
        /* send the message */
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *)&si_other, addr_len) == -1)
        {
            die("sendto()");
        }
    }
    /*
     * receive a reply and print it
     * clear the buffer by filling null, it might have previously received data
     */
    memset(buf, 0, BUFLEN);
    /* try to receive some data, this is a blocking call */
    if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &addr_len) == -1)
    {
        die("recvfrom()");
    }
    printf("server answer=\"%s\"\n", buf);
    close(s);
    return 0;
}