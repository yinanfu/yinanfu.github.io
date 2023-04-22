#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <map>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <iostream>
#include <set>
#include <stdlib.h>
#include <stdio.h>

#define SERVER_A_PORT "21278"
#define SERVER_B_PORT "22278"
#define SERVER_M_UDP_PORT "23278"
#define SERVER_M_TCP_PORT "24278"

#define IP_ADDR "127.0.0.1"

#define MAXDATASIZE 2000

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
    // boot up
    char *inputP;
    char input[MAXDATASIZE];
    vector<string>::iterator iter; // to iterate the input names

    fprintf(stderr, "Client is up and running.\n");
    while (1)
    {
        fprintf(stderr, "Please enter the usernames to check schedule availability:\n");

        // get input names
        // while (sizeof(input) == 0)
        // {
            cin.get(input, MAXDATASIZE);
            // cin.get(input, MAXDATASIZE);
        // }

        // test if the input names are correct
        // iter=inputNames.begin();
        // while (iter != inputNames.end())
        // {
        //     std::cout << *iter << endl;
        //     iter++;
        // }

        // use TCP to send the the usernames to Main server
        int sockfd, numbytes;
        struct addrinfo hints, *servinfo, *p;
        int status;
        char st[INET6_ADDRSTRLEN];
        int s;
        char sendMess[2000];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if ((status = getaddrinfo(IP_ADDR, SERVER_M_TCP_PORT, &hints, &servinfo)) != 0)
        {
            fprintf(stderr, "client set up incorrectly: %s\n", gai_strerror(status));
            exit(1);
        }

        if ((s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
        {
            perror("client: socket");
        }
        // build connection with serverM TCP port
        if (connect(s, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
        {
            perror("client: connect");
            close(s);
        }

        // send names char to serverM
        send(s, input, 2000, 0);
        fprintf(stdout, "Client finished sending the usernames to Main Server.");

        // recv error names from serverM
        char buf[MAXDATASIZE];
        char buf2[MAXDATASIZE];
        char buf3[MAXDATASIZE];

        // recv error names from serverM
        while(!recv(s, buf, 2000, 0))
        {}
        if (sizeof(buf) != 0)
        {
            fprintf(stdout, "Client received the reply from Main Server using TCP over port %s\n", IP_ADDR);
            cout << buf << " do not exist." << endl;
        }

        // recv time slot from serverM
        while(!recv(s, buf2, 2000, 0))
        {}
        // recv(s, buf2, 2000, 0);
        // recv vaild name from serverM
        while(!recv(s, buf3, 2000, 0))
        {}
        // recv(s, buf3, 2000, 0);
        fprintf(stdout, "Client received the reply from Main Server using TCP over port %s\n", IP_ADDR);
        fprintf(stdout, "Time intervals %s works for %s.\n", buf2, buf3);
        fprintf(stdout, "-----Start a new request----- \n");
        cin.get();
    }
}
