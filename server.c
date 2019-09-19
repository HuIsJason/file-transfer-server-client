#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    // server file descriptor and socket address
    int sfd;
    struct sockaddr_in server;
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    // error check for socket()
    if (sfd == -1) {
        perror("error with socket");
        return 1;
    }

    // initialize server to 0
    memset(&server, 0, sizeof(struct sockaddr_in));

    // assign basic values, IP address from command line
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // error check for bind()
    if (bind(sfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        perror("error with bind");
        return 1;
    }

    // error check for listen()
    if (listen(sfd, 5) == -1) {
        perror("error with listen");
        return 1;
    }

    // set up epoll event
    struct epoll_event e;
    int ep;
    ep = epoll_create1(0);
    e.data.fd = sfd;
    e.events = EPOLLIN;
    epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &e);

    // infinite loop
    while (1 == 1) {
        // adding epoll wait
        epoll_wait(ep, &e, 1 , -1);
        if (e.data.fd == sfd) {
            // client address and client file descriptor
            struct sockaddr_in ca;
            socklen_t sinlen = sizeof(struct sockaddr_in);
            int cfd;
            cfd = accept(sfd, (struct sockaddr *)&ca, &sinlen);

            // error check for accept()
            if (cfd == -1) {
                // define epoll event
                e.data.fd = cfd;
                e.events = EPOLLIN;
                epoll_ctl(ep, EPOLL_CTL_ADD, cfd, &e);
            }
        } else {
            // client file descriptor, message receiver
            char buf[100];
            int cfd, receiver;
            cfd = e.data.fd;

            memset(buf, 0, sizeof(buf));

            // message receiver
            receiver = recv(cfd, buf, 100, 0);

            if (receiver <= 0) {
                epoll_ctl(ep, EPOLL_CTL_DEL, cfd, NULL);
                close(cfd);
            } else {
                // error check access()
                if (access(buf, R_OK) == -1) {
                    perror("error with requested file");
                    close(cfd);
                } else {
                    unsigned char filebuf[9999];
                    int fd;
                    fd = open(buf, 'r');

                    // infinite loop
                    while (1 == 1) {
                        int reader_count;
                        reader_count = read(fd, filebuf, sizeof(filebuf));

                        // error check for reading file
                        if (reader_count < 0) {
                            perror("error with reading file");
                            return 1;
                        }

                        // exit if reader_count isn't reading
                        if (reader_count == 0) break;

                        void *increment = filebuf;
                        while (reader_count > 0) {
                            // set up file writing
                            int writer_count;
                            writer_count = write(cfd, increment, reader_count);

                            // error check for file writing
                            if (writer_count <= 0) {
                                perror("error with writing");
                                return 1;
                            }
                            increment += writer_count;
                            reader_count -= writer_count;
                        }
                    }
                    // close appropriate file descriptors
                    close(fd);
                    close(cfd);
                }
            }
        }
    }
    // close server file descriptor
    close(sfd);
    return 0;
}
