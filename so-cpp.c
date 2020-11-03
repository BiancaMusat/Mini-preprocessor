#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "hashtable.h"

#define MAX_LINE_LENGTH 256
#define SEPARATORS "\n\"\t[]{}<>=+- */\\%!&|^.,:;()"

/* read a line from stdin or input file */
int readLine(char **line, FILE *fp)
{
	int still_reading = 1;
	char *buf;
	char *buf_ptr;
	size_t len_used;
	int strip_wspace = 0;

	buf = malloc(sizeof(char) * MAX_LINE_LENGTH);
	if (buf == NULL)
		return 12;

	/* Empty string */
	(*line)[0] = '\0';

	while (still_reading == 1) {
		if (fgets(buf, MAX_LINE_LENGTH * sizeof(*buf), fp) == NULL) {
			still_reading = 0;
			break;
		}

		buf_ptr = buf;
		/* if current line continues on next line ('\'),
		 * remove witespaces from the begining of newline
		 */
		if (strip_wspace == 1) {
			while (*buf_ptr == ' ' || *buf_ptr == '\t')
				buf_ptr++;
		}
		strip_wspace = 0;

		strcat(*line, buf_ptr);
		len_used = strlen(*line);
		/* end reading if '\n' is found and the line doesn`t continue */
		if ((*line)[len_used - 1] == '\n') {
			if (len_used > 1 && (*line)[len_used - 2] == '\\') {
				len_used -= 2;
				(*line)[len_used] = '\0';
				strip_wspace = 1;
			} else {
				free(buf);
				return 0;
			}
		}
	}
	free(buf);
	return -1;
}

/* get path of the input file */
void parsePath(char **infile, char **path)
{
	int j;

	j = strlen(*infile) - 1;
	while (j >= 0) {
		if ((*infile)[j] == '/') {
			strncpy(*path, *infile, j);
			(*path)[j] = '\0';
			break;
		}
		j--;
	}
}

/* parse command line arguments */
int getArgs(struct Elem **array, char **infile,
			char **outfile, char **dir1, char **dir2,
			char **path, int argc, char **argv)
{
	char *token_key;
	char *token_val;
	int i;

	i = 1;
	while (i < argc) {
		if (strlen(argv[i]) == 2) {
			if (strcmp(argv[i], "-D") == 0) {
				token_key = strtok(argv[i + 1], " =");
				token_val = strtok(NULL, " ");
				put(*array, token_key, token_val);
				i = i + 2;
			} else if (strcmp(argv[i], "-I") == 0) {
				if (strcmp(*dir1, "") == 0)
					strcpy(*dir1, argv[i + 1]);
				else
					strcpy(*dir2, argv[i + 1]);
				i = i + 2;
			} else if (strcmp(argv[i], "-o") == 0) {
				strcpy(*outfile, argv[i + 1]);
				i = i + 2;
			} else if (argv[i][0] == '-')
				return 12;
		} else if (strlen(argv[i]) > 2) {
			if (argv[i][0] == '-' && (argv[i][1] == 'D')) {
				argv[i] += 2;
				token_key = strtok(argv[i], " =");
				token_val = strtok(NULL, " ");
				put(*array, token_key, token_val);
				i = i + 1;
			} else if (argv[i][0] == '-' && (argv[i][1] == 'I')) {
				argv[i] += 2;
				if (strcmp(*dir1, "") == 0)
					strcpy(*dir1, argv[i]);
				else
					strcpy(*dir2, argv[i]);
				i = i + 1;
			} else if (argv[i][0] == '-' && (argv[i][1] == 'o')) {
				argv[i] += 2;
				strcpy(*outfile, argv[i]);
				i = i + 1;
			} else {
				if (strcmp(*infile, "") == 0) {
					strcpy(*infile, argv[i]);
					parsePath(infile, path);
				} else if (strcmp(*outfile, "") == 0)
					strcpy(*outfile, argv[i]);
				else
					return 12;
				i = i + 1;
			}
		}
	}
	return 0;
}

/* replace defined variables with their values */
int transformLine(struct Elem *array, char **line)
{
	char *ptr_line = *line;
	char *token = *line;
	int inside_quotes = 0;
	char *value = NULL;
	char *result = malloc(MAX_LINE_LENGTH * sizeof(char));

	if (result == NULL)
		return 12;
	result[0] = '\0';

	for (ptr_line = *line; *ptr_line; ++ptr_line) {
		char current = *ptr_line;
		/* If character is a delimiter */
		if (strchr(SEPARATORS, current)) {
			*ptr_line = '\0';
			/* if variable is inside quotes, do not replace it */
			if (!inside_quotes &&
				get(array, token, &value) == 0 &&
				value != NULL)
				token = value;

			strcat(result, token);
			strncat(result, &current, 1);
			token = ptr_line + 1;

			if (current == '\"')
				inside_quotes = !inside_quotes;
		}
	}

	free(*line);
	*line = result;
	return 0;
}

/* add all defined variables to hashmap */
int parseDefine(struct Elem **array, char *line)
{
	int rc;
	char *key_hash;
	char *value;

	strtok(line, " ");
	key_hash = strtok(NULL, " \t\n");
	value = strtok(NULL, "\n");
	rc = put(*array, key_hash, value);
	if (rc == 12)
		return 12;
	else if (rc == -1) {
		resize(array);
		rc = put(*array, key_hash, value);
		if (rc == 12)
			return 12;
	}
	return 0;
}

/* parse whole file line by line and print output */
int parseFunction(struct Elem **array, FILE *fp_out,
				char **line, FILE *fp_in, int *cond)
{
	int err = 0;
	char *value;
	char *final_val;
	char *token;

	final_val = malloc(sizeof(char) * MAX_LINE_LENGTH);
	if (final_val == NULL)
		return 12;
	if (strncmp(*line, "#undef", 6) != 0 &&
			strncmp(*line, "#ifdef", 6) != 0 &&
			strncmp(*line, "#ifndef", 7) != 0) {
		err = transformLine(*array, line);
		if (err == 12) {
			free(final_val);
			return 12;
		}
	}
	if (strncmp(*line, "#define", 7) == 0) {
		err = parseDefine(array, *line);
		if (err == 12) {
			free(final_val);
			return 12;
		}
	} else if (strncmp(*line, "#undef", 6) == 0) {
		token = strtok(*line, " ");
		token = strtok(NULL, "\n");
		rem(*array, token);
	} else if (strncmp(*line, "#ifdef", 6) == 0) {
		token = strtok(*line, " ");
		token = strtok(NULL, "\n");
		if (get(*array, token, &value) == 0)
			*cond = 1;
		else
			*cond = -1;
	} else if (strncmp(*line, "#ifndef", 7) == 0) {
		token = strtok(*line, " ");
		token = strtok(NULL, "\n");
		if (get(*array, token, &value) != 0)
			*cond = 1;
		else
			*cond = -1;
	} else if (strncmp(*line, "#endif", 6) == 0)
		*cond = 0;
	else if (strncmp(*line, "#if", 3) == 0) {
		token = strtok(*line, " ");
		token = strtok(NULL, "\n");
		if (token[0] != '0')
			*cond = 1;
		else
			*cond = -1;
	} else if (strncmp(*line, "#elif", 5) == 0) {
		token = strtok(*line, " ");
		token = strtok(NULL, "\n");
		if (token[0] != '0' && *cond != 1)
			*cond = 1;
		else
			*cond = -1;
	} else if (strncmp(*line, "#else", 5) == 0) {
		if (*cond != 1)
			*cond = 1;
		else
			*cond = -1;
	} else {
		if (*cond >= 0)
			fputs(*line, fp_out);
	}
	free(final_val);
	return 0;
}

/* try to open a header file, adding a given path */
FILE *tryToOpen(FILE *fp, char *path, char *infile, char *header)
{
	char ch = '/';

	strncat(path, &ch, 1);
	if (strlen(path) > 1)
		strcpy(header, path);
	else
		strcpy(header, "");
	strcat(header, infile);
	fp = fopen(header, "r");
	return fp;
}

/*
 * recursive function that opens all header files and read them
 * line by line in the correct order
 */
int goInDepth(struct Elem **array, char *infile,
			FILE *fp_out, char *outfile, char *path,
			char *dir1, char *dir2, int *cond)
{
	char *line = NULL;
	char *token;
	int err;
	FILE *fp;
	char *header;

	line = malloc(sizeof(char) * MAX_LINE_LENGTH);
	if (line == NULL)
		return 12;
	header = malloc(sizeof(char) * MAX_LINE_LENGTH);
	if (header == NULL) {
		free(line);
		return 12;
	}
	/* open the header by trying out all possible paths */
	fp = fopen(infile, "r");
	if (fp == NULL) {
		fp = tryToOpen(fp, path, infile, header);
		if (fp == NULL) {
			fp = tryToOpen(fp, dir1, infile, header);
			if (fp == NULL) {
				fp = tryToOpen(fp, dir2, infile, header);
				if (fp == NULL) {
					free(line);
					free(header);
					return -1;
				}
			}
		}
	}
	while (1) {
		err = readLine(&line, fp);
		if (err == -1)
			break;
		if (err == 12) {
			free(line);
			free(header);
			if (fp != stdin)
				fclose(fp);
			return 12;
		}
		/* if another #include is found,
		 * go recurssively in specified header
		 */
		if (strncmp(line, "#include", 8) == 0) {
			line[strcspn(line, "\n")] = 0;
			token = strtok(line, " ");
			token = strtok(NULL, " ");
			token += 1;
			token[strlen(token) - 1] = '\0';
			err = goInDepth(array, token, fp_out,
							outfile, path, dir1,
							dir2, cond);
			if (err == 12 || err == -1) {
				free(line);
				free(header);
				if (fp != stdin)
					fclose(fp);
				return 12;
			}
			continue;
		}
		/* parse the rest of the file when you return from recurssion */
		err = parseFunction(array, fp_out, &line, fp, cond);
		if (err == 12) {
			if (line != NULL)
				free(line);
			if (header != NULL)
				free(header);
			if (fp != stdin)
				fclose(fp);
			return 12;
		}
	}

	free(line);
	free(header);
	if (fp != stdin)
		fclose(fp);
	return 0;
}

/* free all dynamically allocated memory */
int freeAll(struct Elem *array, char *infile,
			char *outfile, char *dir1, char *dir2,
			char *path, char *line)
{
	freeStruct(array);
	free(infile);
	free(outfile);
	free(dir1);
	free(dir2);
	free(path);
	free(line);
	return 0;
}

int main(int argc, char **argv)
{
	struct Elem *array = NULL;
	int err;
	char *infile = NULL;
	char *dir1 = NULL;
	char *dir2 = NULL;
	char *outfile = NULL;
	char *path = NULL;
	char *token;
	FILE *fp_in = NULL;
	FILE *fp_out = NULL;
	char *line = NULL;
	int end_of_read = 0;
	int cond = 0;

	/* memory allocation */
	err = allocStruct(&array);
	if (err == 12) {
		printf("HashMap allocation error\n");
		return 12;
	}
	infile = malloc(sizeof(char) * 100);
	if (infile == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}
	outfile = malloc(sizeof(char) * 100);
	if (outfile == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}
	dir1 = malloc(sizeof(char) * 100);
	if (dir1 == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}
	dir2 = malloc(sizeof(char) * 100);
	if (dir2 == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}
	path = malloc(sizeof(char) * 100);
	if (path == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}
	line = malloc(sizeof(char) * MAX_LINE_LENGTH);
	if (line == NULL) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}

	strcpy(infile, "");
	strcpy(outfile, "");
	strcpy(dir1, "");
	strcpy(dir2, "");
	strcpy(path, "");
	/* arguments parsing */
	err = getArgs(&array, &infile, &outfile, &dir1,
				&dir2, &path, argc, argv);
	if (err == 12) {
		freeAll(array, infile, outfile, dir1, dir2, path, line);
		return 12;
	}

	/* manage input */
	if (strcmp(infile, "") == 0)
		fp_in = stdin;
	else {
		fp_in = fopen(infile, "r");
		if (fp_in == NULL) {
			printf("Error: InFile pointer is null.\n");
			freeAll(array, infile, outfile, dir1, dir2, path, line);
			return 12;
		}
	}

	/* manage output */
	if (strcmp(outfile, "") == 0)
		fp_out = stdout;
	else {
		fp_out = fopen(outfile, "w");
		if (fp_out == NULL) {
			printf("Error: OutFile pointer is null.\n");
			freeAll(array, infile, outfile, dir1, dir2, path, line);
			return 12;
		}
	}

	/* keep reading lines from file while EOF is not reached */
	while (end_of_read == 0) {
		err = readLine(&line, fp_in);
		if (err == 12) {
			freeAll(array, infile, outfile, dir1, dir2, path, line);
			return 12;
		}
		if (err == -1) {
			end_of_read = 1;
			break;
		}
		/* if an #include is found, go recursively
		 * in headers and parse them
		 */
		if (strncmp(line, "#include", 8) == 0) {
			line[strcspn(line, "\n")] = 0;
			token = strtok(line, " ");
			token = strtok(NULL, " ");
			token += 1;
			token[strlen(token) - 1] = '\0';
			err = goInDepth(&array, token, fp_out,
							outfile, path, dir1,
							dir2, &cond);
			if (err == 12 || err == -1) {
				if (fp_in != stdin)
					fclose(fp_in);
				if (fp_out != stdout)
					fclose(fp_out);
				freeAll(array, infile, outfile,
					dir1, dir2, path, line);
				return err;
			}
			continue;
		}
		/* after header(s) have been parsed,
		 * continue with the rest of file
		 */
		err = parseFunction(&array, fp_out, &line, fp_in, &cond);
		if (err == 12) {
			if (fp_in != stdin)
				fclose(fp_in);
			if (fp_out != stdout)
				fclose(fp_out);
			freeAll(array, infile, outfile, dir1, dir2, path, line);
			return 12;
		}
	}

	if (fp_in != stdin)
		fclose(fp_in);
	if (fp_out != stdout)
		fclose(fp_out);

	freeAll(array, infile, outfile, dir1, dir2, path, line);
	return 0;
}
