#include <iostream>
#include <unistd.h>
#include <arpa/inet.h> //struct sockaddr_in
#include <sys/socket.h>

#define ERROR 1
#define BUFFERSIZE 1024

void error_handling(const char* message)
{
	std::cerr << message << '\n';
    exit(ERROR);
}

int main(int argc, char const *argv[])
{
	int	clnt_sock;
	struct sockaddr_in	serv_addr;

	clnt_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1)
        error_handling("socket error");

    //인자로 받은 서버 주소 정보 저장
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(atoi(argv[1]));

	// int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (connect(clnt_sock, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) == -1)
	{
		close(clnt_sock);
		error_handling("connect error");
	}

	char buffer[BUFFERSIZE];
	int len, ret;
	while(1)
	{
		std::cin >> buffer;
		// fgets(buffer, BUFFERSIZE, stdin);
        buffer[strlen(buffer)] = '\0';
		ret = write(clnt_sock, buffer, strlen(buffer) + 1);
		if (ret == -1)
		{
			close(clnt_sock);
			error_handling("write error");
		}
		len = read(clnt_sock, buffer, BUFFERSIZE);
		if (len == -1)
		{
			close(clnt_sock);
			error_handling("read error");
		}
		buffer[len] = '\0';

		std::cout << "clnt_sock: " << clnt_sock << '\n';
		std::cout << "server: " << buffer;
	}
	std::cout << "closing client";
	close(clnt_sock);
	return (0);
}
