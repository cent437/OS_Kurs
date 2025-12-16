/*
    Интерфейс сервера для приема и передачи данных по протоколу TCP/IP.
*/

#include <winsock2.h>
#include <unistd.h>
#define PORT_1 1000
#define PORT_2 1001
#define CLIENTS_QUANTITY 2
#define BUFFER_SIZE 1024
/*
    Структура пакета с полем для данных
    и сокетом для отправки этих данных.
*/
typedef struct package
{
    struct sockaddr_in address;
    int sock;
    int recv_socket;
    int addrlen;
    char data[BUFFER_SIZE];
} package;

/* Инициализация сервера. */
void init();
/* Функция установки соединения с клиентскими компьютерами. */
void set_connect();

/*
    Функция отправки сообщений.
    Отправляет сообщения всем подключенным сокетам в структуре package.
    @param data указатель на буфер с данными для отправки.
*/
void send_message(const char *data);
/*
    Функция получения сообщений.
    Читает сообщения из сокета и выводит содержимое в консоль.
    @param arg указатель на структуру пакета,
    содержащую сокет для чтения и буфер для прочитанных данных
*/
unsigned long recieve_message(void *arg);