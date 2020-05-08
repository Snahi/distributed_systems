const MAX_NUM_OF_FILES = 4096;
const MAX_NUM_OF_USERS = 4096;

struct file {
	char filename[257];
};

typedef file files_list<MAX_NUM_OF_FILES>;

struct user {
	char username[257];
	char ip[16];
	char port[6];
};

typedef user users_list<MAX_NUM_OF_USERS>;

program STORAGE {
	version STORAGE_VER {
		int setup() = 1;
		int shutdown() = 2;
		int add_user(string username) = 3;
		int delete_user(string username) = 4;
		int add_connected_user(string username, string in_addr, string port) = 5;
		int delete_connected_user(string username) = 6;
 		users_list get_connected_users() = 7;
		int add_file(string username, string file_name, string description) = 8;
		int delete_file(string username, string file_name) = 9;
		files_list get_files(string username) = 10;
		int is_registered(string username) = 11;
		int is_connected(string username) = 12;
	} = 1;
} = 0x20000000;
