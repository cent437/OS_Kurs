#include "receiver.h"
#include <stdio.h>
#include <pthread.h>
int main(int argc, char const *argv[])
{
    system("chcp 65001");
    WSADATA wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    init();
    set_connect();
    int valread = 0;
    while (1)
    {
        puts("Ожидание сообщений...");
        recieve_message();
    }

    // closing the connected socket
    // close(new_socket_1);
    // close(new_socket_2);
    // closing the listening socket
    // close(socket_1);
    // close(socket_2);
    return 0;
}