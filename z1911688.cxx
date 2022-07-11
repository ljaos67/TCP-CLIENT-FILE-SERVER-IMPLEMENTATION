/* CSCI - 330 - ASSIGNMENT 09
 * NAME: LEONART JAOS
 * Z-ID: z1911688
 * z1911688.cxx
 * 
 * TCP FILE SERVER
 * EXTRA CREDIT--------------------------------------------------------------------------------->
 * 
 * Purpose: This executable connects establishes a connection to TCPClient.cxx for the purpose of
 * a file server implentation. This file server establishes its port connection based on the
 * input command from execution in addition to the root of which the client will search files
 * and directories. From the execution of the client, an argument is read from the client and
 * processed as a forked command. 
 * 	      
 * 	command line arguments:
 * 		argv[1] Port number to listen on
 * 		argv[2] Pathname to a directory 
 * 
 *  
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <dirent.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
#include <ctime>
#include <iostream>
using namespace std;

// This function cleans input from the client side
string cleanInput(const char * a)
{
	string b = "";
	bool check = false;
	for(int i = 0; i < strlen(a); i++)
	{
		if(a[i] == '"')
		{
			continue;
		}
		else if((a[i] == 'G')||(a[i] == 'E')||(a[i] == 'T')||(a[i] == ' '))
		{
			continue;
		}
		else
		{
			b = b + a[i]; 
		}
	}
	// removes commands that are invalid
	if(b[0] != '/')
	{
		string c = "error";
		return c;
	}
	else if(b == "/")
	{
		return b;
	}
	// returns string that contains only the command
	else
	{	
		string c = "";
		for(int i = 0; i < b.size(); i++)
		{
			if(i == 0)
				continue;
			else
			{
				c = c + b[i];
			}
		}
		return c;
	}
}
// this function checks if a file is an HTML file
bool isHTML(const char * arr) 
{
	string comp = "";
	bool start = false;
	if(strlen(arr) < 6)
	{
		return false;
	}
	for(int i = 0; i < strlen(arr); i++)
	{
		if(arr[i] == '.')
		{
			start = true;
			continue;
		}
		if(start)
		{
			comp = comp + arr[i];
		}
	}
	if(comp == "html")
		return true;
	else
		return false;
}
// this function processes requests from the client-side
void processClientRequest(int connSock)
{
	char buffer[1024], p[1024];
	int rs, received;
	// read command
	if((received = read(connSock, p, sizeof(p))) < 0)
	{
		perror("receive");
		exit(EXIT_FAILURE);
	}
	// error output for invalid command
	if(cleanInput(p) == "error")
	{
		strcpy(buffer, "error: ");
		strcat(buffer, p);
		strcat(buffer, " not found");
		write(connSock, buffer, strlen(buffer));
		exit(EXIT_FAILURE);
	}
	const char * path = cleanInput(p).c_str();
	cout << "Client request: " << path << endl;
	struct stat s;
	rs = stat(path, &s);
	if(rs == -1)
	{
		strcpy(buffer, "Error: ");
		strcat(buffer, path);
		strcat(buffer, " not found");
		exit(EXIT_FAILURE);
	}
	// checks if input from client is a file
	if(S_ISREG(s.st_mode))
	{
		// file
                int count, fd;
                fd = open(path, O_RDONLY);
        	if(fd == -1)
        	{
                	strcpy(buffer, "Error: ");
               		strcat(buffer, path);
              		strcat(buffer, " not found");
              		exit(EXIT_FAILURE);
        	}
		// reads from file and writes to client output
		while((count = (read(fd, buffer, 1024)) != 0))
		{
			write(connSock, buffer, strlen(buffer));
                }
		// error, outputs time to client side and date and exits
		if(fd == -1)
		{
	                // tell client that an error occurred
                	strcpy(buffer, path);
                	strcat(buffer, ": could not execute command\n");
                	if (write(connSock, buffer, strlen(buffer)) < 0) {
                        	perror("write");
                        	exit(EXIT_FAILURE);
                	}
			time_t rt;
			struct tm * timeinfo;
			char buf[80];
			time(&rt);
			timeinfo = localtime(&rt);
			strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
			strcat(buf, "->DATE AND TIME OF COMMAND");
			write(connSock, buf, strlen(buf));
                	exit(EXIT_SUCCESS);
		}
		// concludes client-side request and outputs date, time, and writes to clientside
		cout << "done with client request\n";
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
		close(connSock);
		exit(EXIT_FAILURE);
	}
	// checks if path is a directory
	else if(S_ISDIR(s.st_mode))
	{
		// directory
		// open directory
		bool check = false;
		DIR *dirp = opendir(path);
		struct dirent *dirEntry;
		if (dirp == 0) 
		{
			// tell client that an error occurred
			strcpy(buffer, path);
			strcat(buffer, ": could not open directory\n");
			if (write(connSock, buffer, strlen(buffer)) < 0) {
				perror("write");
		 		exit(EXIT_FAILURE);
			}
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
			exit(EXIT_SUCCESS);
		}
		// parse through directory for html file
		while((dirEntry = readdir(dirp)) != NULL)
		{
			if(isHTML(dirEntry->d_name))
			{	
				chdir(path);
				int count, fd;
				fd = open(dirEntry->d_name, O_RDONLY);
				while((count = (read(fd, buffer, 1024)) != 0))
				{
					write(connSock, buffer, strlen(buffer));
				}
				check = true;
				const char * parent = "..";
				chdir(parent);
			}
		}
		closedir(dirp);
		// writes date and time and closes connection
		if(check)
		{
			cout << "done with client request\n";
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
			close(connSock);
			exit(EXIT_SUCCESS);
		}
		// case if there isn;t an html file in directory
		DIR *dirp2 = opendir(path);
                struct dirent *dirEntry2;
                if (dirp2 == 0)
                {
                        // tell client that an error occurred
                        strcpy(buffer, path);
                        strcat(buffer, ": could not open directory\n");
                        if (write(connSock, buffer, strlen(buffer)) < 0) {
                                perror("write");
                                exit(EXIT_FAILURE);
                        }
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
                        exit(EXIT_SUCCESS);
                }
		// reads out contents of directory
		while((dirEntry2 = readdir(dirp2)) != NULL) 
		{
		strcpy(buffer, dirEntry2->d_name);
		strcat(buffer, "\n");
		if (write(connSock, buffer, strlen(buffer)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
		cout << "sent: " << buffer;
		}
		closedir(dirp2);
		cout << "done with client request\n";
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
		close(connSock);
		exit(EXIT_SUCCESS);

	}
	else
	{
		// tell client that an error occurred
		strcpy(buffer, path);
		strcat(buffer, ": Not a file nor directory\n");
		if (write(connSock, buffer, strlen(buffer)) < 0) {
			perror("write");
		 	exit(EXIT_FAILURE);
		}
                        time_t rt;
                        struct tm * timeinfo;
                        char buf[80];
                        time(&rt);
                        timeinfo = localtime(&rt);
                        strftime(buf,sizeof(buf),"\n%m-%d-%Y %H:%M:%S",timeinfo);
                        strcat(buf, "->DATE AND TIME OF COMMAND");
                        write(connSock, buf, strlen(buf));
		exit(EXIT_SUCCESS);
	}


	
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "USAGE: PORTNAME PATHNAME\n";
		exit(EXIT_FAILURE);
	}
	// create the TCP socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	// create address structures
	struct sockaddr_in server_address;	// struct for address of server
	struct sockaddr_in client_address;	// struct for address of client
	unsigned int addrlen = sizeof(client_address);

	// construct the server sockaddr_in structure
	memset(&server_address, 0, sizeof(server_address));	// clear struct
	server_address.sin_family = AF_INET;	// Internet IP
	server_address.sin_addr.s_addr = INADDR_ANY;	// any IP address
	server_address.sin_port = htons(atoi(argv[1]));		// server port

	// Bind the socket
	if(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// listen: make socket passive and set length of queue
	if(listen(sock, 64) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	cout << "z1911688 listening on port: " << argv[1] << endl;
	// changes root directory to server base from input
	chdir(argv[2]);
	// Run until cancelled
	while(true)
	{
		int connSock = accept(sock, (struct sockaddr *) &client_address, &addrlen);
		if(connSock < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// fork
		// parent process
		if(fork())
		{
			close(connSock);
		}
		// child process
		else
		{
			processClientRequest(connSock);
		}

	}
	close(sock);
	return 0;

}
