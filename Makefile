all:directory_server file_server1 file_server2 file_server3 client1 client2
directory_server:directory_server.c
	gcc -o directory_server directory_server.c -lsocket -lnsl -lresolv
file_server1:file_server1.c
	gcc -o file_server1 file_server1.c -lsocket -lnsl -lresolv
file_server2:file_server2.c
	gcc -o file_server2 file_server2.c -lsocket -lnsl -lresolv
file_server3:file_server3.c
	gcc -o file_server3 file_server3.c -lsocket -lnsl -lresolv
client1:client1.c
	gcc -o client1 client1.c -lsocket -lnsl -lresolv
client2:client2.c
	gcc -o client2 client2.c -lsocket -lnsl -lresolv
