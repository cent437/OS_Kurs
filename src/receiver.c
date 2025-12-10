
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h>
#define PORT 8080
int server_fd, new_socket;
ssize_t valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char buffer[1024] = {0};
char *hello = "Hello from server";
typedef struct user
{
    unsigned id;
    char user[256];

} user;

user user_database[20];

void init()
{
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}
void set_connect()
{
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             &addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}
void add_user()
{
}
int main(int argc, char const *argv[])
{
    system("chcp 65001");
    WSADATA wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    init();
    set_connect();
    while (1)
    {
        puts("Ожидание сообщений...");
        valread = recv(new_socket, buffer, sizeof(buffer), 0);
        if (valread < 0)
        {
            int error_code = WSAGetLastError();
            printf("Ошибка передачи. Код: %d\n", error_code);
            if (error_code == WSAECONNRESET)
            {
                puts("Соединение с клиентом потеряно.");
                set_connect();
            }
        }

        if (buffer[0] != 0)
        {
            printf("Сообщение от клиента: %s\n", buffer);
            rewind(stdout);
        }

        sleep(1);
    }

    // closing the connected socket
    close(new_socket);

    // closing the listening socket
    close(server_fd);
    return 0;
}