#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h> // for hostname 

#define PORT 12345              // Port number to use for the server
#define MAX_CLIENTS 5           // Maximum number of clients that can be connected at a time
#define MAX_MESSAGE_LENGTH 1024 // Maximum length of a message that can be received from a client
#define FAILURE -1

// Structure to store Client Information
typedef struct
{
    int sockfd;
    struct sockaddr_in address;
} client_t;

int main(int argc, char *argv[])
{
    printf("[*] Server has Started.\n");
    // Create Array to store Connected Clients Information
    client_t Connected_Clients[MAX_CLIENTS];
    memset(Connected_Clients, 0, sizeof(Connected_Clients));

    // Create a socket for server where server will be listening to requests
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("[-] FAILED to create server socket");
        return FAILURE;
    }
    else
    {
        printf("[*] Server has successfully created socket with fd : %d\n", server_socket);
    }

    // creating default server object for server address i.e socket address in
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT); // htons to convert into network byte order

    // Bind the created socket (fd) with the specified port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("[-] FAILED to bind socket with PORT");
        return FAILURE;
    }
    else
    {
        printf("[*] Server has binded socket : %d with PORT : %d\n", server_socket, PORT);
    }

    // Start Listening for incoming connections
    // MAX_CLEINTS backlog:  it specifies the queue length for completely established sockets waiting to be accepted
    //      or
    // Maximum number of clients that can be connected at a time
    int listen_status = listen(server_socket, MAX_CLIENTS);
    if (listen_status == -1)
    {
        perror("[-] FAILED to LISTEN for connection requests");
        return FAILURE;
    }
    else
    {
        printf("[*] Server is listening on PORT : %d\n", PORT);
    }

    // set up `select()` function
    fd_set myreadset;

    char client_message[MAX_MESSAGE_LENGTH];
    char server_message[MAX_MESSAGE_LENGTH];

    struct timeval time;
    time.tv_sec = 13;
    time.tv_usec = 0;
    int MAX_FD = server_socket;

    // for getting hostname
    struct hostent *host;

    socklen_t sin_size;                // unsigned int - required for client address handling
    struct sockaddr_in client_address; // to store information of client requesting the server.
    while (1)
    {
        FD_ZERO(&myreadset);
        FD_SET(server_socket, &myreadset);

        // Add the socket file descriptors for all the connected clients to the set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (Connected_Clients[i].sockfd > 0)
            {
                FD_SET(Connected_Clients[i].sockfd, &myreadset);
            }
        }

        // Use the `select()`
        int N = select(MAX_FD + 1, &myreadset, NULL, NULL, &time);
        if (N == -1)
        {
            perror("[-] FAILED to Monitor Sockets");
            return FAILURE;
        }
        else
        {
            printf("[*] Server socket is ready.\n");
        }
        if (N == 0)
        {
            printf("[*] TIMEOUT occured, No Clients are avaialeble.\n");
            printf("[*] Exiting the server ...\n[*] Server is Exited.\n");
            return 0;
        }
        if (N > 0)
        {
            int client_socket;
            sin_size = sizeof(client_address);
            client_socket = accept(server_socket, (struct sockaddr *)&client_address, &sin_size);
            if (client_socket == -1)
            {
                perror("[-] FAILED to accept client");
                return FAILURE;
            }
            else
            {   
                host = gethostbyaddr((const void *)&client_address.sin_addr, sizeof(client_address.sin_addr), AF_INET);
                printf("[*] Client Accepted. New Client Socket : %d \n", client_socket);
                printf("[*] Connection from '%s' - %s:%d\n",host->h_name, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                // char port[5];
                // sprintf(port, "%d", ntohs(client_address.sin_port));
                send(client_socket, "This is Server", sizeof("This is Server"), 0);

                // Till here. The client IP and port is recieved from client.
                // Now, Code for handing it over to client for peer to peer connection and
                //  other stuff should be done from here.
            }
            sleep(5);
        }
    }

    return 0;
}