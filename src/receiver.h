/*
    Интерфейс сервера для приема и передачи данных по протоколу TCP/IP.
*/

#include <winsock2.h>
#include <unistd.h>
#define PORT 1000
#define CLIENTS_QUANTITY 3
#define BUFFER_SIZE 4096
/*
    Структура пакета с полем для данных
    и сокетом для отправки этих данных.
*/
typedef struct user
{
    struct sockaddr_in address;
    int listen_socket;
    int recv_socket;
    int addrlen;
    char username[255];
    char answer[BUFFER_SIZE];
} user;

/* Инициализация сервера. */
void init();
/* Функция установки соединения с клиентскими компьютерами. */
void set_connect();
/*
    .
    Читает сообщения из сокета и выводит содержимое в консоль.
    @param arg указатель на структуру пакета,
    содержащую сокет для чтения и буфер для прочитанных данных
*/
unsigned long send_reveive_protocol(void *arg);