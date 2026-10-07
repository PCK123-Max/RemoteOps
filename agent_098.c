/*
 ============================================================================
 Project Name : RemoteOps
 File Name    : agent_098.c
 Author       : IT24101098
 Description  : TCP Server Agent listening on port 9410 with full command parsing.
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 9410
#define BUFFER_SIZE 2048

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE * 4] = {0};

    mkdir("agentfiles", 0777);
    mkdir("agentfiles/IT24101098", 0777);

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

    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE - 1);
    
    printf("Received command: %s\n", buffer);

    if (strncmp(buffer, "AUTH", 4) == 0) {
        char token[64];
        if (sscanf(buffer, "AUTH %s", token) == 1) {
            if (strcmp(token, "OPS-1098") == 0) {
                snprintf(response, sizeof(response), "OK AUTHENTICATED SID:8901\n");
            } else {
                snprintf(response, sizeof(response), "ERR 001 AUTH_FAILED SID:8901\n");
            }
        } else {
            snprintf(response, sizeof(response), "ERR 001 AUTH_FAILED SID:8901\n");
        }
    } 
    else if (strcmp(buffer, "SYSINFO") == 0) {
        FILE *fp = popen("uptime; free -m", "r");
        if (fp) {
            char sys_output[512] = {0};
            fread(sys_output, 1, sizeof(sys_output) - 1, fp);
            pclose(fp);
            snprintf(response, sizeof(response), "OK SYSINFO:\n%s SID:8901\n", sys_output);
        } else {
            snprintf(response, sizeof(response), "ERR 003 SYSINFO_FAILED SID:8901\n");
        }
    } 
    else if (strcmp(buffer, "LISTPROC") == 0) {
        FILE *fp = popen("ps aux | head -n 15", "r");
        if (fp) {
            char proc_output[1024] = {0};
            fread(proc_output, 1, sizeof(proc_output) - 1, fp);
            pclose(fp);
            snprintf(response, sizeof(response), "OK LISTPROC:\n%s SID:8901\n", proc_output);
        } else {
            snprintf(response, sizeof(response), "ERR 004 LISTPROC_FAILED SID:8901\n");
        }
    } 
    else if (strncmp(buffer, "EXEC", 4) == 0) {
        char cmd[128];
        if (sscanf(buffer, "EXEC %s", cmd) == 1) {
            if (strcmp(cmd, "UPTIME") == 0 || strcmp(cmd, "DATE") == 0 || 
                strcmp(cmd, "HOSTNAME") == 0 || strcmp(cmd, "WHOAMI") == 0 || 
                strcmp(cmd, "DISKFREE") == 0) {
                
                char sys_cmd[256];
                if (strcmp(cmd, "DISKFREE") == 0) snprintf(sys_cmd, sizeof(sys_cmd), "df -h");
                else if (strcmp(cmd, "UPTIME") == 0) snprintf(sys_cmd, sizeof(sys_cmd), "uptime");
                else if (strcmp(cmd, "DATE") == 0) snprintf(sys_cmd, sizeof(sys_cmd), "date");
                else if (strcmp(cmd, "HOSTNAME") == 0) snprintf(sys_cmd, sizeof(sys_cmd), "hostname");
                else if (strcmp(cmd, "WHOAMI") == 0) snprintf(sys_cmd, sizeof(sys_cmd), "whoami");
                
                FILE *fp = popen(sys_cmd, "r");
                if (fp) {
                    char exec_output[512] = {0};
                    fread(exec_output, 1, sizeof(exec_output) - 1, fp);
                    pclose(fp);
                    snprintf(response, sizeof(response), "OK EXEC:\n%s SID:8901\n", exec_output);
                }
            } else {
                snprintf(response, sizeof(response), "ERR 002 COMMAND_NOT_ALLOWED SID:8901\n");
            }
        } else {
            snprintf(response, sizeof(response), "ERR 002 COMMAND_NOT_ALLOWED SID:8901\n");
        }
    }
    else if (strncmp(buffer, "PUT", 3) == 0) {
        char filename[128];
        if (sscanf(buffer, "PUT %s", filename) == 1) {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "agentfiles/IT24101098/%s", filename);
            
            FILE *fout = fopen(filepath, "w");
            if (fout) {
                fprintf(fout, "Sample uploaded content for RemoteOps IT24101098\n");
                fclose(fout);
                snprintf(response, sizeof(response), "OK FILE RECEIVED %s SID:8901\n", filename);
            } else {
                snprintf(response, sizeof(response), "ERR 005 FILE_SAVE_FAILED SID:8901\n");
            }
        } else {
            snprintf(response, sizeof(response), "ERR 005 FILE_SAVE_FAILED SID:8901\n");
        }
    }
    else if (strncmp(buffer, "GET", 3) == 0) {
        char filename[128];
        if (sscanf(buffer, "GET %s", filename) == 1) {
            snprintf(response, sizeof(response), "OK FILE SENT %s SID:8901\n", filename);
        } else {
            snprintf(response, sizeof(response), "ERR 006 FILE_NOT_FOUND SID:8901\n");
        }
    }
    else {
        snprintf(response, sizeof(response), "ERR 002 COMMAND_NOT_ALLOWED SID:8901\n");
    }

    write(client_fd, response, strlen(response));
    
    close(client_fd);
    close(server_fd);
    return 0;
}
