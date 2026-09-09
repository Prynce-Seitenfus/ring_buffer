#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "atomic.h"

/**
 * @brief Lock-free Single-Producer Single-Consumer (SPSC) Ring Buffer.
 *
 * Designed specifically for bare-metal and RTOS applications on ARM Cortex-M cores.
 * In an SPSC configuration:
 * - The Producer context (e.g. high-priority ISR or dedicated task) exclusively writes
 *   via ring_buffer_push().
 * - The Consumer context (e.g. background task or secondary ISR) exclusively reads
 *   via ring_buffer_pop().
 *
 * Index variables are encapsulated in atomic_size_t and managed with acquire/release
 * memory fences to prevent register caching and out-of-order execution without disabling interrupts.
 */
typedef struct RingBuffer {
    uint8_t* buffer;        /**< Pointer to caller-allocated static data buffer. */
    size_t capacity;        /**< Total allocated capacity (must be a power of 2). */
    size_t mask;            /**< Fast wrap-around bitmask (capacity - 1). */
    atomic_size_t head;     /**< Write index, modified solely by the Producer. */
    atomic_size_t tail;     /**< Read index, modified solely by the Consumer. */
} RingBuffer;

/**
 * @brief Initializes the lock-free SPSC ring buffer instance.
 *
 * Validates input parameters and configures the ring buffer structure.
 * The buffer capacity must be a power of 2 (e.g., 16, 64, 256, 1024, etc.).
 * To distinguish between full and empty states without shared atomic counters,
 * one slot remains empty, providing an effective capacity of (capacity - 1) bytes.
 *
 * @param rb Pointer to the RingBuffer structure to initialize.
 * @param buffer Pointer to a caller-allocated uint8_t storage array.
 * @param capacity Total size of the storage array in bytes (must be power of 2 >= 2).
 * @return true if initialization succeeded, false if any parameter is invalid.
 */
bool ring_buffer_init(RingBuffer* rb, uint8_t* buffer, size_t capacity);

/**
 * @brief Enqueues a single byte into the ring buffer (Producer API).
 *
 * Lock-free operation. Must only be called from the single Producer context.
 * Ensures memory ordering via atomic release/acquire semantics, remaining completely hardware-agnostic.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @param data Byte value to be enqueued.
 * @return true if data was successfully written, false if the buffer is full.
 */
bool ring_buffer_push(RingBuffer* rb, uint8_t data);

/**
 * @brief Dequeues a single byte from the ring buffer (Consumer API).
 *
 * Lock-free operation. Must only be called from the single Consumer context.
 * Ensures memory ordering via atomic release/acquire semantics, remaining completely hardware-agnostic.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @param data Pointer to a uint8_t variable where the dequeued byte is written.
 * @return true if data was successfully read, false if the buffer is empty.
 */
bool ring_buffer_pop(RingBuffer* rb, uint8_t* data);

/**
 * @brief Checks if the ring buffer is empty.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @return true if empty or invalid pointer, false if contains data.
 */
bool ring_buffer_is_empty(const RingBuffer* rb);

/**
 * @brief Checks if the ring buffer is full.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @return true if full, false otherwise.
 */
bool ring_buffer_is_full(const RingBuffer* rb);

/**
 * @brief Returns the number of bytes currently stored in the ring buffer.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @return Number of stored bytes, or 0 if pointer is NULL.
 */
size_t ring_buffer_count(const RingBuffer* rb);

/**
 * @brief Returns the maximum usable byte capacity of the ring buffer.
 *
 * @param rb Pointer to the RingBuffer instance.
 * @return Usable capacity in bytes (capacity - 1), or 0 if pointer is NULL.
 */
size_t ring_buffer_capacity(const RingBuffer* rb);

/**
 * @brief Clears the ring buffer, resetting it to the empty state.
 *
 * @warning This function is not thread-safe and must only be called during initialization
 * or when both producer and consumer contexts are inactive.
 *
 * @param rb Pointer to the RingBuffer instance.
 */
void ring_buffer_clear(RingBuffer* rb);

#endif /* RING_BUFFER_H */
