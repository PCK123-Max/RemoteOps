/*
 ============================================================================
 Project Name : RemoteOps
 File Name    : controller_098.c
 Author       : IT24101098
 Description  : TCP Client Controller that connects to port 9410 and sends commands.
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9410
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to RemoteOps Agent on port %d.\n", PORT);
    printf("Enter command to execute on Agent (e.g., uname -a): ");
    
    if (fgets(command, sizeof(command), stdin) != NULL) {
        command[strcspn(command, "\n")] = 0;
        write(sock, command, strlen(command));
    }

    int valread = read(sock, buffer, BUFFER_SIZE - 1);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("\n--- Agent Execution Output ---\n%s\n------------------------------\n", buffer);
    }

    close(sock);
    return 0;
}
