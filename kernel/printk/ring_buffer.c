#include "internal.h"

static void ring_buf_flush_part(ring_buf_t *rbuf, int len) {
	while (!ring_buf_empty(rbuf) && len--) {
		rbuf->out_func(rbuf->buffer[rbuf->head++]);
		rbuf->head %= ring_buf_capacity(rbuf);
	}
	rbuf->full = 0;
}

// Ring Buffer APIs Implementation

void ring_buf_init(ring_buf_t *rbuf, void (*out_func)(int)) {
	ring_buf_reset(rbuf);
	rbuf->out_func = out_func;
}

void ring_buf_reset(ring_buf_t *rbuf) {
	rbuf->head = rbuf->tail = 0;
	rbuf->full				= 0;
}

void ring_buf_put(ring_buf_t *rbuf, uint8_t data) {
	if (ring_buf_full(rbuf))
		ring_buf_flush_part(rbuf, ring_buf_capacity(rbuf) / 4);
	rbuf->buffer[rbuf->tail++] = data;
	rbuf->tail %= ring_buf_capacity(rbuf);
	if (rbuf->tail == rbuf->head)
		rbuf->full = 1;
}

void ring_buf_flush(ring_buf_t *rbuf) {
	ring_buf_flush_part(rbuf, ring_buf_size(rbuf));
}

int ring_buf_empty(ring_buf_t *rbuf) {
	return (rbuf->head == rbuf->tail) && (!rbuf->full);
}

int ring_buf_full(ring_buf_t *rbuf) {
	return (rbuf->head == rbuf->tail) && rbuf->full;
}

size_t ring_buf_capacity(ring_buf_t *rbuf) { return MAX_RING_BUF_SIZE; }

size_t ring_buf_size(ring_buf_t *rbuf) {
	if (rbuf->tail < rbuf->head)
		return rbuf->tail + ring_buf_capacity(rbuf) - rbuf->head;
	else
		return rbuf->tail - rbuf->head;
}
