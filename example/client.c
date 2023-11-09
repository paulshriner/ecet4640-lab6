#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define RCVBUFSIZE 1024   /* Size of receive buffer */
#define SNDBUFSIZE 1024   /* Size of receive buffer */

#define IPS "127.0.0.1"
#define CIPHER_START 32
#define CIPHER_END 126

/**
    See Cipher.h for comments.
*/
void EncryptString(char *string, int length, char *cipher, char start, char end);

/**
    See Cipher.h for comments.
 */
void DecryptString(char* string, int length, char* cipher, char start, char end);

int main(int argc, char *argv[]) {
    int servSock = 0, n = 0;
    char recvBuff[RCVBUFSIZE] = {' '};
    char sendBuff[SNDBUFSIZE] = {' '};
    char cipher[CIPHER_END - CIPHER_START + 1] = {' '};
    char command[20]  = {' '}, username[8]  = {' '};
    struct sockaddr_in serv_addr; 
    int quit = 0, mistake = 0, bytesRcvd = 0;
    char servIP[16] = {' '};
    int port = 0;

    // Copy cipher from sub.key to cipher array
    FILE *file = fopen("sub.key", "r");
    // Skip to beginning of cipher
    fseek(file, 6, SEEK_SET);
    int i = 0;
    int c;
    while ((c = fgetc(file)) != EOF) {
        cipher[i] = (char)c;
        ++i;
    }
    fclose(file);

    printf("Please input the port to connect: ");
    scanf("%d", &port);

    strcpy(servIP, IPS);             /* First arg: DRACO1 IP address (dotted quad) */

    printf("Connecting to: %s.\n", IPS);

    if((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family      = AF_INET;             /* Internet address family */
    serv_addr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    serv_addr.sin_port        = htons(port); /* Server port */


    if(connect(servSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    } 

    if ((bytesRcvd = recv(servSock, recvBuff, RCVBUFSIZE + 1, 0)) <= 0) {
        printf("Error: recv() failed or connection closed prematurely\n");
        close(servSock); //close the socket     
        return 1;
    }
        
    do {
        DecryptString(recvBuff, RCVBUFSIZE, cipher, CIPHER_START, CIPHER_END);
        printf("Server_echo:%s \n",recvBuff);
        if (strcmp(recvBuff, "<Message>Goodbye.<Disconnect>") == 0) {
            close(servSock); //close the socket
            return 1;            
        }
        if (strcmp(recvBuff, "<Error>No such user<Disconnect>") == 0) {
            close(servSock); //close the socket
            return 1;            
        }
        printf("Please type to send to server: ");
        scanf("%s", sendBuff);
        EncryptString(sendBuff, SNDBUFSIZE, cipher, CIPHER_START, CIPHER_END);
        if (send(servSock, sendBuff, sizeof(sendBuff), 0) < 0) {
	        printf("Error: send() failed\n");
        }
        sleep(1);
        memset(&recvBuff, '0', sizeof(recvBuff)); 
        if ((bytesRcvd = recv(servSock, recvBuff, RCVBUFSIZE + 1, 0)) <= 0) {
            printf("Error: recv() failed or connection closed prematurely\n");
            close(servSock); //close the socket
            return 1;
        }
        recvBuff[bytesRcvd]='\0';    
    } while (bytesRcvd>0);
    
	
    close(servSock); //close the socket
    return 0;
}

void EncryptString(char *string, int length, char *cipher, char start, char end) {
    char cipher_l = end - start + 1;
    int i;
    for (i = 0; i < length; i++) {
        if (!(string[i] - start > end || string[i] < start)) {
            string[i] = cipher[string[i] - start];
        }
    }
}

void DecryptString(char* string, int length, char* cipher, char start, char end) {
    char cipher_l = end - start + 1;
    
    int i;
    for(i = 0; i < length; i++) {
        if(!(string[i] - start > end || string[i] < start)) {
            char* c = strchr(cipher, string[i]);
            string[i] = c - cipher + start;
        }
    }
}
