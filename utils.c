#include "config.h"

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

char *
pathjoin(const char* path1, const char* path2)
{
	size_t pathlen = strlen(path1) + strlen(path2) + 1;
	char *path = malloc(pathlen + 1);
	snprintf(path, pathlen + 1, "%s/%s", path1, path2);
	return path;
}

char *
getworkdir() {
	char *home = getenv("HOME");
	if (home == NULL)
		home = getpwuid(getuid())->pw_dir;

	char *workdir = pathjoin(home, WORKDIR);

	struct stat st;
	if (stat(workdir, &st) != 0) {
		fprintf(stdout, "Creating directory %s\n", workdir);
		if (mkdir(workdir, 0700) != 0) {
			fprintf(stderr, "Error creating directory %s\n", workdir);
			exit(EXIT_FAILURE);
		}
	}
	else {
		if (!S_ISDIR(st.st_mode)) {
			fprintf(stderr, "%s exists and is not a directory\n", workdir);
			exit(EXIT_FAILURE);
		}
	}

	return workdir;
}
