#include <stdio.h>
#include <stdlib.h>	//atoi
#include <string.h>	//memset 등
#include <unistd.h>	//read, write, sockaddr_in(?) 등
#include <arpa/inet.h>	//htonl, htons, INADDR_ANY, sockaddr_in 등
#include <sys/socket.h>	//socklen_t

void error_handling(char *msg);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;

	//sockaddr_in은 소켓 주소의 틀을 형성해주는 구조체로 AF_INET일 경우 사용
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;	 //accept함수에서 사용됨
	socklen_t clnt_addr_size;

	//TCP연결지향형이고 ipv4 도메인을 위한 소켓을 생성 socket
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket error");
	
	//주소를 초기화한 후 IP주소와 포트 지정
	memset(&serv_addr, 0, sizeof(serv_addr));
	//bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;					//타입: ipv4
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//ip주소
	serv_addr.sin_port = htons(atoi(argv[1]));		//port

	//소켓과 서버 주소를 바인딩 bind
	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind error");

	//연결 대기열 5개 생성 listen
	if (listen(serv_sock, 5) == -1)
		error_handling("listen error");
	
	//클라이언트로부터 요청이 오면 연결 수락 accept
	clnt_addr_size = sizeof(clnt_addr);
	clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
	if (clnt_sock == -1)
		error_handling("accept error");

	/*-----데이터 전송-----*/ //read, write
	char msg[] = "Hello this is server!\n";
	write(clnt_sock, msg, sizeof(msg));

	//소켓들 닫기
	close(clnt_sock);
	close(serv_sock);

	return (0);
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}