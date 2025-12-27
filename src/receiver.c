
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "receiver.h"
struct sockaddr_in address = {0};

int
    addrlen = sizeof(address),
    listen_socket = 0,
    client_counter = 0;

char opt = '1';
const char protocol_commands[][BUFFER_SIZE] = {
    "MT_CONNECT",
    "MT_SEND_TO",
    "MT_SEND_ALL",
    "MSG_TEXT",
    "MSG_FILE",
    "ERR"};
user user_database[CLIENTS_QUANTITY] = {0};
HANDLE
send_receive_thread[CLIENTS_QUANTITY] = {NULL};

void init()
{
    /* Создание сокетов типа TCP. */
    if (((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0))
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    /* Установка опций для сокетов. */
    if (setsockopt(listen_socket, SOL_SOCKET,
                   SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    /* Установка значений IP-адреса и портов. */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if ((bind(listen_socket, (struct sockaddr *)&address,
              sizeof(address)) < 0))
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}
void set_connect()
{

    if ((listen(listen_socket, 3) < 0))
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while ((client_counter) <= CLIENTS_QUANTITY)
    {

        /* Установка соединения и создание сокетов для передачи и приема данных. */
        user_database[client_counter].address = address;
        user_database[client_counter].addrlen = addrlen;
        user_database[client_counter].listen_socket = listen_socket;

        if (((user_database[client_counter].recv_socket = accept(listen_socket, (struct sockaddr *)&address, &addrlen)) < 0))
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (client_counter == CLIENTS_QUANTITY)
            puts("Сервер не может принять больше клиентов.");
        else
        {
            /* Создание и запуск нитей для параллельного приема и передачи данных с клиентских компьютеров. */
            send_receive_thread[client_counter] =
                CreateThread(NULL, 0,
                             send_reveive_protocol,
                             (void *)&user_database[client_counter],
                             0,
                             NULL);
            client_counter++;
        }
    }
}

void send_user_list()
{
    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
    {
        printf("%s\n", user_database[i].username);
        sprintf(user_database[i].answer, "%s|", protocol_commands[0]);
        for (size_t j = 0; j < CLIENTS_QUANTITY; j++)
        {
            if (user_database[j].username[0] == '\0')
                continue;
            strcat(user_database[i].answer, user_database[j].username);
            strcat(user_database[i].answer, "|");
        }
        send(user_database[i].recv_socket, user_database[i].answer, BUFFER_SIZE, 0);
        memset(user_database[i].answer, 0, BUFFER_SIZE);
    }
}
unsigned long send_reveive_protocol(void *arg)
{
    /* Массивы для хранения сообщения и имени пользователя. */
    char
        request[BUFFER_SIZE],
        *command,      // Тип отправки на клиент.
        *message_type, // Тип отправляемого сообщения.
        *message,      // Сообщение (имя файла).
        *username;     // Имя пользователя для MT_SEND_TO
    command = (char *)malloc(BUFFER_SIZE * sizeof(char));
    message_type = (char *)malloc(BUFFER_SIZE * sizeof(char));
    message = (char *)malloc(BUFFER_SIZE * sizeof(char));
    username = (char *)malloc(255 * sizeof(char));
    char *token = NULL;
    /* Переменные для хранения количества прочитанных байт и кода ошибки. */
    int valread = -1,
        error_code = 0;
    /* Присвоение адреса структуры пакета.*/
    user *p = (user *)arg;
    p->index = client_counter;

    /* Бесконечный цикл для приема и передачи сообщений по сети. */
    while (1)
    {

        if ((valread = recv(p->recv_socket, request, BUFFER_SIZE, 0)) > 0)
        {
            char *token = strtok(request, "|");
            strcpy(command, token);
            token = strtok(NULL, "|");
            strcpy(message_type, token);
            /* Подключение к серверу. MT_CONNECT */
            if (strcmp(command, protocol_commands[0]) == 0)
            {
                puts(command);
                token = strtok(NULL, "|");
                strcpy(p->username, token);
                printf("Подключено: %d/%d\n", client_counter, CLIENTS_QUANTITY);
                send_user_list();
            }
            /* Отправка другому клиенту. MT_SEND_TO */
            else if (strcmp(command, protocol_commands[1]) == 0)
            {
                puts(command);
                /* Раздел полученного запроса по разделителям. */
                token = strtok(NULL, "|");
                strcpy(username, token); // Получение имени пользователя.

                /* Отправка сообщения. MSG_TEXT */
                if (strcmp(message_type, protocol_commands[3]) == 0)
                {
                    token = strtok(NULL, "|");
                    strcpy(message, token); // Получение сообщения.
                    puts(message_type);
                    /* Поиск клиента в базе данных. */
                    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
                    {
                        if (strcmp(username, user_database[i].username) == 0)
                        {
                            /* Отправка ответа клиенту. */
                            sprintf(p->answer, "%s|%s(private)|%s", message_type, p->username, message);
                            if (send(user_database[i].recv_socket, p->answer, BUFFER_SIZE, 0))
                                printf("%s -> %s\n", p->username, user_database[i].username);
                        }
                    }
                    if (p->answer[0] == 0)
                    {
                        sprintf(p->answer, "%s|%s", protocol_commands[5], protocol_commands[5]);
                        send(p->recv_socket, p->answer, BUFFER_SIZE, 0);
                    }
                }
                /* Отправка файла. MSG_FILE */
                else if (strcmp(message_type, protocol_commands[4]) == 0)
                {
                    puts(message_type);
                    token = strtok(NULL, "|");
                    strcpy(message, token); // Получение сообщения.
                    char send_folder_name[255];
                    struct timeval tv;
                    int s1[1] = {p->recv_socket};
                    fd_set readfds;
                    FD_SET(p->recv_socket, &readfds);
                    tv.tv_sec = 1;
                    tv.tv_usec = 0;
                    int vr = 0;
                    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
                    {
                        /* Поиск клиента в базе данных. */
                        if (strcmp(username, user_database[i].username) == 0)
                        {
                            sprintf(p->answer, "%s|%s|%s", message_type, p->username, message);

                            /* Отправка файла клиенту. */
                            sprintf(send_folder_name, "%s-%s", p->username, user_database[i].username);
                            mkdir(send_folder_name);
                            char open_folder[255];
                            sprintf(open_folder, "./%s/%s", send_folder_name, message);
                            FILE *f = fopen(open_folder, "wb+");

                            printf("%s -> %s\n", p->username, open_folder);
                            while (select(1, &readfds, NULL, NULL, &tv) != -1)
                            {
                                if (FD_ISSET(p->recv_socket, &readfds))
                                {
                                    vr = recv(p->recv_socket, message, BUFFER_SIZE, 0);
                                    fwrite(message, sizeof(char), vr, f);
                                }
                            }
                            fclose(f);
                            f = fopen(open_folder, "rb");
                            send(user_database[i].recv_socket, p->answer, BUFFER_SIZE, 0);
                            vr = -1;
                            printf("%s -> %s\n", p->username, user_database[i].username);
                            while (vr)
                            {
                                vr = fread(message, sizeof(char), BUFFER_SIZE, f);
                                send(user_database[i].recv_socket, message, vr, 0);
                            }
                        }
                    }
                    if (p->answer[0] == 0)
                    {
                        sprintf(p->answer, "%s|%s", protocol_commands[5], protocol_commands[5]);
                        send(p->recv_socket, p->answer, BUFFER_SIZE, 0);
                        while (select(1, &readfds, NULL, NULL, &tv) != -1)
                        {
                            if (FD_ISSET(p->recv_socket, &readfds))
                            {
                                vr = recv(p->recv_socket, message, BUFFER_SIZE, 0);
                                memset(message, 0, BUFFER_SIZE);
                            }
                        }
                    }
                }
            }
            /* Широковещательная отправка. MT_SEND_ALL */
            else if (strcmp(command, protocol_commands[2]) == 0)
            {
                puts(command);
                /* Отправка сообщения всем клиентам. MSG_TEXT */
                if (strcmp(message_type, protocol_commands[3]) == 0)
                {
                    /* Раздел полученного сообщения по разделителям. */
                    token = strtok(NULL, "|");
                    strcpy(message, token);
                    puts(message_type);
                    sprintf(p->answer, "%s|%s(broadcast)|%s", message_type, p->username, message);
                    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
                    {
                        if (send(user_database[i].recv_socket, p->answer, BUFFER_SIZE, 0))
                            printf("%s -> %s\n", p->username, user_database[i].username);
                        else
                        {
                            error_code = WSAGetLastError();
                            switch (error_code)
                            {
                            default:
                                printf("Ошибка при отправке сообщения. Код %d", error_code);
                                break;
                            }
                        }
                    }
                }
                /* Отправка файла всем клиентам. MSG_FILE*/
                else if (strcmp(message_type, protocol_commands[4]) == 0)
                {
                    token = strtok(NULL, "|");
                    /* Получение сообщения. */
                    strcpy(message, token);
                    /* Формирование ответа. */
                    sprintf(p->answer, "%s|%s|%s", message_type, p->username, message);
                    /*
                        Структура с таймером ожидания сокета,
                        чтобы избежать остановку программы функцией recv().
                    */
                    struct timeval tv;
                    int s1[1] = {p->recv_socket};
                    fd_set readfds;
                    FD_SET(p->recv_socket, &readfds);
                    tv.tv_sec = 1;
                    tv.tv_usec = 0;

                    /* Создание каталога для промежуточного хранения файлов. */
                    char open_folder[255];
                    puts(protocol_commands[4]);
                    mkdir("broadcast_send");
                    sprintf(open_folder, "./broadcast_send/%s", message);
                    FILE *f = fopen(open_folder, "wb+");
                    int bytes_received = 0;
                    /* Отправка сообщения клиенту. */
                    printf("%s -> %s\n", p->username, open_folder);
                    while (1)
                    {
                        /* Проверка наличия данных в сокете без прерываний. */
                        if (select(1, &readfds, NULL, NULL, &tv) == -1)
                            break;
                        else if (FD_ISSET(p->recv_socket, &readfds))
                        {
                            bytes_received = recv(p->recv_socket, message, BUFFER_SIZE, 0);

                            /* Двоичная запись в файл полученных из сокета данных. */
                            fwrite(message, sizeof(char), bytes_received, f);
                        }
                    }
                    fclose(f);

                    for (size_t i = 0; i < CLIENTS_QUANTITY; i++)
                    {
                        bytes_received = -1;
                        if (user_database[i].recv_socket == p->recv_socket)
                            continue;

                        /* Отправка ответа на все подключенные клиенты. */
                        send(user_database[i].recv_socket, p->answer, BUFFER_SIZE, 0);

                        f = fopen(open_folder, "rb");
                        printf("%s -> %s\n", p->username, user_database[i].username);
                        while (bytes_received)
                        {
                            bytes_received = fread(message, sizeof(char), BUFFER_SIZE, f);
                            /* Двоичное чтение из файла и отправка на клиент. */
                            send(user_database[i].recv_socket, message, bytes_received, 0);
                        }
                        fclose(f);
                    }
                }
            }
            else
                printf("%s неизвестная команда протокола.\n", command);
        }
        else if (valread <= 0)
        {

            error_code = WSAGetLastError();
            switch (error_code)
            {
            case 0:
                printf("Соединение с %s потеряно.\n", p->username);
                break;
            default:
                printf("Неизвестная ошибка, код %d.\n", error_code);
                break;
            }
            client_counter--;
            memset(p->username, 0, 255);
            send_user_list();
            break;
        }
        memset(request, 0, BUFFER_SIZE);
        memset(command, 0, BUFFER_SIZE);
        memset(message_type, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);
        memset(p->answer, 0, BUFFER_SIZE);
    }

    free(command);
    free(message_type);
    free(message);
    free(username);
    return 0;
}
