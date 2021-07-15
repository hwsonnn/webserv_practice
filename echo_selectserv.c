#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFSIZE 100

void errorHandling(char* buf);

int main(int argc, char* argv[])
{
	int servSock, clntSock;
	struct sockaddr_in servAddr, clntAddr;
	struct timeval timeout;
	fd_set reads, cpyReads;
	socklen_t addrSz;
	int fdMax, strLen, fdNum, i;
	char buf[BUFSIZE];

	if( argc != 2 )
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	servSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(argv[1]));

	if( bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1 )
		errorHandling("bind() error");

	if( listen(servSock, 5) == -1 )
		errorHandling("listen() error");

	//fd_set 은 integer의 array
	FD_ZERO(&reads);
	FD_SET(servSock, &reads);
	fdMax = servSock;

	while(1)
	{
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if((fdNum = select(fdMax+1, &cpyReads, 0, 0, &timeout)) == -1 )
			break;

		if( fdNum == 0 )
			continue;

		for( i = 0 ; i < fdMax+1 ; i++ )
		{
			if( FD_ISSET(i, &cpyReads) )
			{
				if( i == servSock )
				{
					addrSz = sizeof(clntAddr);
					clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &addrSz);
					FD_SET(clntSock, &reads);

					if( fdMax < clntSock )
						fdMax = clntSock;

					printf("connected client : %d\n", clntSock);
				}
				else
				{
					strLen = read(i, buf, BUFSIZE);
					// close request

					if( strLen == 0 )
					{
						FD_CLR(i, &reads);
						close(i);
						printf("close client : %d\n", i);
					}
					else
					{
						// echo
						write(i, buf, strLen);
					}
				}
			}
		}
	}

	close(servSock);

	return 0;
}
