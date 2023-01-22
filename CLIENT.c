#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MESSAGE_LENGTH 1024
#define ENDPOINTPORT 3592

void Server_Chat(int socket)
{
    char buf[5];
    read(socket, buf, 5);
    printf("Message : %s\n", buf);
    write(socket, "Hi", strlen("Hi") + 1);
}
void Client_Chat(int socket)
{
    char *message;
    message = "Hello";
    write(socket, message, strlen(message) + 1);
    sleep(1);
    read(socket, message, strlen("Hi")+1);
    printf("Message from Client : %s", message);
}
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
    server_address.sin_addr.s_addr = inet_addr(argv[1]); // Address to accept any incoming messages.

    int connection_status = connect(netwrok_socket, (struct sockaddr *)&server_address, sizeof(server_address));
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
    else
    {
        printf("[*] Connected to Indexer Service.\n");
    }

    char message[MAX_MESSAGE_LENGTH];
    char server_response[257];
    int choice = 7;
    while (1)
    {
        if (choice == 7) // to send endpoint to server.
        {
            // for now just port since its a localhost
            write(netwrok_socket, &choice, sizeof(int));
            sleep(1);
            int port = ENDPOINTPORT;
            write(netwrok_socket, &port, sizeof(int));
        }
        printf("[*] Press\n\t1. See Online Users\n\t2. Connect to an Endpoint.\n\t3. Exit\n\t-> Enter Choice : ");
        scanf("%d", &choice);
 
        if (choice == 1)
        {
            write(netwrok_socket, &choice, sizeof(int));
            sleep(1);
            char LIST[MAX_MESSAGE_LENGTH];
            read(netwrok_socket, LIST, MAX_MESSAGE_LENGTH);
            printf("[*] Online Users\n%s", LIST);
            continue;
        }
        else if (choice == 2)
        {
            close(netwrok_socket);
            int port_nmbr;
            // Send a specific identifier to get relevant address from client
            printf("[*] Enter PORT Number : ");
            scanf("%d", &port_nmbr);
            int P2P_socket;
            P2P_socket = socket(PF_INET, SOCK_STREAM, 0);
            struct sockaddr_in P2P_address;
            P2P_address.sin_family = PF_INET;
            P2P_address.sin_port = htons(port_nmbr);
            P2P_address.sin_addr.s_addr = inet_addr(argv[1]);

            // prompt user to act as client or server, this should be mutually exclusive.
            printf("[*] Act as Server (0) : ");
            scanf("%d", &port_nmbr);
            if (port_nmbr == 0) // this is for server
            {
                // bind the socket to localhost port
                if (bind(P2P_socket, (struct sockaddr *)&P2P_address, sizeof(P2P_address)) < 0)
                {
                    perror("[-] bind FAILED");
                    return EXIT_FAILURE;
                }
                printf("[*] Listener on port %d \n", ENDPOINTPORT);

                if (listen(P2P_socket, 1) < 0)
                {
                    perror("[-] listen FAILED");
                    exit(EXIT_FAILURE);
                }
                int client_socket;
                if ((client_socket = accept(P2P_socket, NULL, NULL)) < 0)
                {
                    perror("[-] accept FAILED");
                    return EXIT_FAILURE;
                }
                // here loop for chatting
                Server_Chat(client_socket);
            }
            else // for acting as client
            {

                int connection_status_P2P = connect(P2P_socket, (struct sockaddr *)&P2P_address, sizeof(P2P_address));
                if (connection_status_P2P == -1)
                {
                    perror("FAILED to estabslihed a connection with endpoint : ");
                    return -1;
                }
                else
                {
                    printf("[*] Ready for P2P Communication.\n");
                }
                // here loop for chatting
                Client_Chat(P2P_socket);
            }
            break;
        }
        else if (choice == 3)
        {
            // close the conncetion to server if any
            if (connection_status != 0)
                close(netwrok_socket);
            // Exit the Client
            return EXIT_SUCCESS;
        }
        else
        {
            printf("[-] Invalid Entery!\n");
        }
    }
    return 0;
}