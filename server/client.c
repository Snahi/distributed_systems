#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // for sockaddr_in
#include <strings.h>    // for bzero
#include <stdio.h>
#include <netdb.h>
#include "read_line.h"
#include <string.h>



int main (int argc, char** argv)
{
    int ssd;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    
    if (argc != 2)
    {
        printf("User: client <server_address>\n");
        return 0;
    }

    // obtain the server socket
    ssd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bzero((char*) &server_addr, sizeof(server_addr));
    hp = gethostbyname(argv[1]);

    // copy addres of the server to the server_addr variable
    memcpy(&server_addr.sin_addr, hp->h_addr, hp->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7777);

    // establish the connection
    connect(ssd, (struct sockaddr*) &server_addr, sizeof(server_addr));
	
    // Start your testing here
    char unr[] = "REGISTER";
    send_msg(ssd, (char*) &unr, strlen(unr) + 1);

    char userr[] = "a";
    send_msg(ssd, (char*) &userr, strlen(userr) + 1);

    recv_msg(ssd, (char*) &unr, 1);
    printf("Result unregister: %d\n", (uint8_t) unr[0]);


    close(ssd);
    
    return 0;
}
