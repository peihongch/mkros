#ifndef __INTERNAL_H_
#define __INTERNAL_H_

#include "types.h"

#define MAX_RING_BUF_SIZE 64

typedef struct ring_buffer {
	uint8_t buffer[MAX_RING_BUF_SIZE];
	size_t	head;
	size_t	tail;
	int		full;
	void (*out_func)(int);
} ring_buf_t;

// Initialize the ring buffer
void ring_buf_init(ring_buf_t *rbuf, void (*out_func)(int));

// Reset the ring buffer to empty, head == tail
void ring_buf_reset(ring_buf_t *rbuf);

// Put data to buffer and automatically flush if neccessary
void ring_buf_put(ring_buf_t *rbuf, uint8_t data);

// Flush the buffer
void ring_buf_flush(ring_buf_t *rbuf);

// Check if the buffer is empty
int ring_buf_empty(ring_buf_t *rbuf);

// Check if the buffer is full
int ring_buf_full(ring_buf_t *rbuf);

// Returns the maximum capacity of the buffer
size_t ring_buf_capacity(ring_buf_t *rbuf);

// Returns the current number of elements in the buffer
size_t ring_buf_size(ring_buf_t *rbuf);

#endif /* __INTERNAL_H_ */
