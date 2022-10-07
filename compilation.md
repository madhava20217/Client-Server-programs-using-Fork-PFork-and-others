seq client:
    gcc seq_client.c -o seq_client;./seq_client
seq server:
    gcc seq_server.c -o seq_server;./seq_server 


par client:
    gcc par_client.c -o par_client -lpthread;
