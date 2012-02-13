#include "utils_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *
randpasswd(int size) {
	char *passwd = malloc(sizeof(char) * (size + 1));

	char *randchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	int randcharslen = strlen(randchars);

	int i;
	for (i=0; i<size; i++) {
		passwd[i] = randchars[rand() % randcharslen];
	}
	passwd[size] = '\0';

	return passwd;
}

const char *
search_passwd(sqlite3 *db, const char *user)
{
	const char *zSql = "SELECT passwd FROM users WHERE user = ?";
	sqlite3_stmt *stmt = NULL;
	int rc;
	const char *passwd = NULL;
	char *buff;
	int size;

	sqlite3_prepare_v2(db, zSql, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	switch (rc) {
		// user not found
		case SQLITE_DONE:
			fprintf(stderr, "User %s not found.\n", user);
			break;
		// user found
		case SQLITE_ROW:
			buff = sqlite3_column_text(stmt, 0);
			size = sqlite3_column_bytes(stmt, 0) + 1;
			passwd = malloc(size);
			strncpy((char *)passwd, buff, size);
			fprintf(stderr, "Passwd %s found for user %s.\n", passwd, user);
			break;
		default:
			fprintf(stderr, "Error looking passwd for user %s.\n", user);
	}
	sqlite3_finalize(stmt);

	return passwd;
}

int
registeruser(sqlite3 *db, const char *user, const char *passwd)
{
	const char *zSql = "INSERT INTO users (user, passwd) VALUES (?, ?)";
	sqlite3_stmt *stmt = NULL;
	int rc;

	sqlite3_prepare_v2(db, zSql, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, passwd, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		// user not found
		fprintf(stderr, "Can't register user %s.\n", user);
		return 1;
	}
	sqlite3_finalize(stmt);
	return 0;
}

int
sendmessage(sqlite3 *db, const char *recipient, const char *sender, const char *message)
{
	const char *zSql = "INSERT INTO messages (recipient, sender, message, timestamp) VALUES (?, ?, ?, strftime('%s', 'now'))";
	sqlite3_stmt *stmt = NULL;
	int rc;

	sqlite3_prepare_v2(db, zSql, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, recipient, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, sender, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, message, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Error adding message <%s> for user %s.\n", message, recipient);
		return 1;
	}
	sqlite3_finalize(stmt);
	return 0;
}

int
authenticate(sqlite3 *db, const char *user, const char *passwd)
{
	const char *stored_passwd = search_passwd(db, user);
	int result = 0;

	fprintf(stderr, "passwords: stored=<%s>, sent=<%s>.\n", stored_passwd, passwd);

	if (stored_passwd == NULL) {
		fprintf(stderr, "Can't get passwd for user %s.\n", user);
		result = 1;
	}

	if (strcmp(stored_passwd, passwd) != 0 ) {
		fprintf(stderr, "Passwd don't match for user %s.\n.", user);
		result = 2;
	}

	free((void *)stored_passwd);
	return result;
}

int
getmessages(sqlite3 *db, char **result, const char *user, const char *passwd)
{
	static int resultsize;
	if (*result == NULL) {
		resultsize = 512;
		*result = malloc(sizeof(char) * (resultsize + 1));
	}

	int resultlength = 0;
	(*result)[resultlength] = '\0';

	int authrc = authenticate(db, user, passwd);
	if (authrc > 0) {
		fprintf(stderr, "Can't authenticate, result = %d.\n", authrc);
		return authrc;
	}

	const char *zSql = "SELECT sender, message FROM messages WHERE recipient = ?";
	sqlite3_stmt *stmt = NULL;
	int rc;

	sqlite3_prepare_v2(db, zSql, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);

	char *sender, *message, *line;

	if (rc == SQLITE_ROW) {
		int linelength = 0;
		while (rc == SQLITE_ROW) {
			sender  = (char *)sqlite3_column_text(stmt, 0);
			message = (char *)sqlite3_column_text(stmt, 1);

			linelength = strlen(sender) + strlen(message) + 4;
			line = malloc(sizeof(char) * linelength);
			sprintf(line, "%s: %s\n", sender, message);

			while (linelength > resultsize - resultlength) {
				char *newbuff = realloc(*result, resultsize + 512 + 1);
				if (newbuff == NULL) {
					fprintf(stderr, "Error reallocating result.\n");
					exit(EXIT_FAILURE);
				}
				resultsize += 512;
				*result = newbuff;
			}

			strcat(*result, line);
			resultlength += linelength;
			free(line);

			rc = sqlite3_step(stmt);
		}
	}

	sqlite3_finalize(stmt);
	(*result)[resultlength] = '\0';
	return resultlength;
}

int
delmessages(sqlite3 *db, const char *user, const char *passwd)
{
	int authrc = authenticate(db, user, passwd);
	if (authrc > 0) {
		fprintf(stderr, "Can't authenticate, result = %d.\n", authrc);
		return authrc;
	}

	const char *zSql = "DELETE FROM messages WHERE recipient = ?";
	sqlite3_stmt *stmt = NULL;
	int rc;

	sqlite3_prepare_v2(db, zSql, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, user, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE) {
		fprintf(stderr, "Error deleting messages for user %s.\n", user);
		return 1;
	}
	sqlite3_finalize(stmt);
	return 0;
}
