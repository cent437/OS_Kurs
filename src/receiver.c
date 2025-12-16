
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "receiver.h"
struct sockaddr_in address[2] = {0, 0};

int
    addrlen[CLIENTS_QUANTITY] = {sizeof(address[0]), sizeof(address[1])},
    sockets[CLIENTS_QUANTITY] = {0};
unsigned short
    ports[CLIENTS_QUANTITY] = {PORT_1, PORT_2};

char opt = '1';

package **package_data;
HANDLE
send_receive_thread[CLIENTS_QUANTITY] = {NULL, NULL};

void init()
{

    package_data = (package **)malloc(CLIENTS_QUANTITY * sizeof(package *));
    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
    {
        /* Создание сокетов типа TCP. */
        if (((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        /* Установка опций для сокетов. */
        if (setsockopt(sockets[i], SOL_SOCKET,
                       SO_REUSEADDR, &opt,
                       sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        /* Установка значений IP-адреса и портов. */
        address[i].sin_family = AF_INET;
        address[i].sin_addr.s_addr = INADDR_ANY;
        address[i].sin_port = htons(ports[i]);

        if ((bind(sockets[i], (struct sockaddr *)&address[i],
                  sizeof(address[i])) < 0))
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        /* Выделение памяти под структуру пакета для приема данных из сокета в отдельной нити*/
        package_data[i] = (package *)malloc(sizeof(package));
    }
}
void set_connect()
{

    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
    {
        if ((listen(sockets[i], 3) < 0))
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        /* Установка соединения и создание сокетов для передачи и приема данных. */
        package_data[i]->address = address[i];
        package_data[i]->addrlen = addrlen[i];
        package_data[i]->sock = sockets[i];

        if (((package_data[i]->recv_socket = accept(sockets[i], (struct sockaddr *)&address[i],
                                                    &addrlen[i])) < 0))
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        /* Создание и запуск нитей для параллельного приема и передачи данных с клиентских компьютеров. */
        if (send_receive_thread[i] == NULL)
            send_receive_thread[i] = CreateThread(NULL, 0, recieve_message, (void *)package_data[i], 0, NULL);
    }
    WaitForMultipleObjects(2, send_receive_thread, TRUE, INFINITE);
}

void send_message(const char *data)
{
    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
        send(package_data[i]->recv_socket, data, BUFFER_SIZE, 0);
}
unsigned long recieve_message(void *arg)
{
    /* Массивы для хранения сообщения и имени пользователя. */
    char
        username[255],
        message[BUFFER_SIZE];
    /* Переменные для хранения количества прочитанных байт и кода ошибки. */
    int valread = -1,
        error_code = 0;
    /* Присвоение адреса структуры пакета.*/
    package *p = (package *)arg;
    recv(p->recv_socket, username, 255, 0);
    printf("%s подключен.\n", username);
    send_message(username);
    /* Бесконечный цикл для приема и передачи сообщений по сети. */
    while (1)
    {

        valread = recv(p->recv_socket, message, BUFFER_SIZE, 0);
        /*
            Обработка исключений через переменную,
            хранящую кол-во прочитанных из сокета байт.
        */
        if (valread <= 0)
        {
            error_code = WSAGetLastError();
            switch (error_code)
            {
            case WSAECONNRESET:
                puts("Существующее соединение было принудительно завершено удаленным узлом.");
                break;
            case 0:
                printf("Соединение с %s потеряно.\n", username);

                break;
            default:
                printf("Непредвиденная ошибка.\nКод: %d\n", error_code);
                break;
            }
            if ((p->recv_socket = accept(p->sock, (struct sockaddr *)&p->address,
                                         &p->addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }
        /*
            Передача полученного сообщения с первого компьютера всем клиентам,
            подключенным к серверу. Последующая очистка буфера приема.
        */
        else if (valread > 0)
        {
            printf("%s: %s\n", username, message);
            memset(p->data, 0, BUFFER_SIZE);
            sprintf(p->data, "%s", message);
            send_message(p->data);
            memset(p->data, 0, BUFFER_SIZE);
            rewind(stdout);
        }
    }
    return 0;
}
