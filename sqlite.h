#ifndef _SQLITE_H
#define _SQLITE_H

#include <sqlite3.h>

void initdb(sqlite3 *);
sqlite3 *getdb();

#endif /* _SQLITE_H */
