// Example code: A simple server side code, which echos back the received message.
// Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <unistd.h>

// for IP printing
#include <netdb.h>
#include <ifaddrs.h>

#define TRUE 1
#define FALSE 0
#define PORT 12345
#define MAX_CLIENTS 4
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_LENGTH 1024
#define NI_MAXHOST 1025
#define NI_NUMERICHOST 1

typedef struct
{
    char endpointIP[15];
    int endpointPort;
} ENDPOINT;

typedef struct
{
    char *Name;
    char *IP;
    int port;
    int status;
    ENDPOINT client_EP;
} CLIENTDB;

void CLIENTDB_ZERO(CLIENTDB *Entry)
{
    // Clearning the garbage Value
    Entry->Name = '\0';
    Entry->IP = '\0';
    Entry->port = 0;
    Entry->status = 0;
    Entry->client_EP.endpointPort = 0;
    Entry->client_EP.endpointIP;
}

void CLIENTDB_ADD(CLIENTDB *Entry, char *name, char *ip, int port, int Status)
{
    Entry->Name = name;
    Entry->IP = ip;
    Entry->port = port;
    Entry->status = Status;
}

void printIP()
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return;
    }
    // Iterate through the list of interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }
        family = ifa->ifa_addr->sa_family;

        // Only consider IPv4
        if (family == AF_INET)
        {
            // Get the address as a string
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (!strcmp(ifa->ifa_name, "eth0"))
            {
                // printf("Interface: %s\n", ifa->ifa_name);
                printf("[*] Server IP Address: %s\n", host);
            }
        }
    }
    freeifaddrs(ifaddr);
}

void Terminate_Server(int signal)
{
    printf("\n[*] Terminating the Server ... \n");
    close(PORT);
    printf("[*] Terminated.\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    /* MARK - New Test start, DB code Integration*/
    sigset_t set;
    struct sigaction act = { 0 };
    act.sa_handler = Terminate_Server;
    sigfillset(&set);
    sigaction(SIGINT, &act, NULL);
    sigdelset(&set, SIGINT);

    printIP();
    CLIENTDB Handle_Client[MAX_CLIENTS];
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, i, valread, socket_fd;
    int max_socket_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE]; // data buffer of 1K

    fd_set myreadset; // set of socket descriptors

    char *message = "ECHO Daemon v1.0 \r\n"; // a message

    // initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
        CLIENTDB_ZERO(&Handle_Client[i]);
    }

    // create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("[-] MASTER SOCKET FAILED");
        return EXIT_FAILURE;
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("[-] setsockopt FAILED");
        return EXIT_FAILURE;
    }

    // type of socket created
    address.sin_family = AF_INET;         // IPV4
    address.sin_addr.s_addr = INADDR_ANY; // anyip - local host
    address.sin_port = htons(PORT);

    // bind the socket to localhost port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("[-] bind FAILED");
        return EXIT_FAILURE;
    }
    printf("[*] Listener on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("[-] listen FAILED");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    puts("[*] Waiting for connections ...");

    while (TRUE)
    {
        // clear the socket set
        FD_ZERO(&myreadset);

        // add master socket to set
        FD_SET(master_socket, &myreadset);
        max_socket_fd = master_socket;

        // add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            // socket descriptor
            socket_fd = client_socket[i];
            /* here ig gotta assign values to DB too*/

            // if valid socket descriptor then add to read list
            if (socket_fd > 0)
                FD_SET(socket_fd, &myreadset);

            // highest file descriptor number, need it for the select function
            if (socket_fd > max_socket_fd)
                max_socket_fd = socket_fd;
        }

        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(max_socket_fd + 1, &myreadset, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("[-] select ERROR");
            return EXIT_FAILURE;
        }

        // If something happened on the master socket ,
        // then its an incoming connection
        if (FD_ISSET(master_socket, &myreadset))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("[-] accept FAILED");
                return EXIT_FAILURE;
            }

            // inform user of socket number - used in send and receive commands
            printf("[*] New connection from,\n\t\tClient socket : %d\n\t\tIP : %s\n\t\tPORT : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // send new connection greeting message
            // if (send(new_socket, message, strlen(message), 0) != strlen(message))
            // {
            //     perror("[-] send FAILED");
            // }

            // puts("[*] Welcome message sent successfully");

            // add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    CLIENTDB_ADD(&Handle_Client[i], "Dummy Name", inet_ntoa(address.sin_addr), ntohs(address.sin_port), 1);
                    client_socket[i] = new_socket;
                    printf("[*] Adding to list of sockets as Client : %d\n", i);
                    break;
                }
            }
        }

        // else its some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            socket_fd = client_socket[i];

            if (FD_ISSET(socket_fd, &myreadset))
            {
                int choice;
                // Check if it was for closing , and also read the
                // incoming message
                // if ((valread = read(socket_fd, buffer, 1024)) == 0)
                if ((valread = read(socket_fd, &choice, sizeof(int))) == 0)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(socket_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("[*] Host disconnected,\n\t\tIP : %s\n\t\tPORT : %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    puts("[*] Waiting for connections ...");

                    // here we can decremenet cleint entry ig - TD
                    CLIENTDB_ZERO(&Handle_Client[i]);
                    // Close the socket and mark as 0 in list for reuse
                    close(socket_fd);
                    client_socket[i] = 0;
                }

                // Echo back the message that came in
                else
                {
                    char LIST[MAX_MESSAGE_LENGTH];
                    printf("[*] List of Online Clients:\n");
                    for (int i = 0; i < MAX_CLIENTS; i++)
                    {
                        if (Handle_Client[i].IP == NULL)
                            continue;
                        printf("\t-> Client: %d - IP/PORT : %s:%d\n", i, Handle_Client[i].IP, Handle_Client[i].port);
                    }
                    if (choice == 7)
                    {
                        read(socket_fd, &(Handle_Client[i].client_EP.endpointPort), sizeof(int));
                        read(socket_fd, (Handle_Client[i].client_EP.endpointIP), 15);
                    }

                    if (choice == 1) // to send Online Users to Client
                    {
                        for (int i = 0; i < MAX_CLIENTS; i++)
                        {
                            if (Handle_Client[i].IP == NULL)
                                continue;
                            dprintf(socket_fd, "\t-> Client: %d - ENDPOINT IP/PORT :  %s:%d\n", i, Handle_Client[i].client_EP.endpointIP, Handle_Client[i].client_EP.endpointPort);
                            // sprintf(LIST, "\t-> Client: %d - IP/PORT : %s:%d\n", i, Handle_Client[i].IP, Handle_Client[i].port);
                        }
                    }
                    // else if (choice == 2)   // To recieve the request for client endpoint.
                    {
                        // int user_nmbr;
                        // read(socket_fd, &user_nmbr, sizeof(int));
                        // printf("[*] Client: %d requested for Client: %d endpoint.\n", i, user_nmbr);
                        // if( i == user_nmbr)
                        // {
                        //     printf("[-] Invalid Self Call.\n");
                        //     write(socket_fd, "-1", sizeof(int));
                        // }
                    }
                }
            }
        }
    }

    return 0;
}
