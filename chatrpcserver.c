#include "config.h"
#include "chatrpc.h"
#include "sqlite.h"
#include "utils_server.h"
#include "utils.h"

#include <string.h>

char **
register_1_svc(char *user, struct svc_req *rqstp)
{
	static const char *passwd;

	sqlite3 *db = getdb();
	passwd = search_passwd(db, user);
	if (passwd == NULL) {
		passwd = randpasswd(16);
		if (registeruser(db, user, passwd) != 0)
			passwd = "";
	}
	else {
		passwd = "";
	}
	sqlite3_close(db);
	return &passwd;
}

int *
sendmsg_1_svc(char *recipient, char *sender, char *message, struct svc_req *rqstp)
{
	static int result;
	sqlite3 *db = getdb();
	result = sendmessage(db, recipient, sender, message);
	sqlite3_close(db);
	return &result;
}

char **
recvmsg_1_svc(char *user, char *passwd, struct svc_req *rqstp)
{
	static char *result = NULL;
	sqlite3 *db = getdb();
	getmessages(db, &result, user, passwd);
	sqlite3_close(db);
	return &result;
}

int *
delmsg_1_svc(char *user, char *passwd, struct svc_req *rqstp)
{
	static int result;
	sqlite3 *db = getdb();
	result = delmessages(db, user, passwd);
	sqlite3_close(db);
	return &result;
}
