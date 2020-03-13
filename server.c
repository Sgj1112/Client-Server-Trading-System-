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

int timeStamp=0;
#define TRUE 1
#define FALSE 0

int loggedIn[10];
char* password[6]; // user passwords
int client_socket[30];
int clientId[30];

// PRIORITY QUEUE ----------------------------------------------------------

// Node 
typedef struct node { 
    int price;
    int en;
    int quantity;
    int tid; 
  
    struct node* next; 
} Node; 
  
// Function to Create A New Node 
Node* newNode(int p, int q, int t){ 
    Node* temp = (Node*)malloc(sizeof(Node)); 
    temp->price = p; 
    temp->en = timeStamp;
    timeStamp++;
    temp->quantity = q;
    temp->tid = t;
    temp->next = NULL; 
  
    return temp; 
} 
  
// Return the value at head 
int peek(Node** head){ 
    return (*head)->price; 
} 
  
// Removes the element with the highest priority form the list 
void pop(Node** head){ 
    Node* temp = *head; 
    (*head) = (*head)->next; 
    free(temp); 
} 

// Comparator for nodes of Queue
int comp(Node* h1, Node* h2){
    if(h1->price < h2->price){
        return 1;
    }
    else if(h1->price > h2->price){
        return 0;
    }
    else{
        return (h1->en < h2->en);
    }
}
  
// Function to push according to priority 
void pushMin(Node** head, int p, int q, int t){ 
    Node* start = (*head); 
  
    // Create new Node 
    Node* temp = newNode(p, q, t); 
  
    // Special Case: The head of list has lesser 
    // priority than new node. So insert new 
    // node before head node and change head node. 
    if(comp(temp, *head)){ 
        // Insert New Node before head 
        temp->next = *head; 
        (*head) = temp; 
    } 
    else{ 
  
        // Traverse the list and find a 
        // position to insert new node 
        while (start->next != NULL && comp(start->next, temp)) { 
            start = start->next; 
        } 
  
        // Either at the ends of the list 
        // or at required position 
        temp->next = start->next; 
        start->next = temp; 
    } 
} 

// Function to push according to priority 
void pushMax(Node** head, int p, int q, int t) 
{ 
    Node* start = (*head); 
  
    // Create new Node 
    Node* temp = newNode(p, q, t); 
  
    // Special Case: The head of list has lesser 
    // priority than new node. So insert new 
    // node before head node and change head node. 
    if(!comp(temp, *head)){ 
  
        // Insert New Node before head 
        temp->next = *head; 
        (*head) = temp; 
    } 
    else{ 
  
        // Traverse the list and find a 
        // position to insert new node 
        while (start->next != NULL && !comp(start->next, temp)) { 
            start = start->next; 
        } 
  
        // Either at the ends of the list 
        // or at required position 
        temp->next = start->next; 
        start->next = temp; 
    } 
} 
  
// Function to check is list is empty 
int isEmpty(Node** head){ 
    return (*head) == NULL; 
} 

Node* sellQueue[11], *buyQueue[11];

// ----------------------------------------------------------

// TRADED SET ----------------------------------------------------------

typedef struct linkNode{
        int sid;
        int bid;
        int quantity;
        int sellerprice;
        int buyerprice;
        struct linkNode* next;
} Lnode;


// Creates Lnode
Lnode* createLnode(int s , int b , int q, int ps , int pb){
     Lnode * temp = (Lnode*)malloc(sizeof(Lnode));
     temp->sid = s;
     temp->bid = b;
     temp->quantity = q;
     temp->sellerprice = ps;
     temp->buyerprice = pb;
     temp->next = NULL;
     return temp;
}

Lnode* traded_set;

// ----------------------------------------------------------

/* A utility function to reverse a string  */
void reverse(char* str, int length){ 
    int start = 0; 
    int end = length -1; 
    while (start < end) 
    { 
        char temp=*(str+start);
        *(str+start)= *(str+end);
        *(str+end)=temp; 
        start++; 
        end--; 
    } 
} 

// Implementation of itoa() 
char* itoa(int num, int base){ 
    int i = 0; 
    char *str = malloc(1024*sizeof(char));
    bool isNegative = false; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) 
    { 
        isNegative = true; 
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    // If number is negative, append '-' 
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0'; // Append string terminator 
  
    // Reverse the string 
    reverse(str, i); 
  
    return str; 
}

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

// To load credentials
void load_credentials(){
    FILE *fptr; 
    char *filename = "credentials.txt";  
    
    // Open file 
    fptr = fopen(filename, "r"); 
    if(fptr == NULL){ 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
  
    // Read contents from file 
    char *collect=malloc(1024*sizeof(char));
    while(fgets(collect , 1024 , fptr) && strlen(collect)){
        collect[strlen(collect)-1]='\0';
        printf("%s\n",collect);
        password[collect[0] - '0'] = (collect+2);
        collect=malloc(1024*sizeof(char));
    }
    fclose(fptr); 
}

// Logs out the trader
void logout(int sockIndex){
    int id=clientId[sockIndex];
    clientId[sockIndex]=0;
    loggedIn[id]=0;
    char* reply="SUCCESSFULLY logged out.\n";
    send(client_socket[sockIndex], reply, strlen(reply), 0);
}

// login functions
void login(int sockIndex, char* buffer){
    char *reply;
    while(*buffer!='\0' && *buffer==' '){
        buffer++;
    }
    if(*buffer=='\0' || !isdigit(*buffer)){
        reply="ERROR: Trader ID expected.\n";
    }
    else{
        int id=*buffer-'0';
        if(id<1 || id>5){
            reply="ERROR: Invalid trader id.\n";
        }
        else if(loggedIn[clientId[sockIndex]] && clientId[sockIndex]!= id){
            reply="WARNING: You are logged in with some other id. Please logout first.\n";
        }
        else{
            buffer++;
            if(*buffer!=' '){
                reply="ERROR: Wrong trader id.\n";
            }
            else{
                while(*buffer!='\0' && *buffer==' '){
                    buffer++;
                }
                if(*buffer=='\0'){
                    reply="ERROR: Trader password expected.\n";
                }
                else{
                    if(strcmp(buffer, password[id])==0){
                        if(loggedIn[id]){
                            if(clientId[sockIndex]==id){
                                reply="Already logged in.\n";
                            }
                            else{
                                for(int j=0; j<30; j++){
                                    if(clientId[j]==id){
                                        clientId[j]=0;
                                        break;
                                    }
                                }
                                clientId[sockIndex]=id;
                                reply="SUCCESSFULLY logged in.\n";    
                            }
                        }
                        else{
                            loggedIn[id]=1;
                            clientId[sockIndex]=id;
                            reply="SUCCESSFULLY logged in.\n";
                        }
                    }
                    else{
                        reply="WRONG PASSWORD.\n";
                    }
                }
            }
        }
    }
    send(client_socket[sockIndex], reply, strlen(reply), 0);
}

// Displays Traded Set
void viewTradedSet(int sockIndex){
    int uid = clientId[sockIndex];
    Lnode * temp = traded_set;
    char* reply=malloc(1024*sizeof(char));
    reply[0]='\n';
    reply[1]='\0';
    while (temp->sid != -1){
        if(temp->sid == uid || temp->bid == uid){
            strcat(reply ," SID: ");
            strcat(reply , itoa(temp->sid, 10));
            strcat(reply ," BID:");
            strcat(reply , itoa(temp->bid , 10));
            strcat(reply ," Qty: ");
            strcat(reply , itoa(temp->quantity , 10));
            strcat(reply ," Seller Price: ");
            strcat(reply , itoa(temp->sellerprice , 10));
            strcat(reply ," Buyer Price: ");
            strcat(reply , itoa(temp->buyerprice , 10));
            strcat(reply , "\n");
        }
        temp = temp->next;
    }
    if(strlen(reply)==1){
        reply="Traded Set is Empty.\n";
    }
    send(client_socket[sockIndex] , reply , strlen(reply) , 0);
}

// Inserts into Traded Set
void tradedSetInsertItem(int sid, int bid, int q, int ps, int pb){
    printf("SID = %d , BID = %d , Q = %d , Seller price = %d , Buyer price = %d\n",sid, bid, q , ps ,pb);
    Lnode *newNode = createLnode(sid , bid , q , ps , pb);
    newNode->next = traded_set;
    traded_set = newNode;
}

// Processing BUY request
void workBuy(int sockIndex ,int p ,int q, int in){
    
    while(peek(&sellQueue[in])<=p && q != 0){
        Node* temp=sellQueue[in];                
        int qt=temp->quantity;
        if(qt<=q){
            tradedSetInsertItem(temp->tid, clientId[sockIndex], qt, temp->price, p);
            Node* save=temp;
            sellQueue[in]=temp->next;
            pop(&save);
            q-=qt;
        }
        else{
            temp->quantity-=q;
            tradedSetInsertItem(temp->tid, clientId[sockIndex], q, temp->price, p);
            q=0;
        }
    }
    if(q){
        pushMax(&buyQueue[in], p, q, clientId[sockIndex]);
    }
}

// Processing SELL request
void workSell(int sockIndex ,int p ,int q, int in){
    
    while(peek(&buyQueue[in])>=p && q != 0){
        Node* temp=buyQueue[in];                
        int qt=temp->quantity;
        if(qt<=q){
            tradedSetInsertItem(temp->tid, clientId[sockIndex], qt, temp->price, p);
            Node* save=temp;
            buyQueue[in]=temp->next;
            pop(&save);
            q-=qt;
        }
        else{
            temp->quantity-=q;
            tradedSetInsertItem(temp->tid, clientId[sockIndex], q, temp->price, p);
            q=0;
        }
    }
    if(q){
        pushMin(&sellQueue[in], p, q, clientId[sockIndex]);
    }
}

// Reads number from string
int getnum(char *buffer , int *in){
    int k = 0;
    int l = 0;
    while(*buffer != '\0' && isdigit(*buffer)){
        k *= 10;
        ++l;
        k += (*buffer - '0');
        buffer++;
        /* code */
    }
    *in = k;
    return l;
}

// Validates trader's buy or sell request
void validateRequest(int sockIndex, char* buffer, char type){

    // p = price, q = quantity, in = item number
    int p = 0, q = 0, t = clientId[sockIndex], in = 0;
    while(*buffer!='\0' && *buffer==' '){
        buffer++;
    }
    char* reply;
    int len = getnum(buffer, &in);

    buffer += len;
    if(in >= 1 && in <= 10 && len > 0){
        while(*buffer!='\0' && *buffer==' '){
            buffer++;
        }
        int len = getnum(buffer , &p);      
        buffer += len;
        if(len > 0 && p > 0){
            while(*buffer!='\0' && *buffer==' '){
                buffer++;
            } 
            int len = getnum(buffer , &q);
            buffer += len;
            if(*buffer!='\0'){
                reply="ERROR: Invalid request format. \n";
            }
            else if(len > 0 && q > 0){
                if(type=='B'){
                    workBuy(sockIndex , p , q, in);
                }
                else{
                    workSell(sockIndex, p, q, in);
                }
                reply = "REQUEST ACCEPTED.\n";
            } 
            else{
                reply = "ERROR: Invalid quantity.\n";
            }
        } 
        else{
            reply = "ERROR: Invalid price.\n";
        }
    } 
    else{
        reply = "ERROR: Invalid item number.\n";
    }
    send(client_socket[sockIndex] , reply , strlen(reply) , 0);
}

// Functions to display order status
void viewOrder(int sockIndex){
    char* reply=malloc(1024*sizeof(char));
    reply[0]='\n';
    reply[1]='\0';
    for(int i = 1; i <= 10; ++i){
        int mn = peek(&sellQueue[i]);
        int mx = peek(&buyQueue[i]);

        strcat(reply," Item No.: ");
        strcat(reply , itoa(i , 10));

        strcat(reply ," Best Sell: ");
        if(mn == INT_MAX){
            strcat(reply, "NIL");
            strcat(reply, " Qty: ");
            strcat(reply, "NIL");
        }
        else{
            strcat(reply , itoa(mn , 10));
            strcat(reply, " Qty: ");
            strcat(reply, itoa(sellQueue[i]->quantity, 10));
        }

        strcat(reply ," Best Buy: ");
        if(mx == INT_MIN){
            strcat(reply, "NIL");
            strcat(reply, " Qty: ");
            strcat(reply, "NIL");
        }
        else{
            strcat(reply , itoa(mx , 10));
            strcat(reply, " Qty: ");
            strcat(reply, itoa(buyQueue[i]->quantity, 10));
        }

        strcat(reply , "\n");
    }
    send(client_socket[sockIndex] , reply , strlen(reply) , 0);
}

// Function to process Trader's request
void startTrade(int sockIndex, char* buffer){
    
    char* reply;
    printf("Client Sent: %s\n", buffer);
    char type=buffer[0];
    buffer++;
    if(type=='A'){
        login(sockIndex, buffer);
    }
    else if(type<'A' || type>'F'){
        reply="ERROR: Invalid input.\n";
        send(client_socket[sockIndex], reply, strlen(reply), 0);
    }
    else{
        if(!loggedIn[clientId[sockIndex]]){
            reply="ERROR: Trader not Logged in.\n";
            send(client_socket[sockIndex], reply, strlen(reply), 0);
        }
        else if(type=='B' || type=='C'){
            validateRequest(sockIndex, buffer, type);
        } 
        else{
            while(*buffer!='\0' && *buffer==' '){
                buffer++;
            }
            if(*buffer!='\0'){
                reply="ERROR: Invalid request format.\n";
                send(client_socket[sockIndex], reply, strlen(reply), 0);
            }
            else if(type=='D'){
                viewOrder(sockIndex);
            } 
            else if(type=='E'){
                viewTradedSet(sockIndex);
            }
            else{
                logout(sockIndex);
            }
        }
    }
}

int main(int argc , char *argv[])
{
    // Initially no users logged in
    for(int i=0; i<10; i++){
        loggedIn[i]=0;
    }
    load_credentials();
    // initializing queues

    for(int item = 1; item <= 10; ++item){
        sellQueue[item] = newNode(INT_MAX ,INT_MAX,INT_MAX);
        buyQueue[item] = newNode(INT_MIN ,INT_MIN, INT_MIN);
    }
    traded_set = createLnode(-1,-1,-1,-1,-1);
	int opt = TRUE;
	int master_socket , addrlen , new_socket , 
		max_clients = 30 , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;

	char buffer[1025]; //data buffer of 1K

	//set of socket descriptors
	fd_set readfds;

	//a message
	char *message = "Server: Hello Client";

	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
    if(argc<=1 || argc>2 || !isInt(argv[1])){
        printf("ERROR: Port number not specified properly. \nExpected Format: ./server <PORT> \n");
        exit(0);
    }
	address.sin_port = htons( atoi(argv[1]) );

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port: %s \n", argv[1]);

	//try to specify maximum of 5 pending connections for the master socket
	if (listen(master_socket, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	while(TRUE)
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//add child sockets to set
		for ( i = 0 ; i < max_clients ; i++)
		{
			//socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);

			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

		if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}

		//If something happened on the master socket ,
		//then its an incoming connection
		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs (address.sin_port));

			//send new connection greeting message
			
            /*if( send(new_socket, message, strlen(message), 0) != strlen(message) )
			{
				perror("send");
			}

			puts("Welcome Message Sent to the Client");
            */

			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				//if position is empty
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);

					break;
				}
			}
		}

		//else its some IO operation on some other socket
		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET( sd , &readfds))
			{

				//Check if it was for closing , and also read the
				//incoming message
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n" ,
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;

                    logout(i);
				}

				//Echo back the message that came in
				else
				{
					//set the string terminating NULL byte on the end
					//of the data read
					buffer[valread] = '\0';
                    startTrade(i, buffer);
				}
			}
		}
	}

	return 0;
}
