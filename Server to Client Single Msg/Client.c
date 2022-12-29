#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int agrc, char *argv[])
{
    int netwrok_socket;
    // int socket(int __domain, int __type, int __protocol) - Returns fd or -1

    netwrok_socket = socket(PF_INET, SOCK_STREAM, 0);
    // 0 - default protocol
    // SOCK_STEAM - TCP
    // AF_INET / PF_INET - IPV4

    if (netwrok_socket == -1)
    {
        perror("FAILED to create socket: ");
        return -1;
    }

    // specifying end point for client or adress for socket
    struct sockaddr_in server_address;
    server_address.sin_family = PF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY; // Address to accept any incoming messages.

    int connection_status = connect(netwrok_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    // int connect(int __fd, const struct sockaddr *__addr, socklen_t __len)
    // Open a connection on socket FD to peer at ADDR (which LEN bytes long).
    // For connectionless socket types, just set the default address to send to
    // and the only address from which to accept transmissions.
    // Return 0 on success, -1 for errors.

    if (connection_status == -1)
    {
        perror("FAILED to estabslihed a connection with endpoint : ");
        return -1;
    }

    char server_response[257];
    recv(netwrok_socket, &server_response, sizeof(server_response), 0);
    printf("%s\n",server_response);
    close(netwrok_socket);
    return 0;
}