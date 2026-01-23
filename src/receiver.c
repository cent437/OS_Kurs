
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
const char protocol_commands[][REQUEST_SIZE] = {
    "MT_CONNECT",
    "MT_SEND_TO",
    "MT_SEND_ALL",
    "MSG_TEXT",
    "MSG_FILE",
    "ERR"};
user *head = NULL, *tail = NULL;
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
user *create_node()
{
    user *p = NULL;
    if ((p = (user *)malloc(sizeof(user))) == NULL)
    {
        perror("Ошибка выделения памяти.");
        exit(1);
    }
    p->next = NULL;
    p->prev = NULL;
    p->recv_socket = accept(listen_socket, (struct sockaddr *)&address, &addrlen);
    memset(p->username, 0, 255);
    memset(p->answer, 0, REQUEST_SIZE);

    return p;
}
void push_front()
{
    user *p = create_node();
    if (head == NULL && p != NULL)
    {
        head = p;
        tail = p;
    }
    else if (head != NULL && p != NULL) // Если список есть, то добавляем элемент в начало списка
    {
        head->prev = p;
        p->next = head;
        head = p;
    }
    return;
}
void pop_front()
{
    user *p = head;
    if (head == NULL)
        return;
    head = p->next;
    free(p);
    p = head;
    return;
}

void push_back()
{
    user *p = create_node();
    if (head == NULL && p != NULL) // Если списка нет, то добавляем элемент в голову и в хвост
    {
        head = p;
        tail = p;
    }
    else if (head != NULL && p != NULL) // Если список есть, то добавляем элемент в конец списка
    {
        tail->next = p;
        p->prev = tail;
        tail = p;
    }
    return;
}

void pop_back()
{
    user *p = head;
    user *prev;
    if (head == NULL) // Если список пуст, возвращаемся
        return;
    while (p != tail) // Проход до последнего элемента списка с отслеживанием предпоследнего
    {
        prev = p;
        p = p->next;
    }

    if (head == tail) // Проверка на то, что в списке 1 элемент
    {
        head = NULL; // Обнуляем голову и хвост
        tail = NULL;
        return;
    }
    free(p);
    prev->next = NULL;
    tail = prev;
    return;
}

void erase(const char *username)
{
    user *current = head, *previous = head->prev, *next = head->next;
    user *p = NULL;
    if (strcmp(username, head->username) == 0)
    {
        pop_front();
        return;
    }
    if (strcmp(username, tail->username) == 0)
    {
        pop_back();
        return;
    }
    while (strcmp(username, current->username) != 0)
    {
        current = current->next;
        previous = current->prev;
        next = current->next;
    }
    p = current;
    previous->next = next;
    next->prev = previous;
    free(p);
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
        if (client_counter == CLIENTS_QUANTITY)
            continue;
        else
        {
            push_back();
            /* Создание и запуск нитей для параллельного приема и передачи данных с клиентских компьютеров. */
            send_receive_thread[client_counter] = CreateThread(NULL,
                                                               0,
                                                               send_reveive_protocol, tail, 0, NULL);
            client_counter++;
            printf("Подключено: %d/%d\n", client_counter, CLIENTS_QUANTITY);
        }
    }
}
void send_user_list()
{
    user *iter_i = head, *iter_j = head;
    while (iter_i)
    {
        printf("%s\n", iter_i->username);
        sprintf(iter_i->answer, "%s|", protocol_commands[0]);
        while (iter_j)
        {
            strcat(iter_i->answer, iter_j->username);
            strcat(iter_i->answer, "|");
            iter_j = iter_j->next;
        }
        send(iter_i->recv_socket, iter_i->answer, REQUEST_SIZE, 0);
        memset(iter_i->answer, 0, REQUEST_SIZE);
        iter_i = iter_i->next;
        iter_j = head;
    }
}
unsigned long send_reveive_protocol(void *arg)
{

    /* Массивы для хранения сообщения и имени пользователя. */
    char
        request[REQUEST_SIZE],
        *command,      // Тип отправки на клиент.
        *message_type, // Тип отправляемого сообщения.
        *message,      // Сообщение (имя файла).
        *username;     // Имя пользователя для MT_SEND_TO
    command = (char *)malloc(REQUEST_SIZE * sizeof(char));
    message_type = (char *)malloc(REQUEST_SIZE * sizeof(char));
    message = (char *)malloc(BUFFER_SIZE * sizeof(char));
    username = (char *)malloc(255 * sizeof(char));
    /* Переменные для хранения количества прочитанных байт и кода ошибки. */
    int valread = -1,
        error_code = 0;
    int bytes_received = 0, bytes_sent = 0;
    /* Присвоение адреса структуры пакета.*/
    user *p = (user *)arg;
    user *iter = NULL;

    /*
        Структура с таймером ожидания сокета,
        чтобы избежать остановку программы функцией recv().
    */
    struct timeval tv;
    fd_set readfds = {0};
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    /* Бесконечный цикл для приема и передачи сообщений по сети. */
    while (1)
    {
        iter = head;
        if ((valread = recv(p->recv_socket, request, REQUEST_SIZE, 0)) > 0)
        {
            FD_SET(p->recv_socket, &readfds);
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
                printf("%s подключен.\n", p->username);
                sprintf(p->answer, "%s|%s", command, p->username);
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
                    while (iter)
                    {
                        if (strcmp(username, iter->username) == 0)
                        {
                            /* Отправка ответа клиенту. */
                            sprintf(p->answer, "%s|%s(private)|%s", message_type, p->username, message);
                            send(iter->recv_socket, p->answer, REQUEST_SIZE, 0);
                            printf("[%s - %s]\t%s\n", p->username, iter->username, message);
                            break;
                        }
                        iter = iter->next;
                    }
                }
                /* Отправка файла. MSG_FILE */
                else if (strcmp(message_type, protocol_commands[4]) == 0)
                {
                    puts(message_type);
                    token = strtok(NULL, "|");
                    strcpy(message, token); // Получение сообщения.
                    char send_folder_name[255];
                    while (iter)
                    {
                        /* Поиск клиента в базе данных. */
                        if (strcmp(username, iter->username) == 0)
                        {
                            sprintf(p->answer, "%s|%s|%s", message_type, p->username, message);
                            sprintf(send_folder_name, "%s-%s", p->username, iter->username);
                            mkdir(send_folder_name);
                            char open_folder[255];
                            sprintf(open_folder, "./%s/%s", send_folder_name, message);
                            FILE *f = fopen(open_folder, "wb+");
                            /* Отправка файла на сервер. */
                            printf("[%s - %s]\n", p->username, open_folder);
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
                                    printf("Записано %d байт.\n", bytes_received);
                                }
                            }
                            fclose(f);
                            f = fopen(open_folder, "rb");
                            bytes_received = -1;
                            send(iter->recv_socket, p->answer, REQUEST_SIZE, 0);
                            printf("[%s - %s]\n", p->username, iter->username);
                            while (bytes_received)
                            {
                                bytes_received = fread(message, sizeof(char), BUFFER_SIZE, f);
                                if ((bytes_sent = send(iter->recv_socket, message, bytes_received, 0)) > 0)
                                    printf("Отправлено %d байт.\n", bytes_sent);
                            }
                            fclose(f);
                        }
                        iter = iter->next;
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
                    while (iter)
                    {
                        send(iter->recv_socket, p->answer, REQUEST_SIZE, 0);
                        printf("[%s - %s]\t%s\n", p->username, iter->username, message);
                        iter = iter->next;
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
                    /* Создание каталога для промежуточного хранения файлов. */
                    char open_folder[255];
                    puts(protocol_commands[4]);
                    mkdir("broadcast_send");
                    sprintf(open_folder, "./broadcast_send/%s", message);
                    FILE *f = fopen(open_folder, "wb+");

                    /* Отправка файла на сервер. */
                    printf("[%s - %s]\t%s\n", p->username, open_folder, message);
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
                            printf("Записано %d байт.\n", bytes_received);
                        }
                    }
                    fclose(f);
                    while (iter)
                    {
                        bytes_received = -1;
                        /* Отправка ответа на все подключенные клиенты. */
                        send(iter->recv_socket, p->answer, REQUEST_SIZE, 0);
                        printf("[%s - %s]\n", p->username, iter->username);
                        f = fopen(open_folder, "rb");
                        while (bytes_received)
                        {
                            bytes_received = fread(message, sizeof(char), BUFFER_SIZE, f);
                            /* Двоичное чтение из файла и отправка на клиент. */
                            send(iter->recv_socket, message, bytes_received, 0);
                            printf("Отправлено %d байт.\n", bytes_received);
                        }
                        fclose(f);
                        iter = iter->next;
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
            erase(p->username);
            send_user_list();
            break;
        }
        fflush(NULL);
        memset(request, 0, REQUEST_SIZE);
        memset(command, 0, REQUEST_SIZE);
        memset(p->answer, 0, REQUEST_SIZE);
        memset(message_type, 0, REQUEST_SIZE);
        memset(message, 0, BUFFER_SIZE);
    }

    free(command);
    free(message_type);
    free(message);
    free(username);
    return 0;
}
