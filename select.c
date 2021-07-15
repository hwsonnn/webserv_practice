#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUFSIZE 30

int main(int argc, char* argv[])
{
	fd_set reads, temps;
	int result, strLen;
	char buf[BUFSIZE];
	struct timeval timeout;

	FD_ZERO(&reads);
	FD_SET(0, &reads);

	/*timeout.tv_sec = 5;
	timeout.tv_usec = 5000;*/

	while(1)
	{
		temps = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		
		result = select(1, &temps, 0, 0, &timeout);

		if( result == -1 )
		{
			puts("select() error");
			break;
		}
		else if( result == 0 )
		{
			puts("timeout!");
			break;
		}
		else
		{
			if(FD_ISSET(0, &temps))
			{
				strLen = read(0, buf, BUFSIZE);
				buf[strLen] = 0;
				printf("message from consol: %s", buf);
			}
		}
	}

	return 0;
}
