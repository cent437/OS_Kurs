#include <stdio.h>
#include <winsock2.h>

int main()
{
    struct in_addr ip;
    ip.S_un.S_addr = htonl(0xac13451a);
    printf("%d.%d.%d.%d", ip.S_un.S_un_b.s_b1, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b3, ip.S_un.S_un_b.s_b4);
    return 0;
}