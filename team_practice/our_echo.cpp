#include <unistd.h>
#include <arpa/inet.h> //struct sockaddr_in
#include <sys/socket.h>
#include <iostream>
#include <string>
#include <sys/socket.h>

#define	ERROR 1
#define BUFFERSIZE 1024

void error_handling(const char* message)
{
	std::cerr << message << '\n';
    exit(ERROR);
}

int	main(int argc, char const *argv[])
{
	int	serv_sock, clnt_sock;
	struct sockaddr_in	serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	char	buffer[BUFFERSIZE];

						//ipv4	//TCP
	//serv_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); non block으로 생성시.
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling("socket error");

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;						//ipv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);		//endian
	serv_addr.sin_port = htons(atoi(argv[1]));

	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (bind(serv_sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == -1)
	{
		close(serv_sock);
		error_handling("bind error");
	}

	//int listen(int sockfd, int backlog);
	if (listen(serv_sock, 1) == -1)
	{
		close(serv_sock);
		error_handling("listen error");
	}

	//accept
	//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, reinterpret_cast<struct sockaddr *>(&clnt_addr), &clnt_addr_size);
	if (clnt_sock == -1)
			error_handling("accept error");

	// https://man7.org/linux/man-pages/man2/select.2.html
    while (1)
    {
        int len = read(clnt_sock, buffer, BUFFERSIZE);
		if (len == -1)
			error_handling("read error");
        buffer[len] = '\0';
        write(STDOUT_FILENO, buffer, len + 1);
        write(clnt_sock, buffer, len + 1);
		// if( 통신끝나면 )
		// 	break ;
    }
	close(clnt_sock); // while 안에 넣으면 안됨
	close(serv_sock);


	return (0);
}


// https://man7.org/linux/man-pages/man2/socket.2.html