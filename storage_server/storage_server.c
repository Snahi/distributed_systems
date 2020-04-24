#include "storage.h"
#include <errno.h>
#include <sys/stat.h>	// for directories



///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
// paths
#define FILE_SEPARATOR "/"
#define STORAGE_DIR_PATH "storage/"
#define CONNECTED_USERS_DIR_PATH "connected_users"

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

bool_t
add_user_1_svc(char *username, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
delete_user_1_svc(char *username, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

	return retval;
}

bool_t
add_connected_user_1_svc(char *username, char *in_addr, char *port, int *result,  struct svc_req *rqstp)
{
	bool_t retval;

	/*
	 * insert server code here
	 */

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
