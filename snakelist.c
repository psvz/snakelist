#include "test.h"

snakelist_i*
snakelist_create()
{
	snakelist_i *p;

	if ( (p = calloc(1, sizeof(snakelist_i))))
	{
		if ( (p->head = calloc(1, sizeof(snakelist_n))) == NULL)
		{
			free(p);
			return NULL;
		}

		pthread_mutex_init(&p->change_mtx, NULL);

		initstate(time(NULL), p->random_state, SNAKELIST_RANDOM_STATE);
		if (errno)
		{
			syslog(LOG_CRIT, "snakelist_create(): initstate(): %m");
			_exit(EXIT_FAILURE);
		}
	}

	return p;
}

snakelist_n*
snakelist_scan(const snakelist_i *in, const snakelist_n *node,
	int (*compar)(const snakelist_n*, const snakelist_n*))
{
	int		s;	// result of compar()
	unsigned	i;
	snakelist_n	*p;
	unsigned	right = 1;

	if (!in || !node || !compar) return NULL;	// sanity

	p = in->head;

	i = in->layers_count;

	while (i > 0)
	{
		if (right)
		{
			// here, we decrease layer:
			while (i > 0 && p->next[--i] == NULL);

			// and load the pointer:
			if ( (p = p->next[i]) == NULL) break;

			// to avoid compar() on the same node twice:
			while ( (s = compar(node, p)) > 0 && p->next[i])
									p = p->next[i];

			// s = 0 => target found
			// s > 0 => decrease layer and same direction
			// s < 0 => decrease layer and flip direction

			if (s < 0) right = 0;

		} else {

			while (i > 0 && p->prev[--i] == NULL);

			if ( (p = p->prev[i]) == NULL) break;

			while ( (s = compar(node, p)) < 0 && p->prev[i])
									p = p->prev[i];

			if (s > 0) right = 1;
		}

		if (s == 0) return p;	// found
	}
	return NULL;
}

snakelist_n*
snakelist_insert(snakelist_i *in, snakelist_n *node,
	int (*compar)(const snakelist_n*, const snakelist_n*))
{
	int		s;
	long		r; // random generator
	unsigned	i;
	snakelist_n	*p, *pfollow;
	unsigned	right = 1;

	if (!in || !node || !compar) return NULL;

	if ( (errno = pthread_mutex_lock(&in->change_mtx)) != 0)
	{
		syslog(LOG_CRIT, "snakelist_insert(): pthread_mutex_lock(): %m");
		_exit(EXIT_FAILURE);
	}

	p = in->head;

	i = in->layers_count;

	while (i > 0)
	{
		if (right)
		{
			while (i > 0 && p->next[--i] == NULL)
			{
				node->prev[i] = p;
				node->next[i] = NULL;
			}

			if ( (p = p->next[i]) == NULL) break;

			while ( (s = compar(node, p)) > 0 && p->next[i])
									p = p->next[i];

			if (s < 0)
			{
				right = 0;

				node->next[i] = p;
				node->prev[i] = p->prev[i];

			} else { // we assume s > 0 here, neglecting s = 0

				node->prev[i] = p;
				node->next[i] = NULL;
			}

		} else {

			while (i > 0 && p->prev[--i] == NULL)
			{
				node->next[i] = p;
				node->prev[i] = NULL;
			}

			if ( (p = p->prev[i]) == NULL) break;

			while ( (s = compar(node, p)) < 0 && p->prev[i])
									p = p->prev[i];

			if (s > 0)
			{
				right = 1;

				node->prev[i] = p;
				node->next[i] = p->next[i];

			} else { // assuming s < 0

				node->next[i] = p;
				node->prev[i] = NULL;
			}
		}

		if (s == 0)
		{
			pthread_mutex_unlock(&in->change_mtx);
			return p;
		}
	}

	/* here, we stitch the node into scan-chains up the layers and check the head: */

	for (r = 0; i < in->layers_count && r <= RAND_MAX / 2; i++) // note that i = 0 here as scan finishes
	{							    // at base layer and the node is missing
		if (node->prev[i]) node->prev[i]->next[i] = node;

		else in->head->next[i] = node;	// special case of entry point

		if (node->next[i]) node->next[i]->prev[i] = node;

		else in->head->prev[i] = node;	// only needed to "get max first"

		r = random();
	}

	/* time for layers management, the very first insert() call falls right down here too */

	if (in->layers_count == 0)
	{
		node->next[0] = NULL;
		node->prev[0] = NULL;

		in->head->next[0] = node;
		in->head->prev[0] = node;

		in->layers_count = 1;
	}

	in->nodes_count++;

	if (in->nodes_count >= 1 << (in->layers_count + 1))
	{
		if (in->layers_count < SNAKELIST_LAYERS_NUMBER)
		{
			i = in->layers_count - 1;
			p = pfollow = in->head;

			for ( ;; )
			{
				while ( (p = p->next[i]) && random() > RAND_MAX / 2);

				pfollow->next[i + 1] = p;

				if (p == NULL) break;

				p->prev[i + 1] = pfollow;

				pfollow = p;
			}

			if (in->head->next[i + 1])
			{
				in->head->next[i + 1]->prev[i + 1] = NULL;

				in->head->prev[i + 1] = pfollow;

				in->layers_count++;
			}
		}
		else syslog(LOG_WARNING, "snakelist_insert(): >%u nodes: increase number of layers!",
								1 << (SNAKELIST_LAYERS_NUMBER + 1));
	}

	pthread_mutex_unlock(&in->change_mtx);

	return NULL;
}

inline
snakelist_n*
snakelist_next(const snakelist_i *in, const snakelist_n *node)
{
	return node ? node->next[0] : (in ? in->head->next[0] : NULL);
}

inline
snakelist_n*
snakelist_prev(const snakelist_i *in, const snakelist_n *node)
{
	return node ? node->prev[0] : (in ? in->head->prev[0] : NULL);
}
