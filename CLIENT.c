#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
// for IP printing
#include <netdb.h>
#include <ifaddrs.h>

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_MESSAGE_LENGTH 1024
#define ENDPOINTPORT 3592
#define NI_MAXHOST 1025
#define NI_NUMERICHOST 1
void intro()
{
    system("clear");
    printf("$$$$$$\\  $$\\ $$\\                      $$\\ \n");
    printf("$$  __$$\\ $$ |\\__|                     $$ |  \n");
    printf("$$ /  \\__|$$ |$$\\  $$$$$$\\  $$$$$$$\\ $$$$$$\\ \n");
    printf("$$ |      $$ |$$ |$$  __$$\\ $$  __$$\\_$$  _|  \n");
    printf("$$ |      $$ |$$ |$$$$$$$$ |$$ |  $$ | $$ |  \n");
    printf("$$ |  $$\\ $$ |$$ |$$   ____|$$ |  $$ | $$ |$$\\ \n");
    printf("\\$$$$$$  |$$ |$$ |\\$$$$$$$\\ $$ |  $$ | \\$$$$  | \n");
    printf("\\______/ \\__|\\__| \\_______|\\__|  \\__|  \\____/       SP Project\n");
}
void File_Request_Handler(int socket)
{ // Server File Request Handler
    while (1)
    {
        char buff[255];
        int br = read(socket, buff, 255); // Server: Receives the request
        if (br < 0)
        {
            perror("FIALED to read : ");
            return;
        }
        // Server: Sends the list of contents of default directory to the client
        if (strcmp(buff, "listdirent") == 0)
        {
            DIR *dirp = opendir(".");
            struct dirent *direntp;
            char direntcontent[512] = "";

            while ((direntp = readdir(dirp)) != NULL)
            {
                if (direntp->d_name[0] == '.' || direntp->d_name == "..")
                    continue;
                strcat(direntcontent, "    > ");
                strcat(direntcontent, direntp->d_name);
                strcat(direntcontent, "\n");
            }

            int wr = write(socket, direntcontent, strlen(direntcontent) + 1);
            if (wr < 0)
            {
                perror("FAILED to write : ");
                return;
            }
            sleep(3);
        }
        else //	Server: Receives the request having dir entry name.
        {
            printf("[*] Request from Client : %s \n", buff);
            struct stat stat_buff;
            int r = stat(buff, &stat_buff);
            // a.  If the requested item is a file, server first responds with “Sending file”
            if (S_ISREG(stat_buff.st_mode))
            {
                int br = write(socket, "Sending file", 13);
                if (br < 0)
                {
                    perror("FAILED to write : ");
                    return;
                }
                // sends the contents of the files after a pause of 5 seconds and go back to the default directory
                sleep(5);
                char fileBuff[1024];
                int requestedFile_fd = open(buff, O_RDONLY);
                if (requestedFile_fd == -1)
                {
                    printf("[-] Exiting File Handling Mode. \n");
                    return;
                }
                int br1 = read(requestedFile_fd, fileBuff, 1024);
                if (br1 == -1)
                {
                    perror("FAILED to read from requested file: ");
                    return;
                }
                write(socket, fileBuff, br1);
                sleep(2);
                chdir(".");
                printf("[-] Exiting File Handling Mode.\n");
                break;
            }
            //  b.	If the requested item is a directory, sever first responds with “Listing directory”
            if (S_ISDIR(stat_buff.st_mode))
            {
                write(socket, "Listing directory", 18);
                sleep(5);
                // c.	Server opens the requested directory and goes back to step 5 after a pause of 5 seconds
                DIR *dirp = opendir(buff);
                struct dirent *direntp;
                char direntcontent[512] = "";
                while ((direntp = readdir(dirp)) != NULL)
                {
                    if (direntp->d_name[0] == '.' || direntp->d_name == "..")
                        continue;
                    strcat(direntcontent, "    > ");
                    strcat(direntcontent, direntp->d_name);
                    strcat(direntcontent, "\n");
                }
                int wr = write(socket, direntcontent, strlen(direntcontent) + 1);
                if (wr < 0)
                {
                    perror("FAILED to write : ");
                    return;
                }
                sleep(2);
            }
            if (!S_ISDIR(stat_buff.st_mode) && !S_ISREG(stat_buff.st_mode))
            {
                printf("[-] Exiting File Handling Mode.\n");
                break;
            }
        }
    }
}

void File_Request(int fd)
{ // Client Request File.

    // Client: Sends a request for list of available files and directories
    char req_buffer[] = "listdirent";
    write(fd, req_buffer, sizeof(req_buffer));
    sleep(1);
    char response_buf[512];
    int br;
LISTDIR:
    // Client: Receives the list of files and directories and
    br = read(fd, response_buf, 512);
    if (br == -1)
    {
        perror("FAIELD to read : ");
        return;
    }
    printf("[*] List : \n%s", response_buf);

    // select a name and request the server for its contents
    char name[10];
    printf("[*] Enter a file name/path : ");
    scanf("%s", name);
    write(fd, name, strlen(name) + 1);
    sleep(1);

    // Client: Receives the response from the server
    char res_buff[20];
    read(fd, res_buff, 20);
    printf("[*] Server is %s.\n", res_buff);

    // If the response received is “Listing directory”, go back step 6
    if (!strcmp(res_buff, "Listing directory"))
    {
        goto LISTDIR;
    }
    // If the response received is “Sending file”, receive the contents and save them into a new file
    if (!strcmp(res_buff, "Sending file"))
    {
        char FileContent[1024];
        int br;

        sleep(3);
        int br1 = read(fd, FileContent, 1024);
        if (br1 == -1)
        {
            perror("FAIELD to read from fifo: ");
            return;
        }
        printf("[*] File Contents : \n\t-> %s", FileContent);
        printf("[*] File is recieved.\n[*] Exiting File Requesting Mode\n");
        write(fd, "ESC", strlen("ESC") + 1);
    }
}

void start_chatting(int socket)
{
    char message[1024];
    fd_set readfds;
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(socket, &readfds);
        int result = select(socket + 1, &readfds, NULL, NULL, NULL);
        if (result == -1)
        {
            printf("Error selecting\n");
            return;
        }
        if (FD_ISSET(0, &readfds))
        {
            memset(message, 0, sizeof(message));
            fgets(message, 1024, stdin);
            if (strcmp(message, "exit\n") == 0)
            {
                send(socket, message, strlen(message), 0);
                break;
            }
            send(socket, message, strlen(message), 0);
        }
        if (FD_ISSET(socket, &readfds))
        {
            memset(message, 0, sizeof(message));
            if (recv(socket, message, 1024, 0) == 0)
            {
                printf("Server disconnected\n");
                break;
            }
            else if (!strcmp(message, "exit\n"))
            {
                break;
            }
            printf("Received message: %s\n", message);
        }
    }
}

void Server_Chat(int socket)
{
    while (1)
    {
        printf("\n[*] Waiting for User Input...\n");
        int Client_choice;
        read(socket, &Client_choice, sizeof(int));
        if (Client_choice == 1)
        {
            printf("[+] Chatting Mode Started!\n");
            start_chatting(socket);
            printf("[-] Chatting Ended!\n");
            system("clear");
            intro();
        }
        else if (Client_choice == 2)
        {
            printf("[*] Handling File Request . . .\n");
            File_Request_Handler(socket);
        }
        else if (Client_choice == 3)
        {
            printf("[-] Exiting!\n");
            close(socket);
            exit(0);
        }
        else
        {
            printf("[-] Invalid Entry!\n");
        }
    }
}
void Client_Chat(int socket)
{
    while (1)
    {
        printf("[*] Enter your choice:\n\t 1. Chat \n\t 2. Request File\n\t 3. Exit\n\t -> Enter Choice : ");
        int choice = 0;
        scanf("%d", &choice);
        if (choice == 1) // Chat
        {
            printf("[+] Chatting Mode Started!\n");
            write(socket, &choice, sizeof(int));
            start_chatting(socket);
            printf("[-] Chatting Ended!\n");
            system("clear");
            intro();
        }
        else if (choice == 2) // File
        {
            write(socket, &choice, sizeof(int));
            printf("[*] Request File Contents . . .\n");
            File_Request(socket);
        }
        else if (choice == 3)
        {
            write(socket, &choice, sizeof(int));
            printf("[-] Exiting the client\n");
            close(socket);
            exit(0);
        }
        else
        {
            printf("[-] Invalid choice.\n");
        }
    }
}

char IP[14];

int printIP()
{
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
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

            if (strcmp(ifa->ifa_name, "lo"))
            {
                // printf("Interface: %s\n", ifa->ifa_name);
                // printf("IP Address: %s\n", host);
                strcpy(IP, host);
                return 0;
            }
        }
    }
    freeifaddrs(ifaddr);
}

int main(int agrc, char *argv[])
{
    if (agrc == 1)
    {
        printf("Usage Error!\n%s <IP Address of Server>", argv[0]);
        return EXIT_FAILURE;
    }
    intro();
    printf("\n[+] Cleint Strated Successfully \n");
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
            write(netwrok_socket, &choice, sizeof(int));
            sleep(1);
            int port = ENDPOINTPORT;
            write(netwrok_socket, &port, sizeof(int));
            if (!printIP())
            {
                int wr;
                if ((wr = write(netwrok_socket, IP, strlen(IP) + 1)) == -1)
                {
                    perror("FAiled to write: ");
                    return -1;
                }
                printf("IP : %s\n", IP);
            }
            else
            {
                printf("[-] FAILED to retrieve IP!\n");
            }
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
            // Enter ip/port of another user for P2P Communication.
            int P2P_socket;
            P2P_socket = socket(PF_INET, SOCK_STREAM, 0);

            // prompt user to act as client or server, this should be mutually exclusive.
            int MODE;
            printf("[*] Act as Server - 0, Client - 1 : ");
            scanf("%d", &MODE);
            if (MODE == 0) // this is for server
            {
                struct sockaddr_in P2P_address;
                P2P_address.sin_family = PF_INET;
                P2P_address.sin_port = htons(ENDPOINTPORT);
                P2P_address.sin_addr.s_addr = inet_addr(IP);

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

                char ip_address[15];
                printf("[*] Enter IP Address of User : ");
                scanf("%s", ip_address);
                int port_nmbr;
                printf("[*] Enter PORT Number : ");
                scanf("%d", &port_nmbr);

                struct sockaddr_in P2P_address;
                P2P_address.sin_family = PF_INET;
                P2P_address.sin_port = htons(port_nmbr);
                P2P_address.sin_addr.s_addr = inet_addr(ip_address);

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