/**
 * \addtogroup Server
 * @{
*/
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/net.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "Server.h"
#include "Connection.h"
#include "Util.h"
#include "map.h"
#include "Logfile.h"
#include "File.h"

ServerProperties server;
Connection * connections;

// A private function just for reading the settings map into the server struct and printing warnings as necessary.
void _readSettingsMapIntoServerStruct(map * server_settings) {
    map_result result = Map_Get(server_settings, "port");
    if(!result.found) {
        printYellow("No port setting found. Defaulting to 3000.\n");
        server.port = 3000;
    } else {
        int found_port = atoi(result.data);
        if(found_port <= 0) {
            printYellow("Invalid port setting: %s. Defaulting to 3000.\n", result.data);
            server.port = htons(3000);
        } else {
            server.port = htons(found_port);
        }
    }
    result = Map_Get(server_settings, "send_buffer_size");
    if(!result.found) {
        printYellow("No send_buffer_size setting found. Defaulting to 1024.\n");
        server.send_buffer_size = 1024;
    } else {
        int found_sb_size = atoi(result.data);
        if(found_sb_size <= 0) {
            printYellow("Invalid send_buffer_size setting: %s. Defaulting to 1024.\n", result.data);
            server.send_buffer_size = 1024 * sizeof(char);
        } else {
            server.send_buffer_size = found_sb_size * sizeof(char);
        }
    }
    result = Map_Get(server_settings, "receive_buffer_size");
    if(!result.found) {
        printYellow("No receive_buffer_size setting found. Defaulting to 1024.\n");
        server.send_buffer_size = 1024;
    } else {
        int found_rb_size = atoi(result.data);
        if(found_rb_size <= 0) {
            printYellow("Invalid receive_buffer_size setting: %s. Defaulting to 1024.\n", result.data);
            server.receive_buffer_size = 1024 * sizeof(char);
        } else {
            server.receive_buffer_size = found_rb_size * sizeof(char);
        }
    }
    result = Map_Get(server_settings, "backlog");
    if(!result.found) {
        printYellow("No backlog setting found. Defaulting to 10.\n");
        server.backlog = 10;
    } else {
        int found_backlog = atoi(result.data);
        if(found_backlog <= 0) {
            printYellow("Invalid backlog setting: %s. Defaulting to 10.\n", result.data);
            server.backlog = 10;
        } else {
            server.backlog = found_backlog;
        }
    }
    result = Map_Get(server_settings, "max_connections");
    if(!result.found) {
        printYellow("No max_connections setting found. Defaulting to 20.\n");
        server.max_connections = 20;
    } else {
        int found_max_connections = atoi(result.data);
        if(found_max_connections <= 0) {
            printYellow("Invalid max_connections setting: %s. Defaulting to 20.\n", result.data);
            server.max_connections = 20;
        } else {
            server.max_connections = found_max_connections;
        }
    }
    result = Map_Get(server_settings, "start_char");
    if(!result.found) {
        printYellow("No start_char setting found. Defaulting to ' '\n");
        server.start = ' ';
    } else {
        char* start = result.data;
        server.start = start[0];
    }
    result = Map_Get(server_settings, "end_char");
    if(!result.found) {
        printYellow("No end_char setting found. defaulting to '~'\n");
        server.end = '~';
    } else {
        char* end = result.data;
        server.end = end[0];
    }
    result = Map_Get(server_settings, "cipher");
    if(!result.found) {
        printRed("No cipher found.\n");
    } else {
        server.cipher = result.data;
    }
    result = Map_Get(server_settings, "log_file");
    if(!result.found) {
        printYellow("No log_file specified, defaulting to 'log.txt'.\n");
        SetLogfileName("log.txt");
    } else {
        SetLogfileName((char *) result.data);
        printBlue("Logging to %s.\n", result.data);
    }
};

int InitializeServer(map * server_settings) {
    _readSettingsMapIntoServerStruct(server_settings);
    connections = malloc(server.max_connections * sizeof(Connection));
    int i;
    for(i=0; i < server.max_connections; i++) {
        connections[i].status = ConnectionStatus_CLOSED;
    }
    printGreen("Server initialized with %d max connections.\n", server.max_connections);
    return 1;
}

int StartServer(map * users_map) {
    int serverSocket = 0;
    struct sockaddr_in server_address;
    // Record the time the server started.
    time(&server.time_started);
    // Get a socket file pointer associated with ipv4 internet protocols that represents a two-way connection based byte stream.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    // Set the address to bind to all available interfaces.
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // Set the port.
    server_address.sin_port = server.port;
    // Assign a name to the socket.
    int bind_error = bind(serverSocket, (struct sockaddr*)&server_address, sizeof(server_address));
    if(bind_error) {
        printRed("Error binding the server to port %d.\n", ntohs(server.port));
        perror("Bind Error:");
        return 0;
    }
    server.socket_id = serverSocket;
    int lockfile_success = CreateLockfile();
    if(!lockfile_success) {
        printRed("Failed to create Lockfile! Server cannot start.");
        return 0;
    }
    // Initialized a shared space that will be used across threads.
    ClientShared * shared = InitializeShared(users_map, server.send_buffer_size, server.receive_buffer_size, server.cipher, server.start, server.end);
    // The update thread is responsible for checking if there is 'dirty' data that should be saved to the registered user's file.
    pthread_t registered_update_thread;
    pthread_create(&registered_update_thread, NULL, StartUpdateThread, NULL);
    printBlue("Server listening on port: %d\n", ntohs(server.port));
    LogfileMessage("Server started.");
    // begin listening according to the socket settings
    listen(serverSocket, server.backlog);
    while(!shared->shutting_down) {
        // Get an available connection.
        Connection * next_client = NextAvailableConnection();
        if(next_client == NULL) {
            printYellow("Server connections are maxxed.\n");
            LogfileError("Server couldn't accept connection; available connections are maxxed.");
            sleep(1);
            continue;
        }
        // Accept a connection.
        next_client->address_length = sizeof(next_client->address);
        next_client->socket = accept(serverSocket, (struct sockaddr *)&(next_client->address), &(next_client->address_length));
        if(next_client->socket < 0)
        {
            printRed("Failed to accept() client!\n");
            LogfileError("Failed to accept() client.");
            sleep(1);
            continue;
        }
        printBlue("New client connection from IP: %s\n", inet_ntoa(next_client->address.sin_addr));
        LogfileMessage("New client connection from IP: %s", inet_ntoa(next_client->address.sin_addr));
        next_client->status = ConnectionStatus_ACTIVE;
        // Start a thread to handle communication from that connection.
        pthread_create(&(next_client->thread_id), NULL, StartConnectionThread, next_client);
    }

    return 1;
}

Connection * NextAvailableConnection()
{
    int i;
    for(i = 0; i < server.max_connections; i++) {
        if(connections[i].status == ConnectionStatus_CLOSED)
        {
            return &(connections[i]);
        }
    }
    return NULL;
}

int CloseServer() {
    return close(server.socket_id);
}


/**
 * @}
*/
