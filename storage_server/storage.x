struct file {
	string name<>;
};

typedef file files_vector<>;

struct user {
	string username<>;
	string ip<>;
	string port<>;
};

typedef user users_vector<>;

program STORAGE {
	version STORAGE_VER {
		int setup() = 1;
		int shutdown() = 2;
		int add_user(string username) = 3;
		int delete_user(string username) = 4;
		int add_connected_user(string username, string in_addr, string port) = 5;
		int delete_connected_user(string username) = 6;
 		users_vector get_connected_users() = 7;
		int add_file(string username, string file_name, string description) = 8;
		int delete_file(string username, string file_name) = 9;
		files_vector get_files(string username) = 10;
	} = 1;
} = 0x20000000;
