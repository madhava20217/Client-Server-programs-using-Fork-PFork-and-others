all: select poll epoll

select: select_server.c
	gcc select_server.c -o select_server

poll: poll_server.c
	gcc poll_server.c -o poll_server

epoll: epoll_server.c
	gcc epoll_server.c -o epoll_server

clean:
	rm select_server poll_server epoll_server