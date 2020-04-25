#include "storage.h"
#include "server.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h> 
#include <pthread.h>
#include "lines.h"
#include <regex.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>



///////////////////////////////////////////////////////////////////////////////////////////////////
// global variables
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
	mutex for preventing race condition during copying client socket description to a new thread.
*/
pthread_mutex_t mutex_csd;
/*
	condition for waiting until the cliend socket descriptor is copied to a new thread
*/
pthread_cond_t cond_csd;
/*
	used to check whether main thread should still wait for request thread (client socket not
	yet copied)
*/
int is_copied;
/*
	flat to mark if the programm should continue. if 1 then continue, if 0 then the main loop
	should stop. It is set to 1 after pressing ctrl + c
*/
int is_running = 1;

CLIENT* p_client;


void storage_1(char *host)
{
	CLIENT *clnt;
	// enum clnt_stat retval_1;
	// int result_1;
	// enum clnt_stat retval_2;
	// int result_2;
	// enum clnt_stat retval_3;
	// int result_3;
	// char *add_user_1_username;
	// enum clnt_stat retval_4;
	// int result_4;
	// char *delete_user_1_username;
	// enum clnt_stat retval_5;
	// int result_5;
	// // char *add_connected_user_1_username;
	// // char *add_connected_user_1_in_addr;
	// // char *add_connected_user_1_port;
	// enum clnt_stat retval_6;
	// int result_6;
	// char *delete_connected_user_1_username;
	// enum clnt_stat retval_7;
	// users_vector result_7;
	// enum clnt_stat retval_8;
	// int result_8;
	// char *add_file_1_username;
	// char *add_file_1_file_name;
	// char *add_file_1_description;
	// enum clnt_stat retval_9;
	// int result_9;
	// char *delete_file_1_username;
	// char *delete_file_1_file_name;
	// enum clnt_stat retval_10;
	// //files_vector result_10;
	// char *get_files_1_username;

#ifndef	DEBUG
	clnt = clnt_create (host, STORAGE, STORAGE_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}

	p_client = clnt;
#endif	/* DEBUG */

	// retval_1 = setup_1(&result_1, clnt);
	// if (retval_1 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_2 = shutdown_1(&result_2, clnt);
	// if (retval_2 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_3 = add_user_1(add_user_1_username, &result_3, clnt);
	// if (retval_3 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_4 = delete_user_1(delete_user_1_username, &result_4, clnt);
	// if (retval_4 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_5 = add_connected_user_1(add_connected_user_1_username, add_connected_user_1_in_addr, add_connected_user_1_port, &result_5, clnt);
	// if (retval_5 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_6 = delete_connected_user_1(delete_connected_user_1_username, &result_6, clnt);
	// if (retval_6 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_7 = get_connected_users_1(&result_7, clnt);
	// if (retval_7 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_8 = add_file_1(add_file_1_username, add_file_1_file_name, add_file_1_description, &result_8, clnt);
	// if (retval_8 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_9 = delete_file_1(delete_file_1_username, delete_file_1_file_name, &result_9, clnt);
	// if (retval_9 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
	// retval_10 = get_files_1(get_files_1_username, &result_10, clnt);
	// if (retval_10 != RPC_SUCCESS) {
	// 	clnt_perror (clnt, "call failed");
	// }
#ifndef	DEBUG
	// clnt_destroy (clnt);
#endif	 /* DEBUG */
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) 
{
	// get port and ip of storage server from arguments
	int port;
	char storage_host[MAX_HOST_LEN];
	get_arguments(argc, argv, &port, storage_host);	// if fails -> exit -1
	// temporal
	storage_1(storage_host);
	init_storage(); // if fails -> exit -1

	// obtain and print the local ip
	char* addr = get_server_ip();
	if (addr == NULL)
	{
		printf("ERROR main - could not obtain server's ip\n");
		return -1;
	}

	printf("s> init server %s:%d\n", addr, port);
	printf("s>\n");

	// initialize the main socket
	int server_socket = -1;
	int init_socket_res = init_socket(port, &server_socket);
	if (init_socket_res != 0)
	{
		process_init_socket_error(init_socket_res, server_socket);
		return -1;
	}

	// initialize mutex and condition variable for copying arguments for thread requests
	if (init_copy_client_socket_concurrency_mechanisms() != 0)
		return -1;

	// init request thread attributes
	pthread_attr_t attr_req_thread;
	if (init_request_thread_attr(&attr_req_thread) != 0)
		return -1;

	pthread_t t_request;
		
	// start detecting ctrl + c
	if (!start_listening_sigint())
		return -1;
	
	// start waiting for requests
	struct sockaddr_in client_addr;
	int client_socket;
    socklen_t clinet_addr_size = sizeof(struct sockaddr_in);
	
    while (is_running)
    {
        // accept connection from a client
        client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &clinet_addr_size);

		if (client_socket >= 0)
		{
			if (pthread_create(&t_request, &attr_req_thread, manage_request, (void*) &client_socket) != 0)
				perror("ERROR main - could not create request thread");

			if (wait_till_socket_copying_is_done() != 0)
				return -1;
		}
		else if (errno != EINTR) // if EINTR then ctrl+c was pressed, finish
		{
			perror("ERROR main - could not accept request from socket");
			return -1;
		}
    }
	
	return clean_up(server_socket, &attr_req_thread);
}



void get_arguments(int argc, char* argv[], int* p_port, char* storage_server_ip)
{
	int res = 1;
	int option = -1;

	char port[MAX_PORT_STR_LEN + 1];
	int port_set = 0;
	port[MAX_PORT_STR_LEN] = '\0';
	char ip[MAX_HOST_LEN + 1];
	int ip_set = 0;
	ip[MAX_HOST_LEN] = '\0';

	while ((option = getopt(argc, argv, "p:s:")) != -1)
	{
		switch (option)
		{
			case 'p' : 
				strncpy(port, optarg, MAX_PORT_STR_LEN);
				port_set = 1;
				break;
			case 's' :
				strncpy(ip, optarg, MAX_HOST_LEN);
				ip_set = 1;
				break;
			default :
				res = 0;
		}
	}

	if (port_set == 0 || sscanf(port, "%d", p_port) != 1 ||
		*p_port <= MIN_PORT_NUMBER || *p_port > MAX_PORT_NUMBER)
	{
		res = 0;
	}

	if (ip_set == 0 || strlen(ip) > MAX_HOST_LEN)
	{
		res = 0;
	}

	if (res == 1)
	{
		strcpy(storage_server_ip, ip);
	}
	else
	{
		print_usage();
		exit(-1);
	}
}



void print_usage() 
{
	printf("Usage: server -p <port [1024 - 49151]> -s <storage server IPv4>\n");
}



void init_storage()
{
	// init storage
	int setup_res = -1;
	if (setup_1(&setup_res, p_client) != RPC_SUCCESS)
	{
		clnt_perror (p_client, "ERROR main - could not setup the storage server");
		exit(-1);
	}

	if (setup_res != 0)
	{
		printf("ERROR main - could not setup the storage server. code: %d\n", setup_res);
		exit(-1);
	}
}



char* get_server_ip()
{
	struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr = NULL;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && 										// address specified
			ifa->ifa_addr->sa_family==AF_INET &&					// proper address family
			strcmp(ifa->ifa_name, LOOPBACK_INTERFACE_NAME) != 0)	// no loopback
		{
            sa = (struct sockaddr_in *) ifa->ifa_addr;			// addr on non-loopback interface
            addr = inet_ntoa(sa->sin_addr);						// addr parsed to string
			break;												// one addr is enough
        }
    }

    freeifaddrs(ifap);

	return addr;
}



int init_socket(int port, int* p_socket)
{
    struct sockaddr_in server_addr;
    int sd; // server socket descriptor, client socket descriptor
    
    // obtain server socket descriptor
    sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd == -1)
		return ERR_SOCKET_DESCRIPTOR;

	*p_socket = sd;

    // set server socket options
    int reuse_addr_val = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse_addr_val, sizeof(int)) == -1)
		return ERR_SOCKET_OPTION;

    // clear server addres
    bzero((char*) &server_addr, sizeof(server_addr));

    // initialize the server addres
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // bind the socket to the address
    if (bind(sd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
		return ERR_SOCKET_BIND;

    // start listening on the socket
    if (listen(sd, REQUESTS_QUEUE_SIZE) == -1)
		return ERR_SOCKET_LISTEN;

	return 0;
}



void process_init_socket_error(int err, int sd)
{
	if (err == ERR_SOCKET_DESCRIPTOR)
		perror("ERROR init_socket - could not obtain socket descriptor");
	else
	{
		switch (err)
		{
			case ERR_SOCKET_OPTION : 
				perror("ERROR init_socket - could not set options"); break;
			case ERR_SOCKET_BIND : 
				perror("ERROR init_socket - could not bind"); break;
			case ERR_SOCKET_LISTEN :
				perror("ERROR init_socket - could not start listening"); break;

			if (close(sd) == -1)
				perror ("ERROR process_init_socket_error - could not close the socket");
		}
	}
}



int init_copy_client_socket_concurrency_mechanisms()
{
	if (pthread_mutex_init(&mutex_csd, NULL) != 0)
	{
		perror("ERROR main - could not init mutex_csd");
		return -1;
	}

	if (pthread_cond_init(&cond_csd, NULL) != 0)
	{
		perror("ERROR main - could not init cond_csd");
		return -1;
	}

	return 0;
}



int init_request_thread_attr(pthread_attr_t* attr)
{
	if (pthread_attr_init(attr) != 0)
	{
		printf("ERROR init_request_thread_attr - could not init\n");
		return -1;
	}

    if (pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED) != 0)
	{
		printf("ERROR init_request_thread_attr - could not set detach state\n");
		return -1;
	}

	return 0;
}



void set_exit_flag(int val)
{
	is_running = 0;
}



int start_listening_sigint()
{
	struct sigaction act;
	memset(&act, '\0', sizeof(act));

	act.sa_handler = &set_exit_flag;

	if (sigaction(SIGINT, &act, NULL) != 0)
	{
		perror("ERROR start_listening_siginit - could not perform sigaction");
		return 0;
	}

	return 1;
}



int wait_till_socket_copying_is_done()
{
	if (pthread_mutex_lock(&mutex_csd) != 0)
	{
		printf("ERROR wait_till_socket_copying_is_done - could not lock mutex_csd\n");
		return -1;
	}

	while (is_copied == 0)
	{
		if (pthread_cond_wait(&cond_csd, &mutex_csd) != 0)
		{
			printf("ERROR wait_till_socket_copying_is_done - during condition wait\n");
			return -1;
		}
	}

	is_copied = 0;

	if (pthread_mutex_unlock(&mutex_csd) != 0)
	{
		printf("ERROR wait_till_socket_copying_is_done - could not unlock mutex_csd\n");
		return -1;
	}

	return 0;
}



int clean_up(int server_socket, pthread_attr_t* p_attr)
{
	if (close(server_socket) != 0)
	{
		perror("ERROR clean up - could not close server_socket");
		return -1;
	}

	if (pthread_mutex_destroy(&mutex_csd) != 0)
	{
		perror("ERROR clean up - could not destroy mutex_csd");
		return -1;
	}

	if (pthread_cond_destroy(&cond_csd) != 0)
	{
		perror("ERROR clean up - could not destroy cond_csd");
		return -1;
	}

	if (pthread_attr_destroy(p_attr) != 0)
	{
		printf("ERROR clean up - could not destroy attributes\n");
		return -1;
	}

	int storage_shutdown_res = -1;
	if (shutdown_1(&storage_shutdown_res, p_client) != RPC_SUCCESS)
	{
		clnt_perror (p_client, "ERROR clean up - could not shut down the storage server\n");
		return -1;
	}

	clnt_destroy (p_client);

	return 0;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// manage_request
///////////////////////////////////////////////////////////////////////////////////////////////////

void* manage_request(void* p_args)
{
	int socket = 0;

	// lock the main thread until the socket is copied
	if (pthread_mutex_lock(&mutex_csd) != 0)
	{
		printf("ERROR manage_request - could not lock mutex\n");
		return NULL;
	}

	memcpy(&socket, p_args, sizeof(socket));
	is_copied = 1;
	
	// notify that socket was copied
	if (pthread_cond_signal(&cond_csd) != 0)
		printf("ERROR manage_request - could not signal condition\n");

	if (pthread_mutex_unlock(&mutex_csd) != 0)
		printf("ERROR manage_request - could not unlock mutex\n");

	// process the request
	identify_and_process_request(socket);

	pthread_exit(NULL);
}



void identify_and_process_request(int socket)
{	
	char req_type[MAX_REQ_TYPE_LEN + 1];
	read_line(socket, req_type, MAX_REQ_TYPE_LEN);
	req_type[MAX_REQ_TYPE_LEN] = '\0'; // just in case if the request type is in wrong format

	printf("s> %s FROM ", req_type); // print operation

	// process request type
	if (strcmp(req_type, REQ_REGISTER) == 0)
		register_user(socket);
	else if (strcmp(req_type, REQ_CONNECT) == 0)
		connect_user(socket);
	else if (strcmp(req_type, REQ_DISCONNECT)==0)
		disconnect_user(socket);
	else if (strcmp(req_type, REQ_UNREGISTER) == 0)
		unregister(socket);
	else if (strcmp(req_type, REQ_LIST_USERS) == 0)
		list_users(socket);
	else if (strcmp(req_type, REQ_LIST_CONTENT) == 0)
		list_content(socket);
	else if (strcmp(req_type, REQ_PUBLISH) == 0)
		publish_content(socket);
	else if (strcmp(req_type, REQ_DELETE_PUBLISH) == 0)
		delete_published_content(socket);
	else
		printf("ERROR identify_and_process_request - no such request type\n");

	// close the client socket
	if (close(socket) != 0)
		perror("ERROR manage request - could not close client socket");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// register
///////////////////////////////////////////////////////////////////////////////////////////////////

void register_user(int socket)
{
	uint8_t result = REGISTER_SUCCESS;
	
	char username[MAX_USERNAME_LEN + 1];
	if (safe_socket_read(socket, username, MAX_USERNAME_LEN) > 0)	// username specified
	{
		printf("%s\n", username);
		if (is_username_valid(username))
		{
			int add_user_res = -1;
			if(add_user_1(username, &add_user_res, p_client) == RPC_SUCCESS)
			{
				switch (add_user_res)
				{
					case ADD_CONNECTED_USER_SUCCESS 	: 
						result = REGISTER_SUCCESS; break;
					case ADD_CONNECTED_USER_ERR_EXISTS 	: 
						result = REGISTER_NON_UNIQUE_USERNAME; break;
					default : 
						result = REGISTER_OTHER_ERROR;
						printf("ERROR register user - unknown error in create_user\n");
				}
			}
			else
			{
				clnt_perror (p_client, "ERROR register user - rpc error");
				result = REGISTER_OTHER_ERROR;
			}
		}
		else
		{
			printf("ERROR register_user - invalid username\n");
			result = REGISTER_OTHER_ERROR;
		}
	}
	else	// no username
	{
		printf("\nERROR register_user - no username\n");
		result = REGISTER_OTHER_ERROR;
	}
	
	if (send_msg(socket, (char*) &result, 1) != 0)
		printf("ERROR register_user - could not send message\n");
}



int is_username_valid(char* username)
{
	int res = 1;

	regex_t regex;
	if (regcomp(&regex, "^[A-Za-z0-9_]+$", REG_EXTENDED | REG_NOSUB) != 0)
	{
		printf("ERROR is_username_valid - wrong regex\n");
		return 0;
	}

	if (regexec(&regex, username, 0, NULL, 0) != 0)	// no match
		res = 0;

	regfree(&regex);

	return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// unregister
///////////////////////////////////////////////////////////////////////////////////////////////////

void unregister(int socket)
{
	uint8_t res = UNREGISTER_SUCCESS;

	char username[MAX_USERNAME_LEN + 1];
	if (safe_socket_read(socket, username, MAX_USERNAME_LEN) > 0)	// username specified
	{
		printf("%s\n", username);
		
		int delete_res = -1;
		if (delete_user_1(username, &delete_res, p_client) == RPC_SUCCESS)
		{
			switch (delete_res)
			{
				case DELETE_USER_SUCCESS 		: res = UNREGISTER_SUCCESS; break;
				case DELETE_USER_ERR_NOT_EXISTS : res = UNREGISTER_NO_SUCH_USER; break;
				default :
					res = UNREGISTER_OTHER_ERROR;
					printf("ERROR unregister - unknown error in delete_user: %d\n", delete_res);
			}
		}
		else // RPC error
		{
			clnt_perror(p_client, "ERROR unregister - RPC error");
			res = UNREGISTER_OTHER_ERROR;
		}
	}
	else // no username specified
	{
		printf("\nERROR unregister - no username specified\n");
		res = UNREGISTER_OTHER_ERROR;
	}
	
	if (send_msg(socket, (char*) &res, 1) != 0)
		printf("ERROR unregister - could not send response\n");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// connect
///////////////////////////////////////////////////////////////////////////////////////////////////

void connect_user(int socket)
{
	uint8_t res = CONNECT_USER_SUCCESS;

	char username[MAX_USERNAME_LEN + 1];
	if (safe_socket_read(socket, username, MAX_USERNAME_LEN) > 0) // username specified
	{
		printf("%s\n", username);
		
		char port[MAX_PORT_STR_SIZE + 1];
		if (safe_socket_read(socket, port, MAX_PORT_STR_SIZE) > 0)
		{
			// obtain user's ip address
			struct sockaddr_in addr;
			socklen_t size = sizeof(addr);
			bzero(&addr, size);
			if (getpeername(socket, (struct sockaddr*) &addr, &size) == 0)
			{
				int add_connected_res = -1;
				char port[MAX_PORT_STR_LEN];
				sprintf(port, "%d", addr.sin_port);
				if (add_connected_user_1(username, inet_ntoa(addr.sin_addr), port, 
					&add_connected_res, p_client) == RPC_SUCCESS)
				{
					switch (add_connected_res)
					{
						case ADD_CONNECTED_USER_SUCCESS : 
							res = CONNECT_USER_SUCCESS; break;
						case ADD_CONNECTED_USER_ERR_EXISTS : 
							res = CONNECT_USER_ERR_ALREADY_CONNECTED; break;
						case ADD_CONNECTED_USER_ERR_NOT_REGISTERED :
							res = CONENCT_USER_ERR_NOT_REGISTERED; break;
						default :
							printf("ERROR connect_user - unknown error in add_connected_user\n");
							res = CONNECT_USER_ERR_OTHER;
					}
				}
				else // RPC fail
				{
					clnt_perror(p_client, "ERROR connect_user - RPC error");
					res = CONNECT_USER_ERR_OTHER;
				}
			}
			else // could not obtain ip address of the user
			{
				perror("ERROR connect_user - could not obtain the peer's address:");
				res = CONNECT_USER_ERR_OTHER;
			}
			
		}
		else // port not specified
		{
			printf("ERROR connect_user - no port specified\n");
			res = CONNECT_USER_ERR_OTHER;
		}
	}
	else // username not specified
	{
		printf("\nERROR connect_user - no username specified\n");
		res = CONNECT_USER_ERR_OTHER;
	}
	
	if (send_msg(socket, (char*) &res, 1) != 0)
		printf("ERROR connect - could not send response\n");
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// disconnect
///////////////////////////////////////////////////////////////////////////////////////////////////

void disconnect_user(int socket){

	/*Error for is_connected*/
	int is_connected_res;

	//variable for the output of error
	uint8_t res= DISCONNECT_USER_SUCCESS;

	char username[MAX_USERNAME_LEN+1];
	/*read from socket*/
	if(safe_socket_read(socket,username, MAX_USERNAME_LEN)>0)
	{
		printf("%s\n", username);

		int delete_user_res = -1;
		if(delete_connected_user_1(username, &delete_user_res, p_client) == RPC_SUCCESS)
		{
			switch (delete_user_res)
			{
				case DELETE_CONNECTED_USER_SUCCESS :
					res = DISCONNECT_USER_SUCCESS; break;
				case DELETE_CONNECTED_USER_ERR_NOT_REGISTERED : 
					res = DISCONNECT_USER_ERR_NOT_REGISTERED; break;
				case DELETE_CONNECTED_USER_ERR_NOT_CONNECTED :
					res = DISCONNECT_USER_ERR_NOT_CONNECTED; break;
				default :
					res = DISCONNECT_USER_ERR_OTHER;
					printf("ERROR disconnect_user - unknown error in delete_connected_user\n");
			}
		}
		else // RPC error
		{
			clnt_perror(p_client, "ERROR disconnect_user - rpc error");
			res = DISCONNECT_USER_ERR_OTHER;
		}
	}
	else // no username specified
	{
		printf("\nERROR disconnect_user - no username specified\n");
		res= DISCONNECT_USER_ERR_OTHER;
	}
	
	/*sending back response to the client*/
	if(send_msg(socket,(char*)&res,1)!=0)
		printf("ERROR disconnect_user - unable to send response");

}



///////////////////////////////////////////////////////////////////////////////////////////////////
// list_users
///////////////////////////////////////////////////////////////////////////////////////////////////

void list_users(int socket)
{
	uint8_t res = LIST_USERS_SUCCESS;
	// user** users_list = NULL;

	// char username[MAX_USERNAME_LEN + 1];
	// if (safe_socket_read(socket, username, MAX_USERNAME_LEN) > 0) // if user specified
	// {
	// 	printf("%s\n", username);
	// 	if (is_registered(username))
	// 	{
	// 		int is_connected_res;
	// 		if (is_connected(username, &is_connected_res))
	// 		{
	// 			if (is_connected_res == IS_CONNECTED_SUCCESS)
	// 			{
	// 				if (get_connected_users(&users_list) != GET_CONNECTED_USERS_SUCCESS)
	// 				{
	// 					printf("ERROR list_users - unknown error is get_connected_users\n");
	// 					res = LIST_USERS_OTHER_ERROR;
	// 				}
	// 			}
	// 			else // mutex errors
	// 			{
	// 				printf("ERROR list_users - unknown error in is_connected\n");
	// 				res = LIST_USERS_OTHER_ERROR;
	// 			}
	// 		}
	// 		else
	// 			res = LIST_USERS_DISCONNECTED;
	// 	}
	// 	else
	// 		res = LIST_USERS_NO_SUCH_USER;
	// }
	// else // no username specified
	// {
	// 	printf("\nERROR list_users - no user specified");
	// 	res = LIST_USERS_OTHER_ERROR;
	// }

	// send result

	// if (send_msg(socket, (char*) &res, 1) == 0)
	// {
	// 	if (res == LIST_USERS_SUCCESS)
	// 	{
	// 		int send_res = send_users_list(socket, users_list);

	// 		if (send_res != SEND_USERS_LIST_SUCCESS)
	// 			printf("ERROR list_users - could not send users. Code: %d\n", send_res);
	// 	}
	// }
	// else
	// 	printf("ERROR list_users - could not send response\n");

	// if (users_list != NULL)
	// {
	// 	int num_of_conn_users = vector_size(users_list);
	// 	for (int i = 0; i < num_of_conn_users; i++)
	// 		free(users_list[i]);
	// 	vector_free(users_list);
	// }
}



int send_users_list(int socket, user** users_list)
{
	// // send number of users
	// uint32_t num_of_users = (uint32_t) vector_size(users_list);
	// char str_num_of_users[MAX_NUMBER_OF_CONNECTED_USERS_STR_LEN + 1]; // max number of users is 4 000 000
	// sprintf(str_num_of_users, "%d", num_of_users);
	// if (send_msg(socket, str_num_of_users, strlen(str_num_of_users) + 1) != 0)
	// 	return SEND_USERS_LIST_ERR_NUM_OF_USERS;

	// char addr[14];

	// // send users' data
	// for (uint32_t i = 0; i < num_of_users; i++)
	// {
	// 	if (send_msg(socket, users_list[i]->username, strlen(users_list[i]->username) + 1) != 0)
	// 		return SEND_USERS_LIST_ERR_USERNAME;
		
	// 	strcpy(addr, inet_ntoa(users_list[i]->ip));
	// 	if (send_msg(socket, addr, strlen(addr) + 1) != 0)
	// 		return SEND_USERS_LIST_ERR_IP;

	// 	if (send_msg(socket, users_list[i]->port, strlen(users_list[i]->port) + 1) != 0)
	// 		return SEND_USERS_LIST_ERR_PORT;
	// }

	return SEND_USERS_LIST_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// list_content
///////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t assure_requester_correctness_for_list_content(int socket)
{
	char username[MAX_USERNAME_LEN + 1];
	if (safe_socket_read(socket, username, MAX_USERNAME_LEN) <= 0) // if requesting user specified
	{
		printf("\nERROR list_content - no requesting user specified\n");
		return LIST_CONTENT_OTHER_ERROR;
	}

	printf("%s\n", username);

	int is_registered_res = 0;
	if (is_registered_1(username, &is_registered_res, p_client) != RPC_SUCCESS)
	{
		clnt_perror(p_client, "ERROR list_content - rpc error is_registered:");
		return LIST_CONTENT_OTHER_ERROR;
	}

	if (!is_registered_res)
		return LIST_CONTENT_NOT_REGISTERED;

	int is_connected_res = 0;
	if (is_connected_1(username, &is_connected_res, p_client) != RPC_SUCCESS)
	{
		clnt_perror(p_client, "ERROR list_content - rpc error is_connected");
		return LIST_CONTENT_OTHER_ERROR;
	}
	
	if (!is_connected_res)
		return LIST_CONTENT_DISCONNECTED;

	return LIST_CONTENT_SUCCESS;
}



int send_content_list(int socket, char** content_list, uint32_t num_of_files)
{
	// send number of files
	char str_num_of_files[20];
	sprintf(str_num_of_files, "%d", num_of_files);

	if (send_msg(socket, str_num_of_files, strlen(str_num_of_files) + 1) != 0)
		return SEND_CONTENT_LIST_ERR_NUM_OF_FILES;

	// send users' data
	for (uint32_t i = 0; i < num_of_files; i++)
	{
		printf("file: %s\n", content_list[i]);
		if (send_msg(socket, content_list[i], strlen(content_list[i]) + 1) != 0)
			return SEND_CONTENT_LIST_ERR_SEND;
	}

	return SEND_CONTENT_LIST_SUCCESS;
}



void send_list_content_result(int socket, uint8_t res, char** files, int num_of_files)
{
	if (send_msg(socket, (char*) &res, 1) == 0)
	{
		if (res == LIST_CONTENT_SUCCESS)
		{
			int send_res = send_content_list(socket, files, num_of_files);

			if (send_res != SEND_CONTENT_LIST_SUCCESS)
				printf("ERROR list_content - could not send content. Code: %d\n", send_res);
		}
	}
	else
		printf("ERROR list_content - could not send response\n");
}



void list_content(int socket)
{
	get_files_res files_res;
	int res = assure_requester_correctness_for_list_content(socket);
	if (res == LIST_CONTENT_SUCCESS)
	{
		char content_owner[MAX_USERNAME_LEN + 1];
		if (safe_socket_read(socket, content_owner, MAX_USERNAME_LEN) > 0) // content owner specified
		{
			if (get_files_1(content_owner, &files_res, p_client) != RPC_SUCCESS)
			{
				clnt_perror(p_client, "ERROR list_content - rpc error get files\n");
				res = LIST_CONTENT_OTHER_ERROR;
			}
		}
		else // no content owner specified
		{
			printf("ERROR list_content - no owner's username specified\n");
			res = LIST_CONTENT_OTHER_ERROR;
		}
	}

	printf("res: %d\n", files_res.res);
	// char** files = files_res.string_vector_val;
	// int n_of_files = files_res.string_vector_len;
	// send_list_content_result(socket, res, files, n_of_files);

	// if (files != NULL)
	// {
	// 	for (uint32_t i = 0; i < n_of_files; i++)
	// 		free(files[i]);
	// 	free(files);
	// }
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// publish_content
///////////////////////////////////////////////////////////////////////////////////////////////////
	
int publish_content(int socket)
{
	/*To get the error for is_connected*/
	int is_connected_res;
	/*To store the output of the error*/
	uint8_t res= PUBLISH_CONTENT_SUCCESS;

	char username[MAX_USERNAME_LEN + 1];
	char file_name[MAX_FILENAME_LEN + 1];
	char description[MAX_FILE_DESC_LEN + 1];

	if(safe_socket_read(socket,username, MAX_USERNAME_LEN) > 0 && 
		safe_socket_read(socket,file_name, MAX_FILENAME_LEN) > 0 && 
		safe_socket_read(socket,description, MAX_FILE_DESC_LEN) >0)
	{
		printf("%s\n", username);

		int add_file_res = -1;
		if (add_file_1(username, file_name, description, &add_file_res, p_client) == RPC_SUCCESS)
		{
			switch (add_file_res)
			{
				case ADD_FILE_SUCCESS 		   : res = PUBLISH_CONTENT_SUCCESS; break;
				case ADD_FILE_ERR_NO_SUCH_USER : res = PUBLISH_CONTENT_ERR_USER_NONEXISTENT; break;
				case ADD_FILE_ERR_DISCONNECTED : res = PUBLISH_CONTENT_ERR_USER_NOTCONNECTED; break;
				default : 
					res = PUBLISH_CONTENT_ERR_OTHER;
					printf("ERROR publish_content - unknown error in add_file\n");
			}
		}
		else // RPC error
		{
			clnt_perror(p_client, "ERROR publish_content - RPC error");
			res = PUBLISH_CONTENT_ERR_OTHER;
		}
	}
	else
	{
		printf("\nERROR publish_content - not all parameters specified\n");
		res=PUBLISH_CONTENT_ERR_OTHER;
	}

	return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// delete published content
///////////////////////////////////////////////////////////////////////////////////////////////////

int delete_published_content(int socket){

	/*To get the error for is_connected*/
	int is_connected_res;
	
	/*To store the output of the error*/
	uint8_t res= DELETE_PUBLISHED_CONT_SUCCESS;


	// char username[MAX_USERNAME_LEN + 1];
	// char file_name[MAX_FILENAME_LEN + 1];

	// if(safe_socket_read(socket,username, MAX_USERNAME_LEN)>0 && safe_socket_read(socket,file_name, MAX_FILENAME_LEN)>0)
	// {
	// 	/*To check if the user is registered*/
	// 	if(is_registered(username))
	// 	{
	// 		/*To check if the user is connected*/
	// 		if(is_connected(username,&is_connected_res))
	// 		{
	// 			if(is_connected_res==IS_CONNECTED_SUCCESS)
	// 			{
	// 				int delete_res = delete_content_dir(username, file_name);
					
	// 				if (delete_res != DELETE_CONTENT_SUCCESS)
	// 				{
	// 					if (delete_res == DELETE_CONTENT_ERR_FILE_NOTPUB)
	// 					{
	// 						res = DELETE_PUBLISHED_CONT_ERR_FILE_NOTPUB;
	// 					}
	// 					else
	// 					{
	// 						printf("ERROR delete_published_content - unknown error in delete_content_dir\n");
	// 						res = DELETE_PUBLISHED_CONT_ERR_OTHER;
	// 					}
						
	// 				}
	// 			}
	// 			else
	// 			{
	// 				printf("ERROR delete_published_content - unknown error in is_connected\n");
	// 				res=DELETE_PUBLISHED_CONT_ERR_OTHER;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			res=DELETE_PUBLISHED_CONT_ERR_USER_NOTCON;
	// 		}
			
	// 	}
	// 	else
	// 	{
	// 		res=DELETE_PUBLISHED_CONT_ERR_USER_NONEXISTENT;
	// 	}
	// }
	// else
	// {
	// 	printf("ERROR delete_published_content - not all parameteres specified\n");
	// 	res=DELETE_PUBLISHED_CONT_ERR_OTHER;
	// }

	return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// safe socket read
///////////////////////////////////////////////////////////////////////////////////////////////////

int safe_socket_read(int socket, char* read, int max_read_len)
{
	int total_read = read_line(socket, read, max_read_len);
	read[max_read_len] = '\0'; // just in case if the string is not properly ended
	
	return total_read;
}
