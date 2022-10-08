seq client:
    gcc seq_client.c -o seq_client;./seq_client
seq server:
    gcc seq_server.c -o seq_server;./seq_server 


par client:
    gcc par_client.c -o par_client -lpthread;./par_client

par server fork:
    gcc par_server_fork.c -o par_server_fork;./par_server_fork

par server pthread:
    gcc par_server_thread.c -o par_server_thread -lpthread;./par_server_thread