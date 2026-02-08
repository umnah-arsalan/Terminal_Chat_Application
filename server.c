#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main() {
    int server_fd, client_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int clients[MAX_CLIENTS];
    
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server listening on port %d\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > max_fd)
                    max_fd = clients[i];
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);

            for (int i = 0; i< MAX_CLIENTS; i++) {
                if (clients[i] == -1) {
                    clients[i] = client_fd;
                    break;
                }
            }
            
            printf("New client connected: %d\n", client_fd);
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = clients[i];

            if (sd != -1 && FD_ISSET(sd, &readfds))
            {
                int bytes = read(sd, buffer, BUFFER_SIZE - 1);
                if (bytes <= 0)
                {
                    close(sd);
                    printf("Client disconnected: %d\n", sd);
                    clients[i] = -1;
                } else {
                    buffer[bytes] = '\0';
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (clients[j] != -1 && clients[j] != sd)
                            write(clients[j], buffer, bytes);
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}