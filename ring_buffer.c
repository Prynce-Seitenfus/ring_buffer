#include "ring_buffer.h"

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
    rb->capacity = capacity;
    rb->mask = capacity - 1U;
    atomic_store_release(&rb->head, 0U);
    atomic_store_release(&rb->tail, 0U);

    return true;
}

bool ring_buffer_push(RingBuffer* rb, uint8_t data)
{
    if (rb == NULL) {
        return false;
    }

    size_t current_head = atomic_load_acquire(&rb->head);
    size_t current_tail = atomic_load_acquire(&rb->tail);
    size_t next_head = (current_head + 1U) & rb->mask;

    if (next_head == current_tail) {
        return false; /* Buffer is full */
    }

    rb->buffer[current_head] = data;

    atomic_store_release(&rb->head, next_head);

    return true;
}

bool ring_buffer_pop(RingBuffer* rb, uint8_t* data)
{
    if ((rb == NULL) || (data == NULL)) {
        return false;
    }

    size_t current_head = atomic_load_acquire(&rb->head);
    size_t current_tail = atomic_load_acquire(&rb->tail);

    if (current_head == current_tail) {
        return false; /* Buffer is empty */
    }

    *data = rb->buffer[current_tail];

    atomic_store_release(&rb->tail, (current_tail + 1U) & rb->mask);

    return true;
}

bool ring_buffer_is_empty(const RingBuffer* rb)
{
    if (rb == NULL) {
        return true;
    }

    return (atomic_load_acquire(&rb->head) == atomic_load_acquire(&rb->tail));
}

bool ring_buffer_is_full(const RingBuffer* rb)
{
    if (rb == NULL) {
        return false;
    }

    size_t current_head = atomic_load_acquire(&rb->head);
    size_t current_tail = atomic_load_acquire(&rb->tail);

    return (((current_head + 1U) & rb->mask) == current_tail);
}

size_t ring_buffer_count(const RingBuffer* rb)
{
    if (rb == NULL) {
        return 0U;
    }

    size_t current_head = atomic_load_acquire(&rb->head);
    size_t current_tail = atomic_load_acquire(&rb->tail);

    return ((current_head - current_tail) & rb->mask);
}

size_t ring_buffer_capacity(const RingBuffer* rb)
{
    if (rb == NULL) {
        return 0U;
    }

    return rb->mask;
}

void ring_buffer_clear(RingBuffer* rb)
{
    if (rb != NULL) {
        atomic_store_release(&rb->head, 0U);
        atomic_store_release(&rb->tail, 0U);
    }
}
