#include <stdint.h>
#include "vec.h"
#include <netinet/in.h>
/*
    encapsulates functions dealing with physicall storage.
    IMPORTANT before any operation will be performed it is required to call the init() function and
    if the file won't be used anymore then the destroy() function must be called.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_IP_ADDR_LEN 15
// init
#define INIT_USER_DAO_SUCCESS 0
#define INIT_USER_DAO_ERR_FOLDER_CREATION 1
#define INIT_USER_DAO_ERR_STORAGE_MUTEX_INIT 2
#define INIT_USER_DAO_ERR_CONNECTED_USERS_MUTEX_INIT 3
// destroy
#define DESTROY_USER_DAO_SUCCESS 0
#define DESTROY_USER_DAO_ERR_STORAGE_MUTEX 1
#define DESTROY_USER_DAO_ERR_CONNECTED_USERS_MUTEX 2
#define DESTROY_USER_DAO_ERR_LOCK_CONNECTED_USERS_MUTEX 3
#define DESTROY_USER_DAO_ERR_UNLOCK_CONNECTED_USERS_MUTEX 4
// create user
#define CREATE_USER_SUCCESS 0
#define CREATE_USER_ERR_EXISTS 1
#define CREATE_USER_ERR_DIRECTORY 2
// delete user
#define DELETE_USER_SUCCESS 0
#define DELETE_USER_ERR_MUTEX_LOCK 1
#define DELETE_USER_ERR_MUTEX_UNLOCK 2
#define DELETE_USER_ERR_NOT_EXISTS 3
#define DELETE_USER_ERR_REMOVE_FOLDER 4
#define DELETE_USER_ERR_REMOVE_FILE 5
// get user files list
#define GET_USER_FILES_LIST_SUCCESS 0
#define GET_USER_FILES_LIST_ERR_NO_SUCH_USER 1
#define GET_USER_FILES_LIST_ERR_MUTEX_LOCK 2
#define GET_USER_FILES_LIST_ERR_MUTEX_UNLOCK 3
#define GET_USER_FILES_LIST_ERR_CLOSE_DIR 4
// connected users
#define MAX_USERNAME_LEN 256
// add connected user
#define ADD_CONNECTED_USERS_SUCCESS 0
#define ADD_CONNECTED_USERS_ALREADY_EXISTS 1
#define ADD_CONNECTED_USER_FULL 2
#define ADD_CONNECTED_USER_MUTEX_LOCK_ERROR 3
#define ADD_CONNECTED_USER_MUTEX_UNLOCK_ERROR 4
// get connected users lis
#define GET_CONNECTED_USERS_LIST_SUCCESS 0
#define GET_CONNECTED_USERS_LIST_ERR_LOCK_MUTEX 1
#define GET_CONNECTED_USERS_LIST_ERR_UNLOCK_MUTEX 2
// is connected
#define IS_CONNECTED_SUCCESS 0
#define IS_CONNECTED_ERR_LOCK_MUTEX 1
#define IS_CONNECTED_ERR_UNLOCK_MUTEX 2
//remove connected user
#define REMOVE_CONNECTED_USERS_SUCCESS 0
#define REMOVE_CONNECTED_USERS_ERR_LOCK_MUTEX 1
#define REMOVE_CONNECTED_USERS_ERR_UNLOCK_MUTEX 2
//publish content
#define PUBLISH_DIR_SUCCESS 0
#define PUBLISH_DIR_ERR_EXISTS 1
#define PUBLISH_DIR_ERR_DIRECTORY 2
#define PUBLISH_DIR_ERR_FILE_NOTCREATED 3
#define PUBLISH_DIR_ERR_MUTEX_LOCK 4
#define PUBLISH_DIR_ERR_MUTEX_UNLOCK 5

///////////////////////////////////////////////////////////////////////////////////////////////////
// structs
///////////////////////////////////////////////////////////////////////////////////////////////////

struct user_data {
	char username[MAX_USERNAME_LEN + 1];
	struct in_addr ip;
	char port[6];
};

typedef struct user_data user;



/*
    must be called exactly once at the beginning, before the first call to any function 
    from this file has been done.
    Returns:
        INIT_USER_DAO_SUCCESS               - success
        INIT_USER_DAO_ERR_FOLDER_CREATION   - could not create the storage folder
        INIT_USER_DAO_ERR_MUTEX_INIT        - could not initialize the storage mutex
*/
int init_user_dao();
/*
    must be called exactly once when the functions won't be used anymore.
    Returns:
        DESTROY_USER_DAO_SUCCESS    - success
        DESTROY_USER_DAO_ERR_MUTEX  - could not destroy the storage mutex
*/
int destroy_user_dao();
/*
    creates a new user.
    Returns:
        CREATE_USER_SUCCESS         - success
        CREATE_USER_ERR_EXISTS      - user with such username already exists
        CREATE_USER_ERR_DIRECTORY   - could not create user directory
*/
int create_user(char* username);
/*
    deletes the user with the specified username.
    Returns:
        DELETE_USER_SUCCESS             - success
        DELETE_USER_ERR_MUTEX_LOCK      - could not lock the storage mutex
        DELETE_USER_ERR_MUTEX_UNLOCK    - could not unlock the storage mutex
        DELETE_USER_ERR_NOT_EXISTS      - there is no user with such username
        DELETE_USER_ERR_REMOVE_FOLDER   - could not delete the user folder
        DELETE_USER_ERR_REMOVE_FILE     - could not delete files from user folder
*/
int delete_user(char* username);
/*
    collects names of all files of the user with the specified username in a dynamically allocated
    array of dynamically allocated strings, so it has to be deleted afterwards. Number of the 
    files will be put where the p_quantity points.
    Returns:
        GET_USER_FILES_LIST_SUCCESS             - success
        GET_USER_FILES_LIST_ERR_NO_SUCH_USER    - there is no user with such username
        GET_USER_FILES_LIST_ERR_MUTEX_LOCK      - could not lock the storage mutex
        GET_USER_FILES_LIST_ERR_MUTEX_UNLOCK    - could not unlock the storage mutex
        GET_USER_FILES_LIST_ERR_CLOSE_DIR       - could not close the user directory
*/
int get_user_files_list(char* username, char*** p_user_files, uint32_t* p_quantity);
/*
	checks if the user with the specified username is registered.
	Returns 1 if the user is registered and 0 if no
*/
int is_registered(char* username);
/*
    Adds the user to the connected users vector.
    Returns:
        ADD_CONNECTED_USERS_SUCCESS             - success
        ADD_CONNECTED_USERS_ALREADY_EXISTS      - user is already connected
        ADD_CONNECTED_USER_FULL                 - limit of connected users reached
        ADD_CONNECTED_USER_MUTEX_LOCK_ERROR     - could not lock mutex_connected_users
        ADD_CONNECTED_USER_MUTEX_UNLOCK_ERROR   - could not unlock mutex_connected_users
*/
int add_connected_user(char* name, struct in_addr ip, char* port);

int get_connected_users(user*** p_users);
/*
    p_error is a pointer to a place where result code will be put.
    result codes:
        IS_CONNECTED_SUCCESS            - success
        IS_CONNECTED_ERR_LOCK_MUTEX     - could not lock mutex_connected_users
        IS_CONNECTED_ERR_UNLOCK_MUTEX   - could not unlock mutex_connected_users
    Returns:
        1 - user with the username is connected
        0 - user with the username is not connected
*/
int is_connected(char* username, int* p_err);

int remove_connected_user(char*name);

int publish_content_dir(char* name,char* file_name,char* descr );