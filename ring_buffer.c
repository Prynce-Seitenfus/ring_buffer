#include "ring_buffer.h"
#include <string.h>

/* Helper to check whether capacity is a non-zero power of 2 (>= 2) */
static bool is_power_of_two(size_t value)
{
    return ((value >= 2U) && ((value & (value - 1U)) == 0U));
}

bool ring_buffer_init(RingBuffer* rb, uint8_t* buffer, size_t capacity)
{
    if ((rb == NULL) || (buffer == NULL)) {
        return false;
    }

    if (is_power_of_two(capacity) == false) {
        return false;
    }

    rb->buffer = buffer;
    rb->mask = capacity - 1U;
    atomic_store_release(&rb->head, 0U);
    atomic_store_release(&rb->tail, 0U);

    return true;
}

size_t ring_buffer_write(RingBuffer* rb, const uint8_t* data, size_t count)
{
    if ((rb == NULL) || (data == NULL) || (count == 0U)) {
        return 0U;
    }

    size_t current_head = atomic_load_relaxed(&rb->head);
    size_t current_tail = atomic_load_acquire(&rb->tail);

    size_t free_space = rb->mask - ((current_head - current_tail) & rb->mask);
    size_t to_write = (count < free_space) ? count : free_space;

    if (to_write == 0U) {
        return 0U;
    }

    size_t capacity = rb->mask + 1U;
    size_t first_chunk = capacity - current_head;
    if (first_chunk > to_write) {
        first_chunk = to_write;
    }

    (void)memcpy(&rb->buffer[current_head], data, first_chunk);

    size_t second_chunk = to_write - first_chunk;
    if (second_chunk > 0U) {
        (void)memcpy(&rb->buffer[0], &data[first_chunk], second_chunk);
    }

    size_t next_head = (current_head + to_write) & rb->mask;
    atomic_store_release(&rb->head, next_head);

    return to_write;
}

size_t ring_buffer_read(RingBuffer* rb, uint8_t* data, size_t count)
{
    if ((rb == NULL) || (data == NULL) || (count == 0U)) {
        return 0U;
    }

    size_t current_head = atomic_load_acquire(&rb->head);
    size_t current_tail = atomic_load_relaxed(&rb->tail);

    size_t available = (current_head - current_tail) & rb->mask;
    size_t to_read = (count < available) ? count : available;

    if (to_read == 0U) {
        return 0U;
    }

    size_t capacity = rb->mask + 1U;
    size_t first_chunk = capacity - current_tail;
    if (first_chunk > to_read) {
        first_chunk = to_read;
    }

    (void)memcpy(data, &rb->buffer[current_tail], first_chunk);

    size_t second_chunk = to_read - first_chunk;
    if (second_chunk > 0U) {
        (void)memcpy(&data[first_chunk], &rb->buffer[0], second_chunk);
    }

    size_t next_tail = (current_tail + to_read) & rb->mask;
    atomic_store_release(&rb->tail, next_tail);

    return to_read;
}

void ring_buffer_clear(RingBuffer* rb)
{
    if (rb != NULL) {
        atomic_store_release(&rb->head, 0U);
        atomic_store_release(&rb->tail, 0U);
    }
}
