#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define EVENT_QUEUE_SIZE 256
#define EVENT_QUEUE_MAX EVENT_QUEUE_SIZE

enum SysEventType {
	SE_NONE,
	SE_KEY,
	SE_CHAR,
	SE_MOUSE,
	SE_JOYSTICK_AXIS,
	SE_CONSOLE,
	SE_PACKET
};

struct SysEvent {
	struct SysEvent *evNext;
	void *evPtr;
	enum SysEventType evType;
	int64_t evTime;
	int64_t evValue;
	int64_t evValue2;
	size_t evPtrLength;
	int64_t: 64;
};

struct MemChain {
	struct MemChain *prev;
	struct MemChain *next;
	void *data;
	size_t size;
};

static struct SysEvent *eventQueueHead = NULL;
static struct SysEvent *eventQueueTail = NULL;
static size_t eventQueueSize = 0;

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

static void *Util_Free (void *p)
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

static void Util_Clear (void)
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

static void *Util_Malloc (size_t const sz)
{
	size_t const size = sizeof(struct MemChain) + sz;
	void *p = malloc(size);
	if (!p) {
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

static char *Util_CopyString (const char *string)
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

static void SE_EmQueue (struct SysEvent event)
{
	struct SysEvent *ev = (struct SysEvent*) Util_Malloc(sizeof(event));
	*ev = event;
	ev->evNext = NULL;
	if (!eventQueueHead) {
		eventQueueHead = ev;
		eventQueueTail = ev;
		++eventQueueSize;
		return;
	}

	eventQueueTail->evNext = ev;
	eventQueueTail = ev;
	++eventQueueSize;
}

static struct SysEvent SE_DeQueue (void)
{
	struct SysEvent event;
	memset(&event, 0, sizeof(event));
	if (!eventQueueHead) {
		return event;
	}

	event = *eventQueueHead;
	eventQueueHead = Util_Free(eventQueueHead);
	eventQueueHead = event.evNext;
	event.evNext = NULL;
	--eventQueueSize;
	return event;
}

static size_t SE_SzQueue (void)
{
	return eventQueueSize;
}

static void SE_ClearEvent (struct SysEvent *ev)
{
	ev->evPtr = Util_Free(ev->evPtr);
}

int main ()
{
	assert(sizeof(struct SysEvent) == 64);
	printf("sizeof(struct SysEvent): %zu\n", sizeof(struct SysEvent));
	for (size_t i = 0; i != EVENT_QUEUE_SIZE; ++i) {
		struct SysEvent ev;
		memset(&ev, 0, sizeof(ev));
		ev.evPtr = Util_CopyString("fake event data");
		ev.evPtrLength = 1 + strlen(ev.evPtr);
		SE_EmQueue(ev);
	}
	printf("size: %zu\n", SE_SzQueue());
	for (size_t i = 0; i != EVENT_QUEUE_SIZE; ++i) {
		struct SysEvent ev = SE_DeQueue();
		SE_ClearEvent(&ev);
	}
	printf("size: %zu\n", SE_SzQueue());
	return 0;
}

/*

Quake-III                                             June 07, 2024

author: @misael-diaz
source: src/main/main.c

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
