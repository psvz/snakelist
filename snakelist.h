#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>

#define SNAKELIST_LAYERS_NUMBER	7	// max nodes# is then 1 << (this number + 1)
#define SNAKELIST_RANDOM_STATE	64	// bytes, see random(3) manpage

#define DECLARE_SNAKELIST_N(members)	struct snakelist_node\
{\
	struct snakelist_node *next[SNAKELIST_LAYERS_NUMBER];\
	struct snakelist_node *prev[SNAKELIST_LAYERS_NUMBER];\
	members\
}

typedef struct snakelist_node		snakelist_n;

typedef struct snakelist_instance
{
	unsigned	nodes_count;
	unsigned	layers_count;
	pthread_mutex_t change_mtx;
	snakelist_n	*head;
	char		random_state[SNAKELIST_RANDOM_STATE];
}
snakelist_i;

snakelist_i*
snakelist_create();
/*
** snakelist_insert() returns NULL on success or the dupe that is already on the list
** snakelist_scan() returns list dupe of the node or NULL if no such dupe on the list
** snakelist_next/prev iterates through the list, if node is NULL, the first=minimal/
** last=maximal node is returned if instance is not NULL. Otherwise NULL is returned
*/
snakelist_n* snakelist_insert(snakelist_i*, snakelist_n*,
		     int (*compar)(const snakelist_n*, const snakelist_n*));

snakelist_n* snakelist_scan(const snakelist_i*, const snakelist_n*,
		     int (*compar)(const snakelist_n*, const snakelist_n*));


snakelist_n* snakelist_next(const snakelist_i *in, const snakelist_n *node);


snakelist_n* snakelist_prev(const snakelist_i *in, const snakelist_n *node);
