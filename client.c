#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

#include <netdb.h> 
#include <netinet/in.h>  
#include <sys/socket.h> 
#include <sys/types.h> 
#include <errno.h>  
#include <unistd.h>     
#include <arpa/inet.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

// Checks if the string contains numbers only
int isInt(char *str){
    int ok=1;
    for(int i=0; str[i]!='\0'; i++){
        if(!isdigit(str[i])){
            ok=0;
        }
    }
    return ok;
}

void displayMenu(){
    printf("Enter the input in the following format: \n");
    printf("Function : format \n");
    printf("Login:  A <userid> <password> \n");
    printf("Buy Request: B <item_id> <price> <quantity>\n");    
    printf("Sell Request: C <item_id> <price> <quantity>\n");    
    printf("View Order Status: D \n");    
    printf("View Traded Set: E \n");    
    printf("Logout: F \n");    
    printf("Exit: G \n");
    printf("Display Menu: H \n");
}

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char toSend[1024];
    displayMenu();
    char buffer[1024] = {0};
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }

    if(argc<=1){
        printf("ERROR: IP not specified properly. \nExpected Format: ./client <IP> <PORT> \n");
        exit(0);
    }
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    if(argc<=2 || argc>3 || !isInt(argv[2])){
        printf("ERROR: Port number not specified properly. \nExpected Format: ./client <IP> <PORT> \n");
        exit(0);
    }
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\nConnection Failed \n");
        return -1;
    }

    while(1){
        printf("$ ");

        memset(buffer, 0, sizeof(buffer));
    	for(int i = 0; i < 1024; i++){
            char c;
            c = getchar();
            if(c == '\n'){
                toSend[i] = '\0';
                break;
            }
            toSend[i] = c;
        }

        if(toSend[0]>'H' || toSend[0]<'A' || strlen(toSend)==0){
            printf("INVALID input.\n");
            continue;
        }

        if(toSend[0]=='H'){
            if(strlen(toSend)!=1){
                printf("INVALID input.\n");
                continue;
            }
            displayMenu();
            continue;
        }

        if(toSend[0]=='G'){
            if(strlen(toSend)!=1){
                printf("INVALID input.\n");
                continue;
            }
            send(sock , "F" , strlen("F") , 0 );
            valread = read( sock , buffer, 1024);
            return 0;
        }
        
    	send(sock , toSend , strlen(toSend) , 0 );
    	printf("Your Message has been sent successfully to the Server\n");

        // Read What Socket Has Received From the Server
    	valread = read( sock , buffer, 1024);
    	printf("Server had Echoed: %s\n", buffer);
    }

    return 0;
}