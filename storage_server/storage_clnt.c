/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "storage.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

enum clnt_stat 
setup_1(int *clnt_res, CLIENT *clnt)
{
	 return (clnt_call (clnt, setup, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));

}

enum clnt_stat 
shutdown_1(int *clnt_res, CLIENT *clnt)
{
	 return (clnt_call (clnt, shutdown, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));

}

enum clnt_stat 
add_user_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, add_user,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
delete_user_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, delete_user,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
add_connected_user_1(char *username, char *in_addr, char *port, int *clnt_res,  CLIENT *clnt)
{
	add_connected_user_1_argument arg;
	arg.username = username;
	arg.in_addr = in_addr;
	arg.port = port;
	return (clnt_call (clnt, add_connected_user, (xdrproc_t) xdr_add_connected_user_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
delete_connected_user_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, delete_connected_user,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
get_connected_users_1(users_vector *clnt_res, CLIENT *clnt)
{
	 return (clnt_call (clnt, get_connected_users, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_users_vector, (caddr_t) clnt_res,
		TIMEOUT));

}

enum clnt_stat 
add_file_1(char *username, char *file_name, char *description, int *clnt_res,  CLIENT *clnt)
{
	add_file_1_argument arg;
	arg.username = username;
	arg.file_name = file_name;
	arg.description = description;
	return (clnt_call (clnt, add_file, (xdrproc_t) xdr_add_file_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
delete_file_1(char *username, char *file_name, int *clnt_res,  CLIENT *clnt)
{
	delete_file_1_argument arg;
	arg.username = username;
	arg.file_name = file_name;
	return (clnt_call (clnt, delete_file, (xdrproc_t) xdr_delete_file_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
get_files_1(char *username, get_files_res *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, get_files,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_get_files_res, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
is_registered_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, is_registered,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}

enum clnt_stat 
is_connected_1(char *username, int *clnt_res,  CLIENT *clnt)
{
	return (clnt_call(clnt, is_connected,
		(xdrproc_t) xdr_wrapstring, (caddr_t) &username,
		(xdrproc_t) xdr_int, (caddr_t) clnt_res,
		TIMEOUT));
}
