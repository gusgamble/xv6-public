#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

// Refactor of XV6 distribution umalloc.c to make variables and algorithm more
// clear

struct header {
	uint          size;
};

struct freelist_node {
	struct freelist_node *next;
	struct header hdr;
};

static struct freelist_node  base;
static struct freelist_node *freelist;

void
free(void *ptr)
{
	struct freelist_node *mem, *node;

	mem = (struct freelist_node *)ptr - 1;
	node = freelist;
	while (!(mem > node && mem < node->next) ) {
		if (node >= node->next && (mem > node || mem < node->next)) break;
		node = node->next;
	}
	if (mem + mem->hdr.size == node->next) {
		mem->hdr.size += node->next->hdr.size;
		mem->next = node->next->next;
	} else
		mem->next = node->next;

	if (node + node->hdr.size == mem) {
		node->hdr.size += mem->hdr.size;
		node->next = mem->next;
	} else
		node->next = mem;

	freelist = node;
}

static struct freelist_node *
morecore(uint nu)
{
	char *  ptr;
	struct freelist_node *node;

	if (nu < 4096) nu = 4096;
	ptr = sbrk(nu * sizeof(struct freelist_node));
	if (ptr == (char *)-1) return 0;
	node         = (struct freelist_node *)ptr;
	node->hdr.size = nu;
	free((void *)(node + 1));
	return freelist;
}

void *
malloc(uint nbytes)
{
	struct freelist_node *node, *prev_node, *found_node;
	uint    nunits;
	int		difference = 0;
	int		old_diff = 9999999;

	nunits = (nbytes + sizeof(struct freelist_node) - 1) / sizeof(struct freelist_node) + 1;
	if (freelist == 0) {
		base.next = freelist = prev_node = &base;
		base.hdr.size = 0;
	}

	node = freelist->next;
	prev_node = freelist;
	found_node = freelist->next;

	// do a scan over the whole list to find the node with the closest size
	// if not found, do the same thing it did before

	while (node->next != &base) {
		prev_node = node;
		node = node->next;

		difference = node->hdr.size - nunits;
		if (difference < 0) {
			difference *= -1;
		}
		if (old_diff>difference) {
			old_diff = difference;
			found_node = node;
		}
	}
	while (1)	{
		// printf(1, "in a found node\n");
		if (found_node->hdr.size >= nunits) {
			// this is what is doing the "first fit" in the origial
			if (found_node->hdr.size == nunits) {
				// this just marks the whole free area as now used
				prev_node->next = found_node->next;
			} else {
				// we need to update the start of the new shrunken free area
				found_node->hdr.size -= nunits;
				found_node += found_node->hdr.size;
				found_node->hdr.size = nunits;
			}
			freelist = prev_node;
			return (void *)(found_node + 1);
		}

		if (found_node == freelist)
			if ((found_node = morecore(nunits)) == 0) return 0;

		prev_node = found_node;
		found_node = found_node->next;
	}
}
