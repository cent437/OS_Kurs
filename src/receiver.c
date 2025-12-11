
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "receiver.h"

int socket_1, socket_2, new_socket_1, new_socket_2;
size_t connect_counter;

struct sockaddr_in address_1, address_2;
char opt = '1';
int addrlen_1 = sizeof(address_1), addrlen_2 = sizeof(address_2);
char buffer_1[1024] = {0}, buffer_2[1024] = {0};

void add_user(char *data);
user user_database[50];
size_t hash(const char *data, size_t range)
{
    size_t n = 0, len = strlen(data);
    for (size_t i = 0; i < len; i++)
    {
        n = n * 3;
        n += data[i];
        n = n % range;
    }

    return n;
}
void init()
{
    // Creating socket file descriptor
    if ((socket_1 = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
        (socket_2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(socket_1, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)) ||
        setsockopt(socket_2, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address_1.sin_family = AF_INET;
    address_1.sin_addr.s_addr = INADDR_ANY;
    address_1.sin_port = htons(PORT_1);

    address_2.sin_family = AF_INET;
    address_2.sin_addr.s_addr = INADDR_ANY;
    address_2.sin_port = htons(PORT_2);
    // Forcefully attaching socket to the port 8080
    if ((bind(socket_1, (struct sockaddr *)&address_1,
              sizeof(address_1)) < 0) ||
        (bind(socket_2, (struct sockaddr *)&address_2,
              sizeof(address_2)) < 0))
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}
void set_connect()
{
    if ((listen(socket_1, 3) < 0) || (listen(socket_2, 3) < 0))
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if (((new_socket_1 = accept(socket_1, (struct sockaddr *)&address_1,
                                &addrlen_1)) < 0) ||
        ((new_socket_2 = accept(socket_2, (struct sockaddr *)&address_2,
                                &addrlen_2)) < 0))
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}
void add_user(char *data)
{
    // recv(new_socket_1, buffer, 1024, 0);
    // int splitter_index;
    // int seed;
    // char username[256];
    // size_t passowrd;
    // splitter_index = (strchr(buffer, ' ') - buffer);
    // strncpy(username, buffer, splitter_index);
    // passowrd = hash(buffer + splitter_index, seed);
    // strcpy(user_database[hash(username, 50)].user, username);
    // user_database[hash(username, 50)].passwd = passowrd;
}
void send_message()
{
    send(new_socket_1, buffer_1, 1024, 0);
    send(new_socket_2, buffer_2, 1024, 0);
}
void recieve_message()
{
    int valread_1 = recv(new_socket_1, buffer_1, 1024, 0);
    int valread_2 = recv(new_socket_2, buffer_2, 1024, 0);
    if (valread_1 <= 0 && valread_2 <= 0)
    {
        int error_code = WSAGetLastError();
        switch (error_code)
        {
        case WSAECONNRESET:
            puts("Существующее соединение было принудительно завершено удаленным узлом.");
            break;
        case 0:
            puts("Соединение потеряно.");
            break;
        default:
            printf("Непредвиденная ошибка.\nКод: %d\n", error_code);
            break;
        }
        set_connect();
    }
    else if (valread_1 > 0 && valread_2 > 0)
    {
        printf("Клиент 1:%s\n", buffer_1);
        printf("Клиент 2:%s\n", buffer_2);
        memset(buffer_1, 0, 1024);
        memset(buffer_2, 0, 1024);
        rewind(stdout);
    }
}
