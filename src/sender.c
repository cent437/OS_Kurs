#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h> // Для close()

#define PORT 1225
#define BUFFER_SIZE 1024
int sock = 0;
typedef struct user
{
    unsigned ip_addr;
    char user_name[256];
} user;
struct sockaddr_in serv_addr;

/*
    Подключение к серверу
 *   Возвращает -1 при ошибке создания сокета.
 *   Возвращает -2 при ошибке подключения к серверу.
 *   @param ip_addr IP-адрес в текстовом формате.
 */
int connect_to_server(const char *ip_addr, unsigned short port)
{
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.S_un.S_addr = inet_addr(ip_addr);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Ошибка подключения. Код:%d\n", WSAGetLastError());
        return -2;
    }
    return 0;
}

void send_message(void *data)
{
    send(sock, data, sizeof(data), 0);
}
void receive_message(char *buffer)
{
    int valread = recv(sock, buffer, BUFFER_SIZE, 0);
    if (valread < 0)
    {
        perror("recv failed");
    }
}
void help()
{
    puts("client.exe [IP-адрес] [номер порта]");
}
int main(int argc, char *argv[])
{
    system("chcp 65001");
    WSADATA wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    long long port_of = atoll(argv[2]);

    if (port_of > 0xFFFF)
    {
        puts("Ошибка. Неверное значение порта.");
        exit(1);
    }
    unsigned short port = (unsigned short)port_of;
    char buffer[BUFFER_SIZE] = {0};
    if (argc > 1)
    {
        if (connect_to_server(argv[1], port) == 0)
            printf("Подключение к серверу успешно установлено\n");
        int ui_index = -1;
        char message[BUFFER_SIZE] = {0};
        while (ui_index)
        {
            switch (ui_index)
            {
            case 1:
                printf("Введите сообщение:");
                scanf_s("%s", buffer, BUFFER_SIZE);
                send_message(buffer);
                puts("Сообщение отправлено.");
                break;

            case 2:
                receive_message(message);
                printf("Сообщение с сервера: %s", message);
                memset(message, 0, strlen(message));
                break;
            }
            puts("1. Отправить сообщение.");
            puts("2. Получить сообщения.");
            scanf("%d", &ui_index);
        }
        close(sock);
    }
    else
    {
        puts("Некорректный IP-адрес и порт.");
        help();
    }

    return 0;
}