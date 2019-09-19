#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
    // file descriptor, for-loop counter, and socket address
    int cfd, reader_count;
    struct sockaddr_in server;
    cfd = socket(AF_INET, SOCK_STREAM, 0);

    // error check for socket()
    if (cfd == -1) {
        perror("error with socket");
        return 1;
    }

    // initialize &server to 0
    memset(&server, 0, sizeof(struct sockaddr_in));

    // setting basic values, server IP address from command line
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    // error check for inet_pton()
    if (inet_pton(AF_INET, argv[1], &server.sin_addr) == 0) {
        perror("error with address conversion");
        return 1;
    }

    // error check for connect()
    if (connect(cfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        perror("error with connect");
        return 1;
    }

    send(cfd, argv[3], strlen(argv[3]), 0);
    unsigned char buf[9999];
    memset(buf, '\0', sizeof(buf));

    // infinite loop while reading file
    while (1 == 1) {
        int reader;
        reader = read(cfd, buf, sizeof(buf));

        // error check for reading file
        if (reader < 0) {
            perror("reading error");
            return 1;
        }
        // break at end of file
        else if (reader == 0) break;
        reader_count++;
    }

    // error check if file was read at all
    if (reader_count == 0) {
        perror("file does not exist, or file has size 0");
        close(cfd);
        return 1;
    }

    // creating a file
    FILE *f = fopen(argv[4], "w+");

    // error check for writing a file
    if (f == NULL) {
        perror("writing error");
        close(cfd);
        return 1;
    }

    // writing the file, closing cfd, END
    fwrite(buf, 1, sizeof(buf), f);
    close(cfd);
    return 0;
}
