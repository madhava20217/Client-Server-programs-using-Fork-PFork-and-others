all: clients servers

clients:
	make -C Client_Programs/Parallel\ client
	make -C Client_Programs/Sequential\ client

servers:
	make -C Server_Programs/Concurrent\ server
	make -C Server_Programs/Non-blocking\ server
	make -C Server_Programs/Sequential\ server

clean:
	make clean -C Client_Programs/Parallel\ client
	make clean -C Client_Programs/Sequential\ client
	make clean -C Server_Programs/Concurrent\ server
	make clean -C Server_Programs/Non-blocking\ server
	make clean -C Server_Programs/Sequential\ server