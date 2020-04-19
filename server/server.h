#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h> 
#include <pthread.h>
#include "lines.h"
#include <regex.h>
#include <signal.h>
#include <errno.h>
#include "user_dao.h"
#include "vec.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
// port
#define DEFAULT_PORT 7777
#define MAX_PORT_NUMBER 49151
#define MIN_PORT_NUMBER 1024
// ip address
#define MAX_IP_ADDR_LEN 15
// main socket
#define REQUESTS_QUEUE_SIZE 10
#define ERR_SOCKET_DESCRIPTOR 100
#define ERR_SOCKET_OPTION 110
#define ERR_SOCKET_BIND 120
#define ERR_SOCKET_LISTEN 130
// request
#define MAX_REQ_TYPE_LEN 20
#define REQ_REGISTER "REGISTER"
#define REQ_UNREGISTER "UNREGISTER"
#define REQ_LIST_USERS "LIST_USERS"
#define REQ_LIST_CONTENT "LIST_CONTENT"
// register
#define MAX_USERNAME_LEN 256
#define REGISTER_SUCCESS 0
#define REGISTER_NON_UNIQUE_USERNAME 1
#define REGISTER_OTHER_ERROR 2
// unregister
#define UNREGISTER_SUCCESS 0
#define UNREGISTER_NO_SUCH_USER 1
#define UNREGISTER_OTHER_ERROR 2
// publish
#define MAX_FILENAME_LEN 256
#define MAX_NUMBER_OF_FILES 100000
// list users
#define LIST_USERS_SUCCESS 0
#define LIST_USERS_NO_SUCH_USER 1
#define LIST_USERS_DISCONNECTED 2
#define LIST_USERS_OTHER_ERROR 3
// send users list
#define SEND_USERS_LIST_SUCCESS 0
#define SEND_USERS_LIST_ERR_NUM_OF_USERS 1
#define SEND_USERS_LIST_ERR_USERNAME 2
#define SEND_USERS_LIST_ERR_IP 3
#define SEND_USERS_LIST_ERR_PORT 4
// list content
#define LIST_CONTENT_SUCCESS 0
#define LIST_CONTENT_NOT_REGISTERED 1
#define LIST_CONTENT_DISCONNECTED 2
#define LIST_CONTENT_NO_SUCH_FILES_OWNER 3
#define LIST_CONTENT_OTHER_ERROR 4
// send content list
#define SEND_CONTENT_LIST_SUCCESS 0
#define SEND_CONTENT_LIST_ERR_NUM_OF_FILES 1
#define SEND_CONTENT_LIST_ERR_FILENAME 


///////////////////////////////////////////////////////////////////////////////////////////////////
// structs
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
struct user_data {
	char username[MAX_USERNAME_LEN + 1];
	char ip[MAX_IP_ADDR_LEN + 1];
	char port[6];
};

typedef struct user_data user;
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// function declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Obtains the port number to be used for listening. The port is obtained from the command line
	arguments.
	Returns port number if success, -1 if no port specified or the port is invalid
*/
int obtain_port();
/*
	processes result from the send function and prints approporiate message.
	Returns obtained_port or default port if errors occured.
*/
int process_obtain_port_result(int port);
/*
	starts listening for ctrl+c to finish the program.
	Returns 1 on success and 0 on fail
*/
int start_listening_sigint();
/*
	Prints a cmd template for starting the server
*/
void print_usage();
/*
	Creates a socket to listen on the specified port and assigns the socket descriptor to p_socket.
	Returns:
	ERR_SOCKET_DESCRIPTOR	- could not create the socket
	ERR_SOCKET_OPTION 		- could not set options for the socket
	ERR_SOCKET_BIND 		- could not bind the socket
	ERR_SOCKET_LISTEN 		- could not start listening
*/
int init_socket(int port, int* p_socket);
/*
	initializes mutex_csd and cond_csd.
	Returns 0 on success and -1 on fail.
*/
int init_copy_client_socket_concurrency_mechanisms();
/*
	initializes attributes for request thread.
	Returns 0 on success and -1 on fail
*/
int init_request_thread_attr(pthread_attr_t* attr);
/*
	Deals with the error produced in init_socket.
*/
void process_init_socket_error(int err, int sd);
/*
	Waits in main thread until client socket descriptor is copied to new thread
*/
int wait_till_socket_copying_is_done();
/*
	cleans up after main function.
	Returns 0 on success and -1 on fail.
*/
int clean_up(int server_socket, pthread_attr_t* p_attr);
/*
	Once a request arrives to the server through the general socket (the socket bound to the
	port specifiedif (read_username(socket, username) > 0)	// username specified
	{
		int delete_res = delete_user(username);

		switch (delete_res)
		{
			case DELETE_USER_SUCCESS 		: res = UNREGISTER_SUCCESS; break;
			case DELETE_USER_ERR_NOT_EXISTS : res = UNREGISTER_NO_SUCH_USER; break;
			default							: res = UNREGISTER_OTHER_ERROR; 
		}
	}
	else
	{
		printf("ERROR unregister - no username specified\n");
		res = UNREGISTER_OTHER_ERROR;
	}
	 in cmd) the server will create a new thread for processing the request and
	this is the function which will be runnig in the newly created thread. The function will lock 
	mutex_csd while copying the client socket to a local variable and when it is finish it will
	signal on cond_csd.
*/
void* manage_request(void* p_socket);
/*
	Reads from the socket in order to identify request type, i.e. register, unregister, connect....
	If the request could be identified then a request specific function is called. If the request
	could not be identified an approporiate message will be send back to the socket. 
*/
void identify_and_process_request(int socket);

void register_user(int socket);

/*
	Reads username from the socket and puts it's value into the address space pointed by username.
	Returns number of characters read
*/
int read_username(int socket, char* username);

/*
	checks if username is valid.
	Returns 1 if yes 0 if no
*/
/*int is_username_valid(char* username);*/

void unregister(int socket);
/*
	checks if the user with the username is connected to the server.
	Returns 1 if the user is connected and 0 if no
*/
int is_connected(char* username);

void list_users(int socket);
/*
	dynamically allocates an array of users which are connected to the system, so
	it has to be deleted afterwards.
	Returns number of connected users
*/
uint32_t get_connected_users_list(user** p_users_list);

/*
	Sends list of users through the socket. First username is send, then ip and finally port.
	Returns:
	SEND_USERS_LIST_SUCCESS 			- success
	SEND_USERS_LIST_ERR_NUM_OF_USERS 	- could not send number of users
	SEND_USERS_LIST_ERR_USERNAME 		- could not send username
	SEND_USERS_LIST_ERR_IP 				- could not send ip
	SEND_USERS_LIST_ERR_PORT 			- could not send port
*/
int send_users_list(int socket, user* users_list, uint32_t num_of_users);

void list_content(int socket);
/*
	Sends list of content (names of files) through the socket.
	Returns:
	SEND_CONTENT_LIST_SUCCESS 			- success
	SEND_CONTENT_LIST_ERR_NUM_OF_FILES 	- could not send number of files
	SEND_CONTENT_LIST_ERR_FILENAME 		- could not send filename
*/
int send_content_list(int socket, char** content_list, uint32_t num_of_files);

/*Server recieves a connect request and if all conditions are met then connects the requested
users*/
void connect_users(int socket);

void disconnect_user(int socket);