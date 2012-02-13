#include "config.h"
#include "chatrpc.h"
#include "utils.h"
#include "utils_client.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

CLIENT *
create_client(const char *host) {
	CLIENT *clnt;
	clnt = clnt_create(host, CHATRPC, VERSION, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror(host);
		exit(1);
	}
	return clnt;
}

char *
chat_register_rpc(const char *hostname, const char *username)
{
	CLIENT *clnt = create_client(hostname);
	char **result;

	result = register_1((char *)username, clnt);
	if (result == (char **)NULL) {
		clnt_perror(clnt, "register rpc failed");
		exit(EXIT_FAILURE);
	}

	return *result;
}

int
chat_register(const char *hostname, const char *username)
{
	char *passwd = chat_register_rpc(hostname, username);
	if (strlen(passwd) == 0) {
		fprintf(stderr, "Register failed\n");
		exit(EXIT_SUCCESS);
	}

	char *workdir = getworkdir();
	char *passwdfile = pathjoin(workdir, PASSWDFILE);
	free(workdir);

	FILE *fpasswd = fopen(passwdfile, "w");
	fprintf(fpasswd, "%s\n", passwd);
	fclose(fpasswd);
	chmod(passwdfile, 0600);
	fprintf(stderr, "%s\n", passwd);

	return 0;
}

int
chat_sendmsg(const char *hostname,
	     const char *recipient,
	     const char *sender,
	     const char *message)
{
	CLIENT *clnt = create_client(hostname);
	int *result;

	result = sendmsg_1((char *)recipient, (char *)sender, (char *)message, clnt);
	if (result == (int *)NULL) {
		clnt_perror(clnt, "sendmsg rpc failed");
		exit(EXIT_FAILURE);
	}

	return *result;
}

const char *
chat_recvmsg(const char *hostname,
	     const char *user,
	     const char *passwd)
{
	CLIENT *clnt = create_client(hostname);
	char **result;

	result = recvmsg_1((char *)user, (char *)passwd, clnt);
	if (result == (char **)NULL) {
		clnt_perror(clnt, "recvmsg rpc failed");
		exit(EXIT_FAILURE);
	}

	return *result;
}

int
chat_delmsg(const char *hostname,
	    const char *user,
	    const char *passwd)
{
	CLIENT *clnt = create_client(hostname);
	int *result;

	result = delmsg_1((char *)user, (char *)passwd, clnt);
	if (result == (int *)NULL) {
		clnt_perror(clnt, "delmsg rpc failed");
		exit(EXIT_FAILURE);
	}

	return *result;
}

int
main(int argc, char *argv[])
{
	const char *host_default = "127.0.0.1";

	static int op = 0;
	char *host = host_default;
	char *user = NULL;

	static struct option long_options[] = {
		{"host",     required_argument, 0, 'h'},
		{"user",     required_argument, 0, 'u'},
		{"register", no_argument,     &op,   1},
		{"send",     no_argument,     &op,   2},
		{"receive",  no_argument,     &op,   3},
		{"delete",   no_argument,     &op,   4},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	int c;

	while (1) {
		c = getopt_long(argc, argv, "hu", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 0:
				break;
			case 'h':
				host = optarg;
				break;
			case 'u':
				user = optarg;
				break;
			default:
				abort();
		}
	}

	if (user == NULL)
		user = getpwuid(getuid())->pw_name;

	int rc;
	const char *passwd;

	if (op > 0)
		printf("Working as %s@%s.\n", user, host);

	switch (op) {
		case 1:
			// register
			chat_register(host, user);
			break;
		case 2:
			// send
			if (argc - optind < 2) {
				printf("Please specify recipient and message.\n");
				exit(EXIT_FAILURE);
			}
			char *recipient = argv[optind];
			char *message = argv[optind+1];
			rc = chat_sendmsg(host, recipient, user, message);
			printf("Send %s.\n", rc == 0 ? "OK" : "ERROR");
			break;
		case 3:
			// receive
			passwd = readpasswd();
			if (passwd == NULL) {
				printf("Can't find passwd file. Please register first.\n");
				exit(EXIT_FAILURE);
			}
			const char *result = chat_recvmsg(host, user, passwd);
			if (result == NULL || strlen(result) == 0)
				printf("No messages for %s.\n", user);
			else
				printf("%s", result);
			break;
		case 4:
			// delete
			passwd = readpasswd();
			if (passwd == NULL) {
				printf("Can't find passwd file. Please register first.\n");
				exit(EXIT_FAILURE);
			}
			rc = chat_delmsg(host, user, passwd);
			printf("Delete %s.\n", rc == 0 ? "OK" : "ERROR");
			break;

	}
	exit(EXIT_SUCCESS);
}
