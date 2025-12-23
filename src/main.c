#include "receiver.h"
#include <stdio.h>
#include <locale.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "ru_RU");
    WSADATA wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    init();
    set_connect();

    return 0;
}