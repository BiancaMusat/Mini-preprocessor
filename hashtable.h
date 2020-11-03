#ifndef HASHTABLE_H_
#define HASHTABLE_H_

typedef struct Elem {
	char *key;
	char *value;
} Elem;
int allocStruct(struct Elem **array);
void freeStruct(struct Elem *array);
int get(struct Elem *array, char *key, char **value);
int put(struct Elem *array, char *key, char *value);
int rem(struct Elem *array, char *key);
int getSize(struct Elem *array);
int resize(struct Elem **array);

#endif      /* HASHTABLE_H_ */