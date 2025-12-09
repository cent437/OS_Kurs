#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h> // Для close()
int sock = 0;
#define PORT 1225
#define BUFFER_SIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void send_message(const char *text)
{
    send(sock, text, strlen(text), 0);
}
void receive_message(char *buffer)
{
    int valread = recv(sock, buffer, BUFFER_SIZE, 0);
    if (valread < 0)
    {
        error("recv failed");
    }
}

int main(int argc, char const *argv[])
{
    system("chcp 65001");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    int valread;
    struct sockaddr_in serv_addr;
    serv_addr.sin_addr.S_un.S_addr = htonl(0xc0a87d93);
    char buffer[BUFFER_SIZE] = {0};

    // 1. Создание сокета клиента
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        error("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Connection Failed");
    }
    printf("Подключение к серверу успешно установлено\n");
    int ui_index = -1;
    char message[BUFFER_SIZE] = {0};
    while (ui_index)
    {
        switch (ui_index)
        {
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

    // 5. Закрытие сокета
    close(sock);
    return 0;
}