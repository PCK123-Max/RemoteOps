/*
 ============================================================================
 Project Name : RemoteOps
 File Name    : agent_098.c
 Author       : IT24101098
 Description  : TCP Server Agent listening on port 9410 for remote commands.
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9410
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE * 4] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("RemoteOps Agent listening on port %d...\n", PORT);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Controller connected! Waiting for command...\n");

    read(client_fd, buffer, BUFFER_SIZE);
    printf("Received command: %s\n", buffer);

    FILE *fp = popen(buffer, "r");
    if (fp == NULL) {
        strcpy(response, "Failed to run command.");
    } else {
        size_t len = fread(response, 1, sizeof(response) - 1, fp);
        response[len] = '\0';
        pclose(fp);
    }

    write(client_fd, response, strlen(response));

    close(client_fd);
    close(server_fd);
    return 0;
}
