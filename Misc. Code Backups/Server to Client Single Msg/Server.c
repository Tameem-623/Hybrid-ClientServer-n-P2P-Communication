#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    char server_message[256] = "This is server Response.\n";
    
    int server_socket;
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        perror("FAIELD to create server socket");
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = PF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(5000);

    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    listen(server_socket, 5);

    int client_socket;

    //  accept command returns tyhe socket of the client.

    client_socket = accept(server_socket, NULL, NULL);

    send(client_socket, server_message, sizeof(server_message),0 );

    close(server_socket);

    return 0;
}