#include <winsock2.h>
#include <unistd.h>
#define PORT_1 1000
#define PORT_2 1001
typedef struct user
{
    size_t passwd;
    char user[256];

} user;

void init();
void set_connect();
void send_message();
void recieve_message();