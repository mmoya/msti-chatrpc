#ifndef _UTILS_SERVER_H
#define _UTILS_SERVER_H

#include <sqlite3.h>

const char *randpasswd(int);
const char *search_passwd(sqlite3 *, const char *);
int registeruser(sqlite3 *, const char *, const char *);
int sendmessage(sqlite3 *, const char *, const char *, const char *);
int getmessages(sqlite3 *, char **, const char *, const char *);
int delmessages(sqlite3 *, const char *, const char *);

#endif /* _UTILS_SERVER_H */
