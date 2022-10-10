all: fork thread


fork: par_server_fork.c
	gcc par_server_fork.c -o par_server_fork

thread: par_server_thread.c
	gcc par_server_thread.c -o par_server_thread -lpthread

clean: 
	rm par_server_thread par_server_fork