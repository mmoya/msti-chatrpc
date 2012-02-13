#include "config.h"
#include "sqlite.h"
#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void
initdb (sqlite3 *db)
{
	char *zSql =
		"CREATE TABLE users (user TEXT PRIMARY KEY, passwd TEXT); "
		"CREATE TABLE messages (id INTEGER PRIMARY KEY AUTOINCREMENT, \
					recipient TEXT, \
					sender TEXT, \
					message TEXT, \
					timestamp INTEGER)";

	char *zErrMsg;
	int rc = sqlite3_exec(db, zSql, NULL, NULL, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		exit(EXIT_FAILURE);
	}
}

sqlite3 *
getdb ()
{
	char *workdir = getworkdir();
	char *dbfile = pathjoin(workdir, DBFILE);
	free(workdir);

	int notfound = 0;
	int rc;

	struct stat st;
	rc = stat(dbfile, &st);
	if (rc == -1) {
		if (errno == ENOENT) {
			notfound = 1;
		}
		else {
			fprintf(stderr, "Error stating %s: %d\n", dbfile, errno);
			exit(EXIT_FAILURE);
		}
	}

	sqlite3 *db;
	rc = sqlite3_open(dbfile, &db);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Can't open database %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(EXIT_FAILURE);
	}

	if (notfound) {
		fprintf(stderr, "Initializing database.\n");
		initdb(db);
	}

	return db;
}
