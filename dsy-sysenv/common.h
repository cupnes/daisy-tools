#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <err.h>

#define TRUE	1
#define FALSE	0

#define MAX_DIR_LEN		6
#define MAX_FILENAME_LEN	32
#define MAX_PATH_LEN		(MAX_DIR_LEN + MAX_FILENAME_LEN)

#define ERR_FMT		"at %s:%d %s()",	__FILE__, __LINE__, __FUNCTION__
#define ERR_FMT_MSG	"at %s:%d %s(): %s",	__FILE__, __LINE__, __FUNCTION__

#define ASSERT(X)	if (!(X)) err(EXIT_FAILURE, ERR_FMT)
#define ERROR_WITH(X, MSG)	if ((X)) errx(EXIT_FAILURE, ERR_FMT_MSG, MSG)

typedef unsigned char bool_t;

size_t fread_safe(void *ptr, size_t size, FILE *stream);
size_t fwrite_safe(void *ptr, size_t size, FILE *stream);
void close_namelist(struct dirent **namelist, int num_files);
