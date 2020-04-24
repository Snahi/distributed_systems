#include "storage.h"
#include <errno.h>
#include <sys/stat.h>	// for mkdir
#include <dirent.h>		// for opendir



///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
// paths
#define FILE_SEPARATOR "/"
#define STORAGE_DIR_PATH "storage/"
#define CONNECTED_USERS_DIR_PATH "connected_users"
// length limits
#define MAX_USERNAME_LEN 256

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
// add connected user
#define ADD_CONNECTED_USER_SUCCESS 0
#define ADD_CONNECTED_USER_ERR_EXISTS 1
#define ADD_CONNECTED_USER_ERR_DIRECTORY 2
#define ADD_CONNECTED_USER_ERR_LOCK_MUTEX 3
#define ADD_CONNECTED_USER_ERR_UNLOCK_MUTEX 4



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
// setup
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t setup_1_svc(int *p_result, struct svc_req *rqstp)
{
	bool_t retval = 1;

	// create a directory for registered users and their published files if it doesn't exist
    if (mkdir(STORAGE_DIR_PATH, S_IRWXU) != 0 && errno != EEXIST) 								
    {        
		// could not create the directory, but not because the directory had already existed  
		perror("ERROR setup - could not create the storage directory: ");                                                        
        *p_result = SETUP_ERR_STORAGE_DIR_CREATION;     	
		return 0;
    }

    // initialize storage mutex
    if (pthread_mutex_init(&mutex_storage, NULL) != 0)
    {
		printf("ERROR setup - could not initialize the storage mutex\n");
        *p_result = SETUP_ERR_STORAGE_MUTEX_INIT;
		return 0;
    }

	// create a directory for connected users if it doesn't exist
	if (mkdir(CONNECTED_USERS_DIR_PATH, S_IRWXU) != 0 && errno != EEXIST)							
	{
		// could not create the directory, but not because the directory had already existed   
		perror("ERROR setup - could not create the connected users directory:");
		*p_result = SETUP_ERR_CONNECTED_USERS_DIR_CREATION;
		return 0;
	}

    // initialize connected users mutex
    if (pthread_mutex_init(&mutex_connected_users, NULL) != 0)
    {
		printf("ERROR setup - could not initialize the connected users mutex\n");
        *p_result = SETUP_ERR_CONNECTED_USERS_MUTEX_INIT;
		return 0;
    }

	*p_result = SETUP_SUCCESS;

	return retval;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// shutdown
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t shutdown_1_svc(int *p_result, struct svc_req *rqstp)
{
	bool_t retval = 1;

    if (pthread_mutex_destroy(&mutex_storage) != 0)
    {
		printf("ERROR shutdown - could not destroy the storage mutex\n");
		*p_result = SHUTDOWN_ERR_STORAGE_MUTEX;
		return 0;
	}

    if (pthread_mutex_destroy(&mutex_connected_users) != 0)
    {
		printf("ERROR shutdown - could not destroy the connected users mutex\n");
		*p_result = SHUTDOWN_ERR_CONNECTED_USERS_MUTEX;
		return 0;
	}

	*p_result = SHUTDOWN_SUCCESS;

	return retval;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// add user
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t add_user_1_svc(char *username, int *p_result,  struct svc_req *rqstp)
{
	bool_t retval = 1;

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
				retval = 0;  
			}
			else
			{
				perror("ERROR add_user - could not create a directory:");
				*p_result = ADD_USER_ERR_DIRECTORY;  
				retval = 0;
			}          
		}

		if (pthread_mutex_unlock(&mutex_storage) != 0)
		{
			printf("ERROR add_user - could not unlock the storage mutex\n");
			*p_result = ADD_USER_ERR_UNLOCK_MUTEX;
			retval = 0;
		}
	}
	else
	{
		printf("ERROR add_user - could not lock the storage mutex\n");
		*p_result = ADD_USER_ERR_LOCK_MUTEX;
		retval = 0;
	}

    *p_result = ADD_USER_SUCCESS;

	return retval;
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
	bool_t retval = 1;

    // create user directory path. + 1 because additional / to separate dir from file
    int user_folder_path_len = strlen(STORAGE_DIR_PATH) + strlen(username) + 1;
    char dir_path[user_folder_path_len + 1]; 
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, username);
    strcat(dir_path, FILE_SEPARATOR);

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
				*p_result = DELETE_USER_ERR_REMOVE_FOLDER;
        }
        else if (del_files_res == DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER)
        {
			*p_result = DELETE_USER_ERR_NOT_EXISTS;
			retval = 0;
		}
        else
        {
			*p_result = DELETE_USER_ERR_REMOVE_FILE;
			retval = 0;
		}

        // unlock the storage mutex
        if (pthread_mutex_unlock(&mutex_storage) != 0)
        {
            *p_result = DELETE_USER_ERR_MUTEX_UNLOCK;
			retval = 0;
            printf("ERROR delete_user - could not unlock mutex\n");
        }
    }
    else // couldn't acquire the storage mutex
    {
        *p_result = DELETE_USER_ERR_MUTEX_LOCK;
        printf("ERROR delete_user - could not lock mutex\n");
		retval = 0;
    }

	*p_result = DELETE_USER_SUCCESS;

	return retval;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// add connected user
///////////////////////////////////////////////////////////////////////////////////////////////////

bool_t add_connected_user_1_svc(char *username, char *in_addr, char *port, int *p_result,  
	struct svc_req *rqstp)
{
	bool_t retval = 1;

	// create connected user directory path
    char dir_path[strlen(CONNECTED_USERS_DIR_PATH) + strlen(username) + 1];
    strcpy(dir_path, CONNECTED_USERS_DIR_PATH);
    strcat(dir_path, username);

	if (pthread_mutex_lock(&mutex_connected_users) == 0)
	{
		// create user directory
		if (mkdir(dir_path, S_IRWXU) != 0)
		{          
			if (errno == EEXIST)     
			{
				*p_result = ADD_CONNECTED_USER_ERR_EXISTS;
				retval = 0;  
			}
			else
			{
				perror("ERROR add_connected_user - could not create a directory:");
				*p_result = ADD_CONNECTED_USER_ERR_DIRECTORY;  
				retval = 0;
			}          
		}

		if (pthread_mutex_unlock(&mutex_connected_users) != 0)
		{
			printf("ERROR add_connected_user - could not unlock the mutex\n");
			*p_result = ADD_CONNECTED_USER_ERR_UNLOCK_MUTEX;
			retval = 0;
		}
	}
	else
	{
		printf("ERROR add_connected_user - could not lock the mutex\n");
		*p_result = ADD_CONNECTED_USER_ERR_LOCK_MUTEX;
		retval = 0;
	}

    *p_result = ADD_CONNECTED_USER_SUCCESS;

	return retval;
}

bool_t
delete_connected_user_1_svc(char *username, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
get_connected_users_1_svc(users_vector *result, struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
add_file_1_svc(char *username, char *file_name, char *description, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
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
