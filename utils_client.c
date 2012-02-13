#include "config.h"
#include "utils.h"

#include <errno.h>
#include <sys/stat.h>

const char *
readpasswd()
{
	char *workdir = getworkdir();
	char *passwdfile = pathjoin(workdir, PASSWDFILE);
	free(workdir);

	int notfound = 0;
	int rc;

	struct stat st;
	rc = stat(passwdfile, &st);
	if (rc == -1) {
		if (errno == ENOENT) {
			notfound = 1;
		}
		else {
			fprintf(stderr, "Error stating %s: %d\n", passwdfile, errno);
			exit(EXIT_FAILURE);
		}
	}

	if (notfound) {
		fprintf(stderr, "Can't open %s.\n", passwdfile);
		return NULL;
	}

	FILE *fpasswd = fopen(passwdfile, "r");
	char *linep = NULL;
	size_t n = 0;
	getline(&linep, &n, fpasswd);
	char *passwd = strtok(linep, " \n");

	return passwd;
}

