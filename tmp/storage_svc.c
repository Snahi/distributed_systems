/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

int
_setup_1 (void  *argp, void *result, struct svc_req *rqstp)
{
	return (setup_1_svc(result, rqstp));
}

int
_shutdown_1 (void  *argp, void *result, struct svc_req *rqstp)
{
	return (shutdown_1_svc(result, rqstp));
}

int
_add_user_1 (char * *argp, void *result, struct svc_req *rqstp)
{
	return (add_user_1_svc(*argp, result, rqstp));
}

int
_delete_user_1 (char * *argp, void *result, struct svc_req *rqstp)
{
	return (delete_user_1_svc(*argp, result, rqstp));
}

int
_add_connected_user_1 (add_connected_user_1_argument *argp, void *result, struct svc_req *rqstp)
{
	return (add_connected_user_1_svc(argp->username, argp->in_addr, argp->port, result, rqstp));
}

int
_delete_connected_user_1 (char * *argp, void *result, struct svc_req *rqstp)
{
	return (delete_connected_user_1_svc(*argp, result, rqstp));
}

int
_get_connected_users_1 (void  *argp, void *result, struct svc_req *rqstp)
{
	return (get_connected_users_1_svc(result, rqstp));
}

int
_add_file_1 (add_file_1_argument *argp, void *result, struct svc_req *rqstp)
{
	return (add_file_1_svc(argp->username, argp->file_name, argp->description, result, rqstp));
}

int
_delete_file_1 (delete_file_1_argument *argp, void *result, struct svc_req *rqstp)
{
	return (delete_file_1_svc(argp->username, argp->file_name, result, rqstp));
}

int
_get_files_1 (get_files_1_argument *argp, void *result, struct svc_req *rqstp)
{
	return (get_files_1_svc(argp->username, argp->p_err, result, rqstp));
}

static void
storage_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *add_user_1_arg;
		char *delete_user_1_arg;
		add_connected_user_1_argument add_connected_user_1_arg;
		char *delete_connected_user_1_arg;
		add_file_1_argument add_file_1_arg;
		delete_file_1_argument delete_file_1_arg;
		get_files_1_argument get_files_1_arg;
	} argument;
	union {
		int setup_1_res;
		int shutdown_1_res;
		int add_user_1_res;
		int delete_user_1_res;
		int add_connected_user_1_res;
		int delete_connected_user_1_res;
		users_vector get_connected_users_1_res;
		int add_file_1_res;
		int delete_file_1_res;
		files_vector get_files_1_res;
	} result;
	bool_t retval;
	xdrproc_t _xdr_argument, _xdr_result;
	bool_t (*local)(char *, void *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case setup:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_setup_1;
		break;

	case shutdown:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_shutdown_1;
		break;

	case add_user:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_add_user_1;
		break;

	case delete_user:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_delete_user_1;
		break;

	case add_connected_user:
		_xdr_argument = (xdrproc_t) xdr_add_connected_user_1_argument;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_add_connected_user_1;
		break;

	case delete_connected_user:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_delete_connected_user_1;
		break;

	case get_connected_users:
		_xdr_argument = (xdrproc_t) xdr_void;
		_xdr_result = (xdrproc_t) xdr_users_vector;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_get_connected_users_1;
		break;

	case add_file:
		_xdr_argument = (xdrproc_t) xdr_add_file_1_argument;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_add_file_1;
		break;

	case delete_file:
		_xdr_argument = (xdrproc_t) xdr_delete_file_1_argument;
		_xdr_result = (xdrproc_t) xdr_int;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_delete_file_1;
		break;

	case get_files:
		_xdr_argument = (xdrproc_t) xdr_get_files_1_argument;
		_xdr_result = (xdrproc_t) xdr_files_vector;
		local = (bool_t (*) (char *, void *,  struct svc_req *))_get_files_1;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	retval = (bool_t) (*local)((char *)&argument, (void *)&result, rqstp);
	if (retval > 0 && !svc_sendreply(transp, (xdrproc_t) _xdr_result, (char *)&result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	if (!storage_1_freeresult (transp, _xdr_result, (caddr_t) &result))
		fprintf (stderr, "%s", "unable to free results");

	return;
}

int
main (int argc, char **argv)
{
	register SVCXPRT *transp;

	pmap_unset (STORAGE, STORAGE_VER);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, STORAGE, STORAGE_VER, storage_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (STORAGE, STORAGE_VER, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, STORAGE, STORAGE_VER, storage_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (STORAGE, STORAGE_VER, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);
	/* NOTREACHED */
}
