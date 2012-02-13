CFLAGS=-Wall -ggdb

rpciface = chatrpc
server = $(rpciface)server
client = $(rpciface)client

all: $(server) $(client)

$(rpciface)_xdr.c: $(rpciface).x config.h
	rpcgen -N $<

utils_server.o: utils_server.c
	$(CC) $(CFLAGS) -lsqlite3 -c $^

utils_client.o: utils_client.c
	$(CC) $(CFLAGS) -c $^

$(client): $(rpciface)_xdr.c $(rpciface)_clnt.c $(client).c utils.c utils_client.o
	$(CC) $(CFLAGS) -o $@ $^

$(server): $(rpciface)_xdr.c $(rpciface)_svc.c $(server).c utils.c utils_server.o sqlite.c
	$(CC) $(CFLAGS) -lsqlite3 -o $@ $^

client: $(client)
server: $(server)

clean:
	rm -f $(rpciface).h $(rpciface)_clnt.c $(rpciface)_svc.c $(rpciface)_xdr.c
	rm -f *.o $(client) $(server)

.PHONY: all clean client server
