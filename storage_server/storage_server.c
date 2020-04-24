#include "storage.h"
#include <errno.h>
#include <sys/stat.h>	// for mkdir
#include <dirent.h>		// for opendir
#include <unistd.h>		// for access
#include <string.h>



///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
// paths
#define FILE_SEPARATOR "/"
#define STORAGE_DIR_PATH "storage/"
#define CONNECTED_USERS_DIR_PATH "connected_users"
// length limits
#define MAX_USERNAME_LEN 256
#define IP_STR_LEN 32
#define PORT_STR_LEN 5

// methods results ////////////////////////////////////////////////////////////////////////////////
// setup
#define SETUP_SUCCESS 0
#define SETUP_ERR_STORAGE_DIR_CREATION 1
#define SETUP_ERR_CONNECTED_USERS_DIR_CREATION 2
#define SETUP_ERR_STORAGE_MUTEX_INIT 3
#define SETUP_ERR_CONNECTED_USERS_MUTEX_INIT 4
// shutdown
#define SHUTDOWN_SUCCESS 0
#define SHUTDOWN_ERR_STORAGE_MUTEX 1
#define SHUTDOWN_ERR_CONNECTED_USERS_MUTEX 2
// add user
#define ADD_USER_SUCCESS 0
#define ADD_USER_ERR_EXISTS 1
#define ADD_USER_ERR_DIRECTORY 2
#define ADD_USER_ERR_LOCK_MUTEX 3
#define ADD_USER_ERR_UNLOCK_MUTEX 4
// delete all user files
#define DELETE_ALL_USER_FILES_SUCCESS 0
#define DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER 1
#define DELETE_ALL_USER_FILES_ERR_REMOVE 2
#define DELETE_ALL_USER_FILES_ERR_CLOSE_DIR 3
// delete user
#define DELETE_USER_SUCCESS 0
#define DELETE_USER_ERR_MUTEX_LOCK 1
#define DELETE_USER_ERR_MUTEX_UNLOCK 2
#define DELETE_USER_ERR_NOT_EXISTS 3
#define DELETE_USER_ERR_REMOVE_FOLDER 4
#define DELETE_USER_ERR_REMOVE_FILE 5
#define DELETE_USER_ERR_DISCONNECT 6
// add connected user
#define ADD_CONNECTED_USER_SUCCESS 0
#define ADD_CONNECTED_USER_ERR_EXISTS 1
#define ADD_CONNECTED_USER_ERR_LOCK_MUTEX 2
#define ADD_CONNECTED_USER_ERR_UNLOCK_MUTEX 3
#define ADD_CONNECTED_USER_ERR_CREATE_FILE 4
#define ADD_CONNECTED_USER_ERR_WRITE_TO_FILE 5
#define ADD_CONNECTED_USER_ERR_CLOSE_FILE 6
#define ADD_CONNECTED_USER_ERR_NOT_REGISTERED 7
// delete connected user
#define DELETE_CONNECTED_USER_SUCCESS 0
#define DELETE_CONNECTED_USER_ERR_LOCK_MUTEX 1
#define DELETE_CONNECTED_USER_ERR_UNLOCK_MUTEX 2
#define DELETE_CONNECTED_USER_ERR_NOT_CONNECTED 3
#define DELETE_CONNECTED_USER_ERR_REMOVE_FILE 4
#define DELETE_CONNECTED_USER_ERR_NOT_REGISTERED 5
// get connected users
#define GET_CONNECTED_USERS_SUCCESS 0
#define GET_CONNECTED_USERS_ERR_LOCK_MUTEX 1
#define GET_CONNECTED_USERS_ERR_UNLOCK_MUTEX 2
#define GET_CONNECTED_USERS_ERR_OPEN_CONNECTED_DIR 3
// add file
#define ADD_FILE_SUCCESS 0
#define ADD_FILE_ERR_CREATE_FILE 1
#define ADD_FILE_ERR_EXISTS 2
#define ADD_FILE_ERR_LOCK_MUTEX 3
#define ADD_FILE_ERR_UNLOCK_MUTEX 4
#define ADD_FILE_ERR_NO_SUCH_USER 5
#define ADD_FILE_ERR_DISCONNECTED 6



///////////////////////////////////////////////////////////////////////////////////////////////////
// global variables
///////////////////////////////////////////////////////////////////////////////////////////////////
/*
	mutex for accessing anything that is in the storage folder - users and their published files
*/
pthread_mutex_t mutex_storage;
/*
	mutex for accessing connected users folder
*/
pthread_mutex_t mutex_connected_users;



///////////////////////////////////////////////////////////////////////////////////////////////////
// is_registered
///////////////////////////////////////////////////////////////////////////////////////////////////

int is_registered(char* username)
{
    int user_folder_path_len = strlen(STORAGE_DIR_PATH) + strlen(username);
    char dir_path[user_folder_path_len + 1]; 
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, username);

    struct stat st = {0};

    return stat(dir_path, &st) == 0 ? 1 : 0;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// is_connected
///////////////////////////////////////////////////////////////////////////////////////////////////

int is_connected(char* username)
{
	char user_file_path[strlen(CONNECTED_USERS_DIR_PATH) + strlen(username) + 1];
    strcpy(user_file_path, CONNECTED_USERS_DIR_PATH);
    strcat(user_file_path, username);

	if (access(user_file_path, F_OK) == 0) // is connected
		return 1;
	else
		return 0;	
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// setup
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t setup_1_svc(int *p_result, struct svc_req *rqstp)
{
	// create a directory for registered users and their published files if it doesn't exist
    if (mkdir(STORAGE_DIR_PATH, S_IRWXU) != 0 && errno != EEXIST) 								
    {        
		// could not create the directory, but not because the directory had already existed  
		perror("ERROR setup - could not create the storage directory: ");                                                        
        *p_result = SETUP_ERR_STORAGE_DIR_CREATION;     	
		return TRUE;
    }

    // initialize storage mutex
    if (pthread_mutex_init(&mutex_storage, NULL) != 0)
    {
		printf("ERROR setup - could not initialize the storage mutex\n");
        *p_result = SETUP_ERR_STORAGE_MUTEX_INIT;
		return TRUE;
    }

	// create a directory for connected users if it doesn't exist
	if (mkdir(CONNECTED_USERS_DIR_PATH, S_IRWXU) != 0 && errno != EEXIST)							
	{
		// could not create the directory, but not because the directory had already existed   
		perror("ERROR setup - could not create the connected users directory:");
		*p_result = SETUP_ERR_CONNECTED_USERS_DIR_CREATION;
		return TRUE;
	}

    // initialize connected users mutex
    if (pthread_mutex_init(&mutex_connected_users, NULL) != 0)
    {
		printf("ERROR setup - could not initialize the connected users mutex\n");
        *p_result = SETUP_ERR_CONNECTED_USERS_MUTEX_INIT;
		return TRUE;
    }

	*p_result = SETUP_SUCCESS;
	
	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// shutdown
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t shutdown_1_svc(int *p_result, struct svc_req *rqstp)
{
    if (pthread_mutex_destroy(&mutex_storage) != 0)
    {
		printf("ERROR shutdown - could not destroy the storage mutex\n");
		*p_result = SHUTDOWN_ERR_STORAGE_MUTEX;
		return TRUE;
	}

    if (pthread_mutex_destroy(&mutex_connected_users) != 0)
    {
		printf("ERROR shutdown - could not destroy the connected users mutex\n");
		*p_result = SHUTDOWN_ERR_CONNECTED_USERS_MUTEX;
		return TRUE;
	}

	*p_result = SHUTDOWN_SUCCESS;

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// add user
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t add_user_1_svc(char *username, int *p_result,  struct svc_req *rqstp)
{
	*p_result = ADD_USER_SUCCESS;

	// create user directory path
    char dir_path[strlen(STORAGE_DIR_PATH) + strlen(username) + 1];
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, username);

	if (pthread_mutex_lock(&mutex_storage) == 0)
	{
		// create user directory
		if (mkdir(dir_path, S_IRWXU) != 0)
		{          
			if (errno == EEXIST)     
			{
				*p_result = ADD_USER_ERR_EXISTS;
			}
			else
			{
				perror("ERROR add_user - could not create a directory:");
				*p_result = ADD_USER_ERR_DIRECTORY;
			}          
		}

		if (pthread_mutex_unlock(&mutex_storage) != 0)
		{
			printf("ERROR add_user - could not unlock the storage mutex\n");
			*p_result = ADD_USER_ERR_UNLOCK_MUTEX;
		}
	}
	else
	{
		printf("ERROR add_user - could not lock the storage mutex\n");
		*p_result = ADD_USER_ERR_LOCK_MUTEX;
	}

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// delete user
///////////////////////////////////////////////////////////////////////////////////////////////////

int delete_all_user_files(char* user_dir_path, int file_path_len)
{
    int res = DELETE_ALL_USER_FILES_SUCCESS;

    // delete user files
    DIR* p_user_dir = opendir(user_dir_path);
    if (p_user_dir != NULL)
    {
        struct dirent* p_next_file;
        char filepath[file_path_len + 1];

        while ((p_next_file = readdir(p_user_dir)) != NULL )
        {
            if (p_next_file->d_name[0] != '.') // ignore 'non files'
            {
                // build the path for each file in the folder
                sprintf(filepath, "%s%s", user_dir_path, p_next_file->d_name);
                if (remove(filepath) != 0)
                {
                    perror("ERROR delete_all_user_files - could not remove file:");
                    return DELETE_ALL_USER_FILES_ERR_REMOVE;
                }
            }
        }
        
        if (closedir(p_user_dir) != 0)
        {
            perror("ERROR delete_all_user_files - could not close dir:");
            return DELETE_ALL_USER_FILES_ERR_CLOSE_DIR;
        }
    }
    else // no such user
        res = DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER;

    return res;
}



bool_t delete_user_1_svc(char *username, int *p_result,  struct svc_req *rqstp)
{
	*p_result = DELETE_USER_SUCCESS;

    // create user directory path. + 1 because additional / to separate dir from file
    int user_folder_path_len = strlen(STORAGE_DIR_PATH) + strlen(username) + 1;
    char dir_path[user_folder_path_len + 1]; 
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, username);
    strcat(dir_path, FILE_SEPARATOR);

	// disconnect if is connected
	int disconnect_result = DELETE_CONNECTED_USER_SUCCESS;
	if (is_connected(username))
		delete_connected_user_1_svc(username, &disconnect_result, NULL);

	if (disconnect_result == DELETE_CONNECTED_USER_SUCCESS)
	{
		// acquire the storage mutex
		if (pthread_mutex_lock(&mutex_storage) == 0)
		{
			// first delete all user's files
			int file_path_len = user_folder_path_len + MAX_USERNAME_LEN; 
			int del_files_res = delete_all_user_files(dir_path, file_path_len);
			if (del_files_res == DELETE_ALL_USER_FILES_SUCCESS)
			{
				// remove user directory
				if (remove(dir_path) != 0)
				{
					*p_result = DELETE_USER_ERR_REMOVE_FOLDER;
					perror("ERROR delete_user - can't remove file:");
				}
			}
			else if (del_files_res == DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER)
				*p_result = DELETE_USER_ERR_NOT_EXISTS;
			else
				*p_result = DELETE_USER_ERR_REMOVE_FILE;

			// unlock the storage mutex
			if (pthread_mutex_unlock(&mutex_storage) != 0)
			{
				*p_result = DELETE_USER_ERR_MUTEX_UNLOCK;
				printf("ERROR delete_user - could not unlock mutex\n");
			}
		}
		else // couldn't acquire the storage mutex
		{
			*p_result = DELETE_USER_ERR_MUTEX_LOCK;
			printf("ERROR delete_user - could not lock mutex\n");
		}
	}
	else // error during disconnecting
		*p_result = DELETE_USER_ERR_DISCONNECT;

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// add connected user
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t add_connected_user_1_svc(char *username, char *addr, char *port, int *p_result,  
	struct svc_req *rqstp)
{
	*p_result = ADD_CONNECTED_USER_SUCCESS;

	// create connected user directory path
    char user_file_path[strlen(CONNECTED_USERS_DIR_PATH) + strlen(username) + 1];
    strcpy(user_file_path, CONNECTED_USERS_DIR_PATH);
    strcat(user_file_path, username);

	if (is_registered(username))
	{
		if (pthread_mutex_lock(&mutex_connected_users) == 0)
		{
			// check if is connected
			if(access(user_file_path, F_OK) == -1 )	// is not connected
			{
				FILE* user_file = fopen(user_file_path, "w");
				if (user_file != NULL)
				{
					if (fprintf(user_file, "%s\n%s", addr, port) <= 0) // nothing was written
					{
						printf("ERROR add_connected_user - could not wirte to the file\n");
						*p_result = ADD_CONNECTED_USER_ERR_WRITE_TO_FILE;
					}
				}
				else
				{
					perror("ERROR add_connected_user - could not create a file:");
					*p_result = ADD_CONNECTED_USER_ERR_CREATE_FILE;
				}

				if (fclose(user_file) != 0)
				{
					perror("ERROR add_connected_user - could not close file:");
					*p_result = ADD_CONNECTED_USER_ERR_CLOSE_FILE;
				}
			} 
			else // is already connected
				*p_result = ADD_CONNECTED_USER_ERR_EXISTS;

			if (pthread_mutex_unlock(&mutex_connected_users) != 0)
			{
				printf("ERROR add_connected_user - could not unlock the mutex\n");
				*p_result = ADD_CONNECTED_USER_ERR_UNLOCK_MUTEX;
			}
		}
		else
		{
			printf("ERROR add_connected_user - could not lock the mutex\n");
			*p_result = ADD_CONNECTED_USER_ERR_LOCK_MUTEX;
		}
	}
	else // not registered
		*p_result = ADD_CONNECTED_USER_ERR_NOT_REGISTERED;

	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// delete connected user
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t delete_connected_user_1_svc(char *username, int *p_result,  struct svc_req *rqstp)
{
	*p_result = DELETE_CONNECTED_USER_SUCCESS;

    char user_file_path[strlen(CONNECTED_USERS_DIR_PATH) + strlen(username) + 1];
    strcpy(user_file_path, CONNECTED_USERS_DIR_PATH);
    strcat(user_file_path, username);

	if (is_registered(username))
	{
		if (pthread_mutex_lock(&mutex_connected_users) == 0)
		{
			if (remove(user_file_path) != 0)
			{
				if (errno == ENOENT)
					*p_result = DELETE_CONNECTED_USER_ERR_NOT_CONNECTED;
				else
				{
					perror("ERROR delete_connected_user - could not delete file:");
					*p_result = DELETE_CONNECTED_USER_ERR_REMOVE_FILE;
				}
			}

			if (pthread_mutex_unlock(&mutex_connected_users) != 0)
			{
				printf("ERROR delete_connected_user - could not unlock the mutex\n");
				*p_result = DELETE_CONNECTED_USER_ERR_UNLOCK_MUTEX;
			}
		}
		else // couldn't lock mutex
		{
			printf("ERROR delete_connected_user - could not lock the mutex\n");
			*p_result = DELETE_CONNECTED_USER_ERR_LOCK_MUTEX;
		}
	}
	else // not registered
		*p_result = DELETE_CONNECTED_USER_ERR_NOT_REGISTERED;
	
	return TRUE;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// get connected users
///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t count_files(DIR* p_directory)
{
    uint32_t quantity = 0;
    struct dirent* p_next_file;

    while ((p_next_file = readdir(p_directory)) != NULL)
    {
        if (p_next_file->d_name[0] != '.') // skip files which are not users
            ++quantity;
    }

    rewinddir(p_directory);

    return quantity;
}



int get_connected_user_ip_and_port(char* filepath, char* p_ip, char* p_port)
{
	FILE* p_file = fopen(filepath, "r");
	int res = 0;

	if (p_file != NULL)
	{
		if (fscanf(p_file, "%s\n%s", p_ip, p_port) == 2)
			res = 1;
		else
			res = 0;	

		if (fclose(p_file) != 0)
		{
			perror("ERROR could not close connected user file:");
			res = 0;
		}	
	}
	else
	{
		perror("ERROR could not open connected user file:");
		res = 0;
	}

	return res;
}



bool_t get_connected_users_1_svc(users_vector *p_result, struct svc_req *rqstp)
{
	bool_t retval = 1;

	//  // create user directory path
    // int connected_path_len = strlen(CONNECTED_USERS_DIR_PATH) + 1; // + 1 --> /
    // char connected_path[connected_path_len];
	// strcpy(connected_path, CONNECTED_USERS_DIR_PATH);

    // if (pthread_mutex_lock(&mutex_connected_users) == 0)
    // {
    //     DIR* p_connected_users_dir = opendir(connected_path);
    //     if (p_connected_users_dir != NULL)
    //     {
    //         struct dirent* p_next_file;
    //         char* file_name;
    //         int num_of_users = count_files(p_connected_users_dir);
    //         p_result->users_vector_val = malloc(*num_of_users * sizeof(user));
	// 		p_result->users_vector_len = num_of_users;
	// 		user* users = p_result->users_vector_val;

    //         int file_idx = 0;
	// 		char ip[IP_STR_LEN];
	// 		char port[PORT_STR_LEN];
	// 		char file_path[connected_path_len + MAX_USERNAME_LEN];

    //         while ((p_next_file = readdir(p_connected_users_dir)) != NULL)
    //         {
    //             file_name = p_next_file->d_name;
    //             if (file_name[0] != '.') // ignore non user files
    //             {
    //                 users[file_idx].username = malloc((strlen(file_name) + 1) * sizeof(char));
    //                 strcpy(users[file_idx].username, file_name);

	// 				strcpy(file_path, connected_path);
	// 				strcat(file_path, file_name);
	// 				if (get_connected_user_ip_and_port(file_path, ip, port))
	// 				{
	// 					users[file_idx].ip = malloc(IP_STR_LEN * sizeof(char));
	// 					strcpy(users[file_idx].ip, ip);

	// 					users[file_idx].port = malloc(PORT_STR_LEN * sizeof(char));
	// 					strcpy(users[file_idx].port, port);
	// 				}
	// 				else
	// 				{
						
	// 				}
					
    //                 ++file_idx;
    //             }
    //         }

    //         *p_user_files = user_files;
            
    //         if (closedir(p_user_dir) != 0)
    //         {
    //             perror("ERROR get_user_files_list - could not close dir");
    //             res = GET_USER_FILES_LIST_ERR_CLOSE_DIR;
    //         }
    //     }
    //     else // no such user
    //     {
	// 		*p_result = GET_CONNECTED_USERS_ERR_OPEN_CONNECTED_DIR;
	// 		retval = 0;
	// 		perror("ERROR get_connected_users - could not open connected users directory\n");
	// 	}

    //     if (pthread_mutex_unlock(&mutex_connected_users) != 0)
    //     {
    //         *p_result = GET_CONNECTED_USERS_ERR_UNLOCK_MUTEX;
	// 		retval = 0;
    //         printf("ERROR get_connected_users - could not unlock mutex\n");
    //     }
    // }
    // else // couldn't lock mutex
    // {
    //     *p_result = GET_CONNECTED_USERS_ERR_LOCK_MUTEX;
	// 	retval = 0;
    //     printf("ERROR get_connected_users- could not lock mutex\n");
    // }

	return retval;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// add file
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t add_file_1_svc(char *username, char *file_name, char *description, int *result,  
	struct svc_req *rqstp)
{
 	/*variable to store error*/
    int res= ADD_FILE_SUCCESS;

	if (is_registered(username))
	{
		if (is_connected(username))
		{
			/*File pointer to hold the reference to the file*/
			FILE * fileptr;

			/*directory path*/
			char dir_path[strlen(STORAGE_DIR_PATH) + strlen(username)+ strlen(FILE_SEPARATOR) +
				strlen(file_name) + 1];
			strcpy(dir_path, STORAGE_DIR_PATH);
			strcat(dir_path, username);
			strcat(dir_path, FILE_SEPARATOR);
			strcat(dir_path,file_name);

			if(pthread_mutex_lock(&mutex_storage)==0)
			{
				/*checks if the file with that filename already exists*/
				if(access(dir_path, F_OK)==-1)	// doesn't exist
				{
					/*create a new file*/
					fileptr=fopen(dir_path,"w");
						
					/*Checking if the file was created successfully*/
					if(fileptr==NULL)
					{
						res=ADD_FILE_ERR_CREATE_FILE; 
					}
					else
					{
						fputs(description,fileptr);
						fclose(fileptr);
					}
				}
				else
				{
					res=ADD_FILE_ERR_EXISTS;
				} 
				
				// unlock the storage mutex
				if (pthread_mutex_unlock(&mutex_storage) != 0)
				{
					res = ADD_FILE_ERR_UNLOCK_MUTEX;
					printf("ERROR add_file - could not unlock mutex\n");
				}

			}
			else
			{
				printf("ERROR add_file - could not lock mutex\n");
				res=ADD_FILE_ERR_LOCK_MUTEX;
			}
		}
		else // not registered
			res = ADD_FILE_ERR_DISCONNECTED;
	}
	else // not registered
		res = ADD_FILE_ERR_NO_SUCH_USER;

	return TRUE;
}

bool_t
delete_file_1_svc(char *username, char *file_name, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
get_files_1_svc(char *username, files_vector *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

int
storage_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
