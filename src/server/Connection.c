/**
 * \addtogroup Connection
 * @{
*/
#include "Connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Util.h"
#include "Data.h"
#include "File.h"
#include "Logfile.h"
#include "Cipher.h"

ClientShared shared;

ClientShared * InitializeShared(map * users_map, size_t send_buffer_size, size_t receive_buffer_size, char* cipher, char start, char end)
{
    shared.users = users_map;
    shared.dirty = 0;
    shared.shutting_down = 0;
    shared.send_buffer_size = send_buffer_size;
    shared.receive_buffer_size = receive_buffer_size;
    shared.cipher = cipher;
    shared.start = start;
    shared.end = end;
    pthread_mutex_init(&(shared.mutex), NULL);
    return &shared;
}

void * StartUpdateThread(void * parameter)
{
    while(shared.shutting_down == 0) {
        if(shared.dirty) {
            pthread_mutex_lock(&(shared.mutex));
            shared.dirty = 0;
            FILE * reg_file = CreateOrOpenFileVerbose(REGISTERED_FILE, NULL);
            if(reg_file != NULL) {
                UpdateRegisteredFileFromUsersMap(reg_file, shared.users);
                fclose(reg_file);
            } else {
                LogfileError("FAILED TO OPEN REGISTERED FILE - NO DATA WILL BE UPDATED");
                shared.dirty = 1;
            }
            pthread_mutex_unlock(&(shared.mutex));
        }
        sleep(1);

    }
    return NULL;
}

void * StartConnectionThread(void * p_connection)
{
    Connection * connection = (Connection *) p_connection;
    connection->state = ClientState_ENTRY;
    connection->user = NULL;
    time(&(connection->time_connected));
    // allocate send and receive buffers.
    char * send_buffer = malloc(shared.send_buffer_size);
    char * receive_buffer = malloc(shared.receive_buffer_size);
    map_result result;

    // ask for their user ID initially, or disconnect them.
    strcpy(send_buffer, "<Message>Welcome. Please send your user ID.");
    MessageOrClose(send_buffer, receive_buffer, connection);
    if(connection->status == ConnectionStatus_ACTIVE) {
        result = Map_Get(shared.users, receive_buffer);
        if(!result.found)
        {
            printYellow("Unauthorized access attempt by %s with name '%s'.\n", inet_ntoa(connection->address.sin_addr), receive_buffer);
            strcpy(send_buffer, "<Error>No such user");
            MessageAndClose(send_buffer, connection);
            LogfileError("Unauthorized access attempt by unknown user %s from %s.", receive_buffer, inet_ntoa(connection->address.sin_addr));
            // send a one-way message to the client
        } else {
            User * user = (User *) result.data;
            if(user->connected) {
                printYellow("User %s attempted to double connect from IP %s.\n", user->id, inet_ntoa(connection->address.sin_addr));
                strcpy(send_buffer, "<Error>You are already connected.");
                LogfileError("User %s attempted to double connect from IP %s.\n", user->id, inet_ntoa(connection->address.sin_addr));
                MessageAndClose(send_buffer, connection);
                // send the other connected user an informative message?
            } else {
                connection->user = user;
                connection->user->connected = 1;
                strcpy(connection->user->ip, inet_ntoa(connection->address.sin_addr));
                if(connection->user->registered) {
                    connection->state = ClientState_UNAUTHENTICATED;
                    LogfileMessage("User %s is attempting a login from ip %s.", connection->user->name, inet_ntoa(connection->address.sin_addr));
                } else {
                    connection->state = ClientState_ACCESSING;
                }                
            }
        }
    } 

    if(connection->state == ClientState_ACCESSING && connection->status == ConnectionStatus_ACTIVE) {
        strcpy(send_buffer, "<Message>Say something, unregistered user!");
    } else if (connection->state == ClientState_UNAUTHENTICATED && connection->status == ConnectionStatus_ACTIVE) {
        strcpy(send_buffer, "<Message>Say something, registered user (logged out)!");
    } else if (connection->state == ClientState_REGISTERED && connection->status == ConnectionStatus_ACTIVE) {
        strcpy(send_buffer, "<Message>Say something, registered user (logged in)!");
    }

    while(connection->status == ConnectionStatus_ACTIVE)
    {
        if(connection->state == ClientState_ACCESSING) {
            MessageOrClose(send_buffer, receive_buffer, connection);
            if (strcmp(receive_buffer, "help") == 0) {
                _help(connection, send_buffer);
            } else if (strcmp(receive_buffer, "exit") == 0) {
                strcpy(send_buffer, "<Message>Goodbye.");
                MessageAndClose(send_buffer, connection);
            } else if (strcmp(receive_buffer, "register") == 0) {
                if (_password(connection) == 0) {
                    _register(connection, send_buffer);
                    LogfileMessage("User %s registered.", connection->user->id);
                } else {
                    strcpy(send_buffer, "<Error>Invalid password entered, cannot register");
                    LogfileError("User %s attempted to register with an invalid password.", connection->user->id);
                }
            } else {
                strcpy (send_buffer, "<Error>Invalid command, use 'help' for list of commands");
            }
        } else if (connection -> state == ClientState_UNAUTHENTICATED) {
            MessageOrClose(send_buffer, receive_buffer, connection);
            if (strcmp(receive_buffer, "help") == 0) {
                _help(connection, send_buffer);
            } else if (strcmp(receive_buffer, "exit") == 0) {
                strcpy(send_buffer, "<Message>Goodbye.");
                MessageAndClose(send_buffer, connection);
            } else if (strcmp(receive_buffer, "login") == 0) {
                if (_authenticate(connection) == 0) {
                    strcpy(send_buffer, "<Message>You have logged in!");
                    connection->user->lastConnection = time(NULL);
                    printBlue("Setting connection time_connected to: %d\n", connection->time_connected);
                    shared.dirty = 1;
                    LogfileMessage("User %s logged in.", connection->user->id);
                } else {
                    strcpy(send_buffer, "<Message>Login failed!");
                    LogfileError("Failed login attempt for user %s.", connection->user->id);
                }
            } else {
                strcpy (send_buffer, "<Error>Invalid command, use 'help' for list of commands");
            }
        } else if(connection->state == ClientState_REGISTERED) {
            MessageOrClose(send_buffer, receive_buffer, connection);
            if (strcmp(receive_buffer, "help") == 0) {
                _help(connection, send_buffer);
            } else if (strcmp(receive_buffer, "exit") == 0) {
                strcpy(send_buffer, "<Message>Goodbye.");
                MessageAndClose(send_buffer, connection);                
            } else if (strcmp(receive_buffer, "myinfo") == 0) {
                _myinfo(connection, send_buffer);
            } else if (strcmp(receive_buffer, "who") == 0) {
                _who(send_buffer);
            } else if (strcmp(receive_buffer, "random-gpa") == 0) {
                _rand_gpa(connection, send_buffer);
            } else if (strcmp(receive_buffer, "random-age") == 0) {
                _rand_age(connection, send_buffer);
            } else if (strcmp(receive_buffer, "advertisement") == 0) {
                _advertisement(connection, send_buffer);
            } else if (strcmp(receive_buffer, "change-password") == 0) {
                if (_password(connection) == 0) {
                    strcpy(send_buffer, "<Message>Password has been changed");
                } else {
                    strcpy(send_buffer, "<Error>Invalid password entered, no action taken");
                }
            } else {
                strcpy(send_buffer, "<Error>Invalid command, use 'help' for list of commands");
            }
            // call a function for processing this state.
        } else {
            printRed("Client entered invalid state. Disconnecting. \n");
            strcpy(send_buffer, "<Error>You entered an invalid state!");
            MessageAndClose(send_buffer, connection);
            connection->status = ConnectionStatus_CLOSING;
        }
    }

    if(connection->user != NULL) {
        connection->user->connected = 0;
        printf("User %s from ip %s disconnected.\n", connection->user->id, connection->user->ip);
        LogfileMessage("User %s from ip %s disconnected.", connection->user->id, connection->user->ip);
    } else {
        printf("Ip %s disconnected.\n", inet_ntoa(connection->address.sin_addr));
        LogfileMessage("Ip %s disconnected.\n", inet_ntoa(connection->address.sin_addr));
    }
    

    free(send_buffer);
    free(receive_buffer);
    close(connection->socket);
    if(connection->user != NULL) {
        connection->user->connected = 0;
    }
    connection->status = ConnectionStatus_CLOSED;
    return NULL;
}


int MessageOrClose(char * send_buffer, char * receive_buffer, Connection * connection) {
    receive_buffer[0] = '\0';
    EncryptString(send_buffer, strlen(send_buffer), shared.cipher, shared.start, shared.end);
    memset(receive_buffer, 0, shared.receive_buffer_size);
    if(send(connection->socket, send_buffer, shared.send_buffer_size, 0) < 0) {
        printRed("Failed to send message to %s. Disconnecting.\n", inet_ntoa(connection->address.sin_addr));
        perror("Error:");
        connection->status = ConnectionStatus_CLOSING;
        return 0;
    }
    int received_size = recv(connection->socket, receive_buffer, shared.receive_buffer_size, 0);
    if(received_size < 0) {
        printRed("Failed to receive message from %s. Disconnecting.\n", inet_ntoa(connection->address.sin_addr));
        perror("Error: ");
        connection->status = ConnectionStatus_CLOSING;
        return 0;
    }
    if(received_size == 0 ) {
        printBlue("%s disconnected.\n", inet_ntoa(connection->address.sin_addr));
        connection->status = ConnectionStatus_CLOSING;
        return 0;
    }
    send_buffer[0] = '\0';
    // memset(send_buffer, 0, shared.send_buffer_size);

    DecryptString(receive_buffer, strlen(receive_buffer), shared.cipher, shared.start, shared.end);
    return received_size;
}



void MessageAndClose(char * send_buffer, Connection * connection) {
    strcat(send_buffer, "<Disconnect>");
    EncryptString(send_buffer, strlen(send_buffer), shared.cipher, shared.start, shared.end);
    send(connection->socket, send_buffer, shared.send_buffer_size, 0);
    connection->status = ConnectionStatus_CLOSING;
    if (connection -> user != NULL) {
        connection->user->connected = 0;
    } 
}

void _help(Connection* connection, char* response) {
    if (connection -> state == ClientState_UNAUTHENTICATED) {
        strcpy(response, "<Message>help - get a list of available commands\n");
        strcat(response, "login - login to the server\n");
        strcat(response, "exit - disconnect from the server");
        LogfileMessage("%s asked for help.", inet_ntoa(connection->address.sin_addr));
    } else if(connection->state != ClientState_REGISTERED) {
        strcpy(response, "<Message>help - get a list of available commands\n");
        strcat(response, "register - register your user\n");
        strcat(response, "exit - disconnect from the server");
        LogfileMessage("%s asked for help.", inet_ntoa(connection->address.sin_addr));
    } else if(connection->state == ClientState_REGISTERED) {
        strcpy(response, "<Message>help - get a list of available commands\n");
        strcat(response, "exit - disconnect from the server\n");
        strcat(response, "who - get a list of online users\n");
        strcat(response, "random-gpa - set your gpa to a new random value\n");
        strcat(response, "random-age - set your age to a new random value\n");
        strcat(response, "advertisement - get a colorful advertisement\n");
        strcat(response, "myinfo - get info about yourself\n");
        strcat(response, "change-password - change your current password\n");
        LogfileMessage("%s asked for help.", connection->user->name);
    }
}

int _register(Connection * connection, char* response) {
    if(connection->user->registered) {
        strcpy(response, "<Error>");
        strcat(response, connection->user->id);
        strcat(response, " is already registered.");

        printRed("%s from ip %s has attempted to register a second time.\n", connection->user->id, inet_ntoa(connection->address.sin_addr));
        LogfileError("%s from ip %s has attempted to register a second time.\n", connection->user->id, inet_ntoa(connection->address.sin_addr));
        return 0;
    }

    pthread_mutex_lock(&(shared.mutex));
    
    connection->user->registered = 1;

    connection->user->age = RandomInteger(18, 22);

    if(RandomFlag(.4)) {
        connection->user->gpa = 4.0;
    } else {
        connection->user->gpa = RandomFloat(2.5, 4);
    }

    connection->state = ClientState_REGISTERED;

    LogfileMessage("%s registered from ip %s.", connection->user->id, inet_ntoa(connection->address.sin_addr));
    printBlue("%s registered.\n", connection->user->id);

    shared.dirty = 1;
    pthread_mutex_unlock(&(shared.mutex));

    strcpy(response, "<Message>You have been registered ");
    strcat(response, connection->user->name);

    return 1;
}

int _myinfo(Connection* connection, char* response) {
    
    if (!(connection->user->registered)) {
        strcpy(response, "<Error>");
        strcat(response, connection->user->id);
        strcat(response, " is not registered.");

        LogfileError("%s from ip %s has attempted to view their information as an unregistered user.\n", connection->user->id, inet_ntoa(connection->address.sin_addr));

        return 1;
    }

    //Referenced snprintf from https://cplusplus.com/reference/cstdio/snprintf/
    snprintf(response, shared.send_buffer_size, "<User.Name>%s<User.Age>%d<User.GPA>%.2f<User.IP>%s", connection->user->name, connection->user->age, connection->user->gpa, inet_ntoa(connection->address.sin_addr));

    printf("%s viewed their information.\n", connection->user->id);
    LogfileMessage("%s viewed their information.", connection->user->name);

    return 0;
}

void _who(char * response) {
    int i;
    for(i = 0; i < RECORD_COUNT; i++) {
        map_result result = Map_Get(shared.users, accepted_userIDs[i]);
        if(result.found) {
            User* user = (User *) result.data;

            if(user->connected) {
                strcat(response, "<OnlineUser>");
                strcat(response, user->id);
            }
        }
    }
}

void _rand_gpa(Connection* connection, char* response) {
    char gpa_str[5];
    pthread_mutex_lock(&(shared.mutex));
    if(RandomFlag(.4)) {
        connection->user->gpa = 4.0;
    } else {
        connection->user->gpa = RandomFloat(2.2, 4.0);
    }
    shared.dirty = 1;
    pthread_mutex_unlock(&(shared.mutex));
    sprintf(gpa_str, "%.2f", connection->user->gpa);
    strcat(response, "<User.GPA>");
    strcat(response, gpa_str);
    LogfileMessage("%s randomized their gpa.", connection->user->name);
}

void _rand_age(Connection* connection, char * response) {
    char age_str[5];
    pthread_mutex_lock(&(shared.mutex));
    connection->user->age = RandomInteger(18, 22);
    shared.dirty = 1;
    pthread_mutex_unlock(&(shared.mutex));
    sprintf(age_str, "%d", connection->user->age);
    strcat(response, "<User.Age>");
    strcat(response, age_str);
    LogfileMessage("%s randomized their age.", connection->user->name);
}

void _advertisement(Connection * connection, char * response) {
    char filename[FILENAME_MAX];

    GetRandomFileNameFromDir(ADS_DIR, filename);

    char* filepath = malloc(FILENAME_MAX + sizeof(ADS_DIR));
    strcpy(filepath, ADS_DIR);
    strcat(filepath, "/");
    strcat(filepath, filename);

    strcat(response, "<Message>");

    LogfileMessage("User %s viewed advertisement %s.", connection->user->name, filepath);
    CatFileToBuffer(filepath, response, shared.send_buffer_size);

    free(filepath); //always free malloced strings to prevent mem leaks!
}

int _password (Connection* connection) {
    // Create send/rcv buffers, password buffer
    char * send_buffer = malloc(shared.send_buffer_size);
    char * receive_buffer1 = malloc(shared.receive_buffer_size);
    char * receive_buffer2 = malloc(shared.receive_buffer_size);
    char * password = malloc(PASSWORD_LENGTH);

    // Prompt user for password two times
    strcpy(send_buffer, "<Message>Enter a password");
    MessageOrClose(send_buffer, receive_buffer1, connection);
    strcpy(send_buffer, "<Message>Enter the same password");
    MessageOrClose(send_buffer, receive_buffer2, connection);

    // Check if passwords match and is within PASSWORD_LENGTH
    if ((strcmp(receive_buffer1, receive_buffer2) != 0) || (strlen(receive_buffer1) > PASSWORD_LENGTH)) {
        free(send_buffer);
        free(receive_buffer1);
        free(receive_buffer2);
        free(password);
        return 1;
    }

    // If so, fill password with buffer up to PASSWORD_LENGTH
    int i = 0;
    for (i = 0; i < PASSWORD_LENGTH; ++i) {
        password[i] = receive_buffer1[i];
    }

    // Set user's password
    pthread_mutex_lock(&(shared.mutex));
    strcpy(connection -> user -> password, password);
    shared.dirty = 1;
    pthread_mutex_unlock(&(shared.mutex));

    free(send_buffer);
    free(receive_buffer1);
    free(receive_buffer2);
    free(password);
    return 0;
}

int _authenticate (Connection* connection) {
    // Create send/rcv buffers
    char * send_buffer = malloc(shared.send_buffer_size);
    char * receive_buffer = malloc(shared.receive_buffer_size);

    // Prompt for password
    strcpy(send_buffer, "<Message>Enter your password");
    MessageOrClose(send_buffer, receive_buffer, connection);

    // Check if password matches current user's password
    if (strcmp(receive_buffer, connection -> user -> password) != 0) {
        free(send_buffer);
        free(receive_buffer);
        return 1;
    }

    // If so, user is now authenticated
    pthread_mutex_lock(&(shared.mutex));
    connection->state = ClientState_REGISTERED;
    shared.dirty = 1;
    pthread_mutex_unlock(&(shared.mutex));

    free(send_buffer);
    free(receive_buffer);
    return 0;
}

/**
 * @}
*/
