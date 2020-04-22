#include "user_dao.h"
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h> 
#include <unistd.h>



///////////////////////////////////////////////////////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////////////////////////////////////////////////////
// paths
#define STORAGE_DIR_PATH "storage/"
#define STORAGE_SLASH "/"
// names lengths
#define MAX_FILENAME_LEN 256
// delete_all_user_files
#define DELETE_ALL_USER_FILES_SUCCESS 0
#define DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER 1
#define DELETE_ALL_USER_FILES_ERR_REMOVE 2
#define DELETE_ALL_USER_FILES_ERR_CLOSE_DIR 3
// connect
#define MAX_NUMBER_OF_CONNECTED_USERS 4000000




///////////////////////////////////////////////////////////////////////////////////////////////////
// global variables
///////////////////////////////////////////////////////////////////////////////////////////////////
pthread_mutex_t mutex_storage;
pthread_mutex_t mutex_connected_users;
user** connected_users;


//////////////////////////////////////////////////////////////////////////////////////////////
// init
///////////////////////////////////////////////////////////////////////////////////////////////////

int init_user_dao()
{
    // create the storage directory if it doesn't exist
    if (mkdir(STORAGE_DIR_PATH, S_IRWXU) != 0 && errno != EEXIST)   // error occured, but not 
    {                                                               // because the the folder 
        return INIT_USER_DAO_ERR_FOLDER_CREATION;                   // already existed
    }

    // initialize storage mutex
    if (pthread_mutex_init(&mutex_storage, NULL) != 0)
    {
        return INIT_USER_DAO_ERR_STORAGE_MUTEX_INIT;
    }

    // initialize connected users mutex
    if (pthread_mutex_init(&mutex_connected_users, NULL) != 0)
    {
        return INIT_USER_DAO_ERR_CONNECTED_USERS_MUTEX_INIT;
    }

    // initialize connected users list
    connected_users = vector_create();

    return INIT_USER_DAO_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// destroy
///////////////////////////////////////////////////////////////////////////////////////////////////

int destroy_user_dao()
{
    if (pthread_mutex_lock(&mutex_connected_users) != 0)
        return DESTROY_USER_DAO_ERR_LOCK_CONNECTED_USERS_MUTEX;
    else
    {
         // free connected users memory
        int numOfConnectedUsers = vector_size(connected_users);
        for (int i = 0; i < numOfConnectedUsers; i++)
        {
            free(connected_users[i]);
        }

        vector_free(connected_users);
        connected_users = NULL;

        if (pthread_mutex_unlock(&mutex_connected_users) != 0)
            return DESTROY_USER_DAO_ERR_UNLOCK_CONNECTED_USERS_MUTEX;
    }

    if (pthread_mutex_destroy(&mutex_storage) != 0)
        return DESTROY_USER_DAO_ERR_STORAGE_MUTEX;

    if (pthread_mutex_destroy(&mutex_connected_users) != 0)
        return DESTROY_USER_DAO_ERR_CONNECTED_USERS_MUTEX;

    return DESTROY_USER_DAO_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// create_user
///////////////////////////////////////////////////////////////////////////////////////////////////

int create_user(char* name)
{
    // create user directory path
    char dir_path[strlen(STORAGE_DIR_PATH) + strlen(name) + 1];
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, name);

    // create user directory
    if (mkdir(dir_path, S_IRWXU) != 0)
    {          
        if (errno == EEXIST)     
            return CREATE_USER_ERR_EXISTS;    
        else
        {
            perror("ERROR create_user - could not create directory");
            return CREATE_USER_ERR_DIRECTORY;   
        }          
    }

    return CREATE_USER_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// publish_content_dir
///////////////////////////////////////////////////////////////////////////////////////////////////


int publish_content_dir(char* name, char* file_name, char* descr){

    /*variable to store error*/
    int res= PUBLISH_DIR_SUCCESS;

     /*File pointer to hold the reference to the file*/
    FILE * fileptr;

    /*directory path*/
    char dir_path[strlen(STORAGE_DIR_PATH) + strlen(name)+ strlen(STORAGE_SLASH)+strlen(file_name) + 1];
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, name);
    strcat(dir_path, STORAGE_SLASH);
    strcat(dir_path,file_name);

    
    if(pthread_mutex_lock(&mutex_storage)==0)
    {
        /*checks if the file with that filename already exists*/
        if(access(dir_path, F_OK)==-1)
        {
            /*Opens the file_name that was earlier created*/
            fileptr=fopen(dir_path,"w");
                
            /*Checking if the file was created successfully*/
            if(fileptr==NULL)
            {
                res=PUBLISH_DIR_ERR_FILE_NOTCREATED;
                
            }
            else
            {
                /*fputs(data, file_name);*/
                fputs(descr,fileptr);
                fclose(fileptr);
            }
        }
        else
        {
            res=PUBLISH_DIR_ERR_EXISTS;
        } 
        
        // unlock the storage mutex
        if (pthread_mutex_unlock(&mutex_storage) != 0)
        {
            res = PUBLISH_DIR_ERR_MUTEX_UNLOCK;
            printf("ERROR publish_directory - could not unlock mutex\n");
        }

    }
    else
    {
        res=PUBLISH_DIR_ERR_MUTEX_LOCK;
    }

    return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// delete_user
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
                    perror("ERROR delete_all_user_files - could not remove file");
                    return DELETE_ALL_USER_FILES_ERR_REMOVE;
                }
            }
        }
        
        if (closedir(p_user_dir) != 0)
        {
            perror("ERROR delete_all_user_files - could not close dir");
            return DELETE_ALL_USER_FILES_ERR_CLOSE_DIR;
        }
    }
    else // no such user
        res = DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER;

    return res;
}



int delete_user(char* name)
{
    int res = DELETE_USER_SUCCESS;

    // create user directory path. + 1 because additional / to separate dir from file
    int user_folder_path_len = strlen(STORAGE_DIR_PATH) + strlen(name) + 1;
    int file_path_len = user_folder_path_len + MAX_FILENAME_LEN; 
    char dir_path[user_folder_path_len + 1]; 
    strcpy(dir_path, STORAGE_DIR_PATH);
    strcat(dir_path, name);
    strcat(dir_path, STORAGE_SLASH);

    // acquire the storage mutex
    if (pthread_mutex_lock(&mutex_storage) == 0)
    {
        // first delete all user's files
        int del_files_res = delete_all_user_files(dir_path, file_path_len);
        if (del_files_res == DELETE_ALL_USER_FILES_SUCCESS)
        {
            // remove user directory
            if (remove(dir_path) != 0)
                res = DELETE_USER_ERR_REMOVE_FOLDER;
        }
        else if (del_files_res == DELETE_ALL_USER_FILES_ERR_NO_SUCH_USER)
            res = DELETE_USER_ERR_NOT_EXISTS;
        else
            res = DELETE_USER_ERR_REMOVE_FILE;

        // unlock the storage mutex
        if (pthread_mutex_unlock(&mutex_storage) != 0)
        {
            res = DELETE_USER_ERR_MUTEX_UNLOCK;
            printf("ERROR delete_user - could not unlock mutex\n");
        }
    }
    else // couldn't acquire the storage mutex
    {
        res = DELETE_USER_ERR_MUTEX_LOCK;
        printf("ERROR delete_user - could not lock mutex\n");
    }

    return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// get_user_files_list
///////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t count_user_files(DIR* p_directory)
{
    uint32_t quantity = 0;
    struct dirent* p_next_file;

    while ((p_next_file = readdir(p_directory)) != NULL)
    {
        if (p_next_file->d_name[0] != '.') // skip files which are not part of the storage
            ++quantity;
    }

    rewinddir(p_directory);

    return quantity;
}



int get_user_files_list(char* username, char*** p_user_files, uint32_t* p_quantity)
{
    int res = GET_USER_FILES_LIST_SUCCESS;

    // create user directory path
    int user_folder_path_len = strlen(STORAGE_DIR_PATH) + strlen(username) + 1; // + 1 --> /
    char user_dir_path[user_folder_path_len + 1]; 
    strcpy(user_dir_path, STORAGE_DIR_PATH);
    strcat(user_dir_path, username);
    strcat(user_dir_path, "/");

	// open user directory
    if (pthread_mutex_lock(&mutex_storage) == 0)
    {
        DIR* p_user_dir = opendir(user_dir_path);
        if (p_user_dir != NULL)
        {
            struct dirent* p_next_file;
            char* file_name;
            *p_quantity = count_user_files(p_user_dir);
            char** user_files = malloc(*p_quantity * sizeof(char*));
            int file_idx = 0;

            while ((p_next_file = readdir(p_user_dir)) != NULL )
            {
                file_name = p_next_file->d_name;
                if (file_name[0] != '.') // ignore 'non files'
                {
                    user_files[file_idx] = malloc((strlen(file_name) + 1) * sizeof(char));
                    strcpy(user_files[file_idx], file_name);
                    ++file_idx;
                }
            }

            *p_user_files = user_files;
            
            if (closedir(p_user_dir) != 0)
            {
                perror("ERROR get_user_files_list - could not close dir");
                res = GET_USER_FILES_LIST_ERR_CLOSE_DIR;
            }
        }
        else // no such user
            res = GET_USER_FILES_LIST_ERR_NO_SUCH_USER;

        if (pthread_mutex_unlock(&mutex_storage) != 0)
        {
            res = GET_USER_FILES_LIST_ERR_MUTEX_UNLOCK;
            printf("ERROR get_user_files_list - could not unlock mutex\n");
        }
    }
    else // couldn't lock mutex
    {
        res = GET_USER_FILES_LIST_ERR_MUTEX_LOCK;
        printf("ERROR get_user_files_list - could not lock mutex\n");
    }

	return res;
}



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
// connect user
///////////////////////////////////////////////////////////////////////////////////////////////////

int is_in_connected_users(char* name)
{
    int numOfConnectedUsers = vector_size(connected_users);
    for (int i = 0; i < numOfConnectedUsers; i++)
    {
        if (strcmp(connected_users[i]->username, name) == 0)
            return 1;
    }

    return 0;
}



int add_connected_user(char* name, struct in_addr ip, char* port)
{
    int res = ADD_CONNECTED_USERS_SUCCESS;

    if (pthread_mutex_lock(&mutex_connected_users) != 0)
    {
        printf("ERROR add_connected_user - could not lock mutex\n");
        res = ADD_CONNECTED_USER_MUTEX_LOCK_ERROR;
    }
    else
    {
        if (is_in_connected_users(name))
            res = ADD_CONNECTED_USERS_ALREADY_EXISTS;
        else
        {
            if (vector_size(connected_users) == MAX_NUMBER_OF_CONNECTED_USERS)
                res = ADD_CONNECTED_USER_FULL;
            else
            {
                user* p_user = malloc(sizeof(user));
                strcpy(p_user->username, name);
                memcpy(&p_user->ip, &ip, sizeof(ip));
                strcpy(p_user->port, port);

                vector_add(&connected_users, p_user);

                res = ADD_CONNECTED_USERS_SUCCESS;
            }
        }

        if (pthread_mutex_unlock(&mutex_connected_users) != 0)
        {
            printf("ERROR add_connected_user - could not unlock mutex\n");
            res = ADD_CONNECTED_USER_MUTEX_UNLOCK_ERROR;
        }
    }

    return res;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// is connected
///////////////////////////////////////////////////////////////////////////////////////////////////

int is_connected(char* username, int* p_err)
{
    int res = IS_CONNECTED_SUCCESS;
    int connected = 0;

    if (pthread_mutex_lock(&mutex_connected_users) != 0)
        res = IS_CONNECTED_ERR_LOCK_MUTEX;
    {
        int numOfConnectedUsers = vector_size(connected_users);
        for (int i = 0; i < numOfConnectedUsers; i++)
        {
            if (strcmp(connected_users[i]->username, username) == 0)
            {
                *p_err = res;
                connected = 1;
            }
        }

        if (pthread_mutex_unlock(&mutex_connected_users) != 0)
            res = IS_CONNECTED_ERR_UNLOCK_MUTEX;
    }

    *p_err = res;
    return connected;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// get connected users list
///////////////////////////////////////////////////////////////////////////////////////////////////

int get_connected_users(user*** p_users)
{
    int res = GET_CONNECTED_USERS_LIST_SUCCESS;

    if (pthread_mutex_lock(&mutex_connected_users) != 0)
    {
        printf("ERROR get_connected_users - could not lock mutex\n");
        res = GET_CONNECTED_USERS_LIST_ERR_LOCK_MUTEX;
    }
    else
    {
        *p_users = vector_create();
        int num_of_connected_users = vector_size(connected_users);
        for (int i = 0; i < num_of_connected_users; i++)
        {
            vector_add(p_users, connected_users[i]);
        }

        if (pthread_mutex_unlock(&mutex_connected_users) != 0)
        {
            printf("ERROR get_connected_users = could not unlock mutex\n");
            res = GET_CONNECTED_USERS_LIST_ERR_UNLOCK_MUTEX;
        }
    }
    
    return res;
}



///////////////////////////S////////////////////////////////////////////////////////////////////////
// remove_connected_userS
///////////////////////////////////////////////////////////////////////////////////////////////////
int remove_connected_user(char*name){
   
   int res= REMOVE_CONNECTED_USERS_SUCCESS;
    
    if(pthread_mutex_lock(&mutex_connected_users)!=0)
    {
        printf("ERROR remove_connected:user, could not unlock mutex\n");
        res=REMOVE_CONNECTED_USERS_ERR_LOCK_MUTEX;
    }
    else
    {
        int totalConnectedUsers= vector_size(connected_users);
        /*traverse the vector*/
        for (int i = 0; i < totalConnectedUsers; i++)
        {
            if (strcmp(connected_users[i]->username, name) == 0)
            {
                free(connected_users[i]);
                vector_remove(&connected_users,i);
                    
                /*unlock mutex*/
                if(pthread_mutex_unlock(&mutex_connected_users)!= 0)
                {
                    printf("ERROR remove_connected_users= could not unlock mutex\n");
                    res=REMOVE_CONNECTED_USERS_ERR_UNLOCK_MUTEX;
                }
                break; 
            }
        }
        
    }   
          
    return res;
}