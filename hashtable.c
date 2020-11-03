#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "hashtable.h"

#define SENTRY (char *)1

int SIZE = 11;

/* Constructor */
int allocStruct(struct Elem **array)
{
	int i;
	/* Allocate memory for array of Elem structures */
	*array = malloc(SIZE * sizeof(struct Elem));
	if (*array == NULL)
		return 12;

	/* Allocate memory for each element of the hashtable */
	for (i = 0; i < SIZE; ++i) {
		(*array)[i].key = NULL;
		(*array)[i].value = NULL;
	}

	return 0;
}

/* Destructor */
void freeStruct(struct Elem *array)
{
	int i;

	for (i = 0; i < SIZE; i++) {
		if (array[i].key != NULL && array[i].key != SENTRY) {
			free(array[i].key);
			free(array[i].value);
		}
	}
	free(array);
}

/* return actual size of hashtable */
int getSize(struct Elem *array)
{
	int i;
	int size = 0;

	for (i = 0; i < SIZE; i++) {
		if (array[i].key != NULL && array[i].key != SENTRY)
			size++;
	}
	return size;
}

/* Hash function */
unsigned int hash(char *str)
{
	int hash = 53;
	int c;

	while ((c = *str++))
		hash = ((hash << 2) + hash) + c;

	return hash % SIZE;
}

/* Return element if exists, NULL otherwise */
int get(struct Elem *array, char *key, char **value)
{
	/* compute hash of key */
	int key_hash = hash(key);
	int sz = SIZE;
	/* find entrance for the pair */
	while (sz != 0) {
		if (array[key_hash].key == NULL)
			return -1;
		if (array[key_hash].key != SENTRY &&
				strcmp(array[key_hash].key, key) == 0) {
			*value = array[key_hash].value;
			return 0;
		}
		key_hash++;
		key_hash %= SIZE;
		sz--;
	}
	return -1;
}

/* Add element to hashtable */
int put(struct Elem *array, char *key, char *value)
{
	int key_hash;

	if (getSize(array) == SIZE)
		return -1;
	/* compute hash of key */
	key_hash = hash(key);
	/* find empty entrance for the pair */
	while (array[key_hash].key != NULL &&
		   array[key_hash].key != SENTRY &&
		   strcmp(array[key_hash].key, key) != 0) {
		/* go to next cell */
		key_hash++;
		key_hash %= SIZE;
	}
	if (array[key_hash].key != NULL &&
			array[key_hash].key != SENTRY) {
		free(array[key_hash].key);
		free(array[key_hash].value);
	}
	array[key_hash].key = malloc(strlen(key) + 1);
	if (array[key_hash].key == NULL)
		return 12;
	strcpy(array[key_hash].key, key);
	if (value != NULL) {
		array[key_hash].value = malloc(strlen(value) + 1);
		if (array[key_hash].value == NULL)
			return 12;
		strcpy(array[key_hash].value, value);
	} else {
		array[key_hash].value = NULL;
	}
	return 0;
}

/* Remove element */
int rem(struct Elem *array, char *key)
{
	/* compute hash of key */
	int key_hash = hash(key);
	int sz = SIZE;
	/* find entrance for the pair */
	while (sz != 0) {
		if ((array[key_hash].key != NULL) &&
				(array[key_hash].key != SENTRY) &&
				strcmp(array[key_hash].key, key) == 0) {
			free(array[key_hash].key);
			free(array[key_hash].value);
			array[key_hash].key = SENTRY;
			array[key_hash].value = NULL;
			return 0;
		}
		if (array[key_hash].key == NULL)
			return -1;
		key_hash++;
		key_hash %= SIZE;
		sz--;
	}
	return -1;
}

/* resize hashtable */
int resize(struct Elem **array)
{
	int i;
	int size;
	struct Elem *temp;

	size = 2 * SIZE;
	SIZE = size;

	/* Allocate memory for array of Elem structures */
	temp = malloc(size * sizeof(struct Elem));
	if (temp == NULL)
		return 12;
	/* Allocate memory for each element of the hashtable */
	for (i = 0; i < size; ++i) {
		temp[i].key = NULL;
		temp[i].value = NULL;
	}
	/* copy entrances from initial hashtable */
	for (i = 0; i < size / 2; i++) {
		if ((*array)[i].key != NULL && (*array)[i].key != SENTRY)
			put(temp, (*array)[i].key, (*array)[i].value);
	}
	array = &temp;
	free(*array);

	return 0;
}
