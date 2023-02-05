
// Define a structure which will handles entries of the connected clients in the MAIN server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENTS 2

typedef struct
{
    char *Name;
    char *IP;
    int PORT;
    int status;
} CLIENTDB;

void CLIENTDB_ZERO(CLIENTDB *Entry)
{
    // Clearning the garbage Value
    Entry->Name = '\0';
    Entry->IP = '\0';
    Entry->PORT = 0;
    Entry->status = 0;
}

void CLIENTDB_ADD(CLIENTDB *Entry, char *name, char *ip, int port, int Status)
{
    Entry->Name = name;
    Entry->IP = ip;
    Entry->PORT = port;
    Entry->status = Status;
}

int main()
{
    {
        // Discarded
        //     // name IP Port Status
        //     char *CleintBase[4];
        //     CleintBase[1] = "Cleint 1";
        //     CleintBase[2] = "127.0.0.1";
        //     CleintBase[3] = "12345";

        //     printf("%s %s %s\n", CleintBase[1], CleintBase[2], CleintBase[3]);
    }

    // what if i make array of struct, each struct presenting one entry for client

    CLIENTDB ClientDatabase[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++)
    {

        {
            // --- clearing garbage in MAIN FLOW
            // Clearning the garbage Value
            // ClientDatabase[i].Name = '\0';
            // ClientDatabase[i].IP = '\0';
            // ClientDatabase[1].PORT = 0;
            // ClientDatabase[1].status = 0;

            // printf("Enter Name : \n");
            // scanf("%s", ClientDatabase[i].Name);

            // printf("Enter IP : ");
            // scanf("%s", ClientDatabase[i].IP);
        }
        // custom function which can be used to clear struct entry or iniialize struct with zero
        CLIENTDB_ZERO(&ClientDatabase[i]);

        if (i == 1)
        {
            CLIENTDB_ADD(&ClientDatabase[i], "User 1", "192.168.0.50", 8080, 1);
            break;
        }
        printf("Enter PORT : ");
        scanf("%d", &ClientDatabase[i].PORT);

        printf("Enter STATUS : ");
        scanf("%d", &ClientDatabase[i].status);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {

        printf("Client {%d}\n\tName : %s \n", i + 1, ClientDatabase[i].Name);
        printf("\tIP : %s \n", ClientDatabase[i].IP);
        printf("\tPORT : %d \n", ClientDatabase[i].PORT);
        printf("\tSTATUS : %d \n", ClientDatabase[i].status);
    }

    return EXIT_SUCCESS;
}