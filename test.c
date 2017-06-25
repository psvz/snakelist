#include "test.h"

int
compar(const snakelist_n *a, const snakelist_n *b)
{
	return (a->key - b->key);
}

int
main (int argc, char **argv)
{
	int		i, j;

	snakelist_i	*in;
	snakelist_n	*node, *p;

struct	timespec	tick, tock;

	in = snakelist_create();

	for (i = 0; i < 140; i++)
	{
		node = calloc(1, sizeof(snakelist_n));
		node->key = i;
		snakelist_insert(in, node, compar);
	}
	for (i = 150; i > 130; i--)
        {
                node = calloc(1, sizeof(snakelist_n));
                node->key = i;
                snakelist_insert(in, node, compar);
        }
	for (i = 0; i < SNAKELIST_RANDOM_STATE; i++)
		printf("%hhX", in->random_state[i]);
	printf("\n");

	node = calloc(1, sizeof(snakelist_n));
	node->key = -50;
	snakelist_insert(in, node, compar);
	node = calloc(1, sizeof(snakelist_n));
	node->key = 678956;
	snakelist_insert(in, node, compar);;
	printf("nodes: %u (expected 153), layers: %u (exp 7)\n", in->nodes_count, in->layers_count);
	node = calloc(1, sizeof(snakelist_n));
	node->key = -5;
	if (snakelist_scan(in, node, compar))
		printf("oops -5 is found\n");

	node->key = 266;
        if (snakelist_scan(in, node, compar))
                printf("oops 266 is found\n");

        node->key = 90;
        if (snakelist_scan(in, node, compar))
                printf("yes: 90 is found\n");

	for (j = 0; j < 7; j++)
	{
		for (i = 0, p = in->head; (p = p->next[j]); i++);
		printf("%dth layer count (153 nodes): %d\n", j + 1, i);
	}
	for (i = 151; i < 400; i++)
	{
		node = calloc(1, sizeof(snakelist_n));
		node->key = i;
		snakelist_insert(in, node, compar);
	}
	printf("check syslog: must be overloading: %u (402) nodes now\n", in->nodes_count);

        for (j = 0; j < 7; j++)
        {
                for (i = 0, p = in->head; (p = p->next[j]); i++);
                printf("%dth layer count (402 nodes): %d\n", j + 1, i);
        }

	printf("max key: %d\n", snakelist_prev(in, NULL)->key);

        for (i = 0; i < SNAKELIST_RANDOM_STATE; i++)
                printf("%hhX", in->random_state[i]);
        printf("\n");

	node = calloc(1, sizeof(snakelist_n));
	clock_gettime(CLOCK_MONOTONIC, &tick);
	for (i = 0; i < 2E6; i++)
	{
		double r = random();
		node->key = r / (RAND_MAX + 1.0) * 1000 - 500;
		snakelist_scan(in, node, compar);
	}
	clock_gettime(CLOCK_MONOTONIC, &tock);
	if ((tock.tv_nsec -= tick.tv_nsec) < 0)
	{
		tock.tv_nsec += 1E9;
		tock.tv_sec--;
	}
	tock.tv_sec -= tick.tv_sec;
	printf("skiplist did %.6fs\n", tock.tv_sec + tock.tv_nsec / 1E9);

	clock_gettime(CLOCK_MONOTONIC, &tick);
        for (i = 0; i < 2E6; i++)
        {
                double r = random();
                node->key = r / (RAND_MAX + 1.0) * 1000 - 500;
                for (p = in->head->next[0]; (p) && compar(p, node) != 0; p = p->next[0]);
        }
        clock_gettime(CLOCK_MONOTONIC, &tock);
        if ((tock.tv_nsec -= tick.tv_nsec) < 0)
        {
                tock.tv_nsec += 1E9;
                tock.tv_sec--;
        }
        tock.tv_sec -= tick.tv_sec;
        printf("lin*list did %.6fs\n", tock.tv_sec + tock.tv_nsec / 1E9);


	return 0;
}
