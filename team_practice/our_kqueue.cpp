#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <vector>

#define	ERROR 1
#define BUFFERSIZE 1024
#define EVENTSIZE 8

void exit_with_perror(const std::string message)
{
	perror(message.c_str());
	exit(ERROR);
}

int main(int argc, char const *argv[])
{
	int					server_socket;
	struct sockaddr_in	server_addr;

	if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		exit_with_perror("socket error\n" + std::string(strerror(errno)));
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		exit_with_perror("bind error\n");

	if (listen(server_socket, 5) == -1)
		exit_with_perror("listen error\n");

	fcntl(server_socket, F_SETFL, O_NONBLOCK);  // ??

	int	kq = kqueue();
	if (kq == -1)
		exit_with_perror("kqueue error\n");

	std::map<int, std::string>	client_data;
	std::vector<struct kevent>	change_list;
	struct kevent				event_list[EVENTSIZE];
	struct kevent				temp_event;


// 	struct kevent {
// 			uintptr_t       ident;  /* identifier for this event */
// 			int16_t         filter; /* filter for event */
// 			uint16_t        flags;  /* general flags */
// 			uint32_t        fflags; /* filter-specific flags */
// 			intptr_t        data;   /* filter-specific data */
// 			void            *udata; /* opaque user data identifier */
//	};

	EV_SET(&temp_event, server_socket, EVFILT_READ, EV_ADD, 0, 0, NULL); //kqueue에 kevent 등록
	change_list.push_back(temp_event);
	std::cout << "echo server started" << std::endl;

	int				num_events;
	struct kevent	*curr_event;

	while (true)
	{
		num_events = kevent(kq, &change_list[0], change_list.size(), event_list, EVENTSIZE, NULL);
							//vec는 컨테이너, 포인터를 넣어야해서 타입을 맞춰주기 위함!
		if (num_events == -1)
			exit_with_perror("kevent error");

		change_list.clear();

		for (int i = 0; i < num_events; ++i)
		{
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				if (curr_event->ident == server_socket)
				    exit_with_perror("server socket error");
				else
				{
				    std::cerr << "client socket error" << std::endl;
				    close(curr_event->ident);
				    client_data.erase(curr_event->ident);
				}
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (curr_event->ident == server_socket)
				{
					int client_socket;
					// accept(listen_fd,(struct sockaddr *)&client_addr, &addrlen);
					client_socket = accept(server_socket, NULL, NULL); // client address가 필요 없어서 NULL!
					if (client_socket == -1)
						exit_with_perror("accept() error");
					std::cout << "accept new client: " << client_socket << std::endl;
					fcntl(client_socket, F_SETFL, O_NONBLOCK);

					EV_SET(&temp_event, client_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
					change_list.push_back(temp_event);
					EV_SET(&temp_event, client_socket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
					change_list.push_back(temp_event);

					client_data[client_socket] = "";
				}
				else if (client_data.find(curr_event->ident) != client_data.end())
				{
					char	buffer[1024];
					int		n;

					n = read(curr_event->ident, buffer, sizeof(buffer) - 1);
					if (n == -1)
					{
						std::cerr << "read error" << std::endl;
						continue;
					}
					else if (n == 0)
					{
						std::cout << "client disconnected" << std::endl;
						client_data.erase(curr_event->ident);
						close(curr_event->ident);
					}
					else
					{
						buffer[n] = '\0';
						client_data[curr_event->ident] += buffer;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				if (client_data[curr_event->ident].length() > 0)
				{
					int	n;
					if((n = write(curr_event->ident, client_data[curr_event->ident].c_str(), client_data[curr_event->ident].length())) == -1)
						std::cerr << "write error" << '\n';
					client_data[curr_event->ident].clear();
				}
			}
		}
	}

	return (0);
}
