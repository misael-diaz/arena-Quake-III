#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "util.h"

struct MemChain {
	struct MemChain *prev;
	struct MemChain *next;
	void *data;
	size_t size;
};

static struct MemChain m_chain;
static size_t m_size = 0;
static size_t m_count = 0;

static struct MemChain *Util_Chain (struct MemChain *node)
{
	struct MemChain *next = (m_chain.next)? m_chain.next : NULL;
	if (next) {
		next->prev = node;
	}

	node->next = next;
	node->prev = &m_chain;
	m_chain.next = node;
	return node;
}

static struct MemChain *Util_Remove (struct MemChain *node)
{
	struct MemChain *prev = node->prev;
	struct MemChain *next = node->next;
	if (next) {
		next->prev = prev;
	}

	prev->next = next;
	node->next = NULL;
	node->prev = NULL;
	node->data = NULL;
	free(node);
	node = NULL;
	return node;
}

void *Util_Free (void *p)
{
	if (!p) {
		return NULL;
	}

	struct MemChain *node = ((struct MemChain*) p) - 1;
	size_t const size = node->size;
	node = Util_Remove(node);

	m_size -= size;
	--m_count;

	return NULL;
}

void Util_Clear (void)
{
	struct MemChain *next = NULL;
	for (struct MemChain *node = m_chain.next; node; node = next) {
		next = node->next;
		void *data = node->data;
		node = (struct MemChain*) Util_Free(data);
	}

	m_size = 0;
	m_count = 0;
}

void *Util_Malloc (size_t const sz)
{
	size_t const size = sizeof(struct MemChain) + sz;
	void *p = NULL;
	int const rc = posix_memalign(p, CACHELINE_SIZE, size);
	if (rc || !p) {
		errno = rc; // we have to set errno, see man posix_memalign
		fprintf(stderr, "Util_Malloc: %s\n", strerror(errno));
		Util_Clear();
		exit(EXIT_FAILURE);
	}

	struct MemChain* node = (struct MemChain*) p;
	void *data = (node + 1);

	node = Util_Chain(node);
	node->data = data;
	node->size = size;

	m_size += size;
	++m_count;

	return data;
}

char *Util_CopyString (const char *string)
{
	size_t const len = strlen(string);
	size_t const sz = (len + 1);
	void *ptr = Util_Malloc(sz);
	if (!ptr) {
		fprintf(stderr, "Util_CopyString: error\n");
		return NULL;
	}

	const char *src = string;
	char *dst = (char*) ptr;
	return strcpy(dst, src);
}

void Util_Init (void)
{
	static_assert(sizeof(struct MemChain) == CACHELINE_SIZE);
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/util/util.c

Copyright (C) 2024 Misael Díaz-Maldonado

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/
