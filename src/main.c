#include "receiver.h"
#include <stdio.h>
#include <locale.h>

int main(int argc, char const *argv[])
{
    system("chcp 65001");
#ifdef _WIN32
    WSADATA wsaData = {0};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    init();
    set_connect();

    return 0;
}