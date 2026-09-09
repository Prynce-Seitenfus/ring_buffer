# Ring Buffer

A lightweight, lock-free Single-Producer Single-Consumer (SPSC) circular FIFO buffer in C99, tailored for bare-metal ARM Cortex-M and RTOS embedded applications.

---

## Key Features

* **Lock-Free SPSC**: Requires no mutexes, semaphores, critical sections, or interrupt-disabling primitives. Safe for asynchronous Producer-Consumer contexts (e.g. high-priority ISR writing and background RTOS task reading).
* **Zero Dynamic Allocation**: Operates entirely over a caller-provided static storage array.
* **Fast Bitwise Wrap-Around**: Buffer capacity is enforced as a power of 2, enabling $O(1)$ index wrap-around using bitwise AND (`& mask`) without division or modulo (`%`) instructions.
* **Hardware-Agnostic Synchronization**: Built on a dedicated atomic abstraction layer ([`atomic.h`](../atomic/atomic.h)) using acquire/release memory fences.
* **MISRA C:2012 Compliant**: Adheres strictly to MISRA C:2012 standards, including defensive parameter validation, unsigned integer literals, and explicit typing.

---

## API Reference

The public API is declared in [`ring_buffer.h`](ring_buffer.h):

```c
/**
 * @brief Initializes the ring buffer instance.
 * @param rb Pointer to RingBuffer instance.
 * @param buffer Pointer to caller-allocated uint8_t storage array.
 * @param capacity Buffer capacity in bytes (must be power of 2 >= 2).
 * @return true on success, false if parameters are invalid.
 */
bool ring_buffer_init(RingBuffer* rb, uint8_t* buffer, size_t capacity);

/**
 * @brief Enqueues a single byte (Producer API).
 * Thread-safe for the single Producer context without locks.
 * @param rb Pointer to RingBuffer instance.
 * @param data Byte to write.
 * @return true if written, false if buffer is full.
 */
bool ring_buffer_push(RingBuffer* rb, uint8_t data);

/**
 * @brief Dequeues a single byte (Consumer API).
 * Thread-safe for the single Consumer context without locks.
 * @param rb Pointer to RingBuffer instance.
 * @param data Pointer to destination byte.
 * @return true if read, false if buffer is empty.
 */
bool ring_buffer_pop(RingBuffer* rb, uint8_t* data);

/**
 * @brief Checks if the buffer is empty.
 */
bool ring_buffer_is_empty(const RingBuffer* rb);

/**
 * @brief Checks if the buffer is full.
 */
bool ring_buffer_is_full(const RingBuffer* rb);

/**
 * @brief Returns the number of bytes currently stored.
 */
size_t ring_buffer_count(const RingBuffer* rb);

/**
 * @brief Returns usable capacity (capacity - 1).
 */
size_t ring_buffer_capacity(const RingBuffer* rb);

/**
 * @brief Resets head and tail to zero.
 */
void ring_buffer_clear(RingBuffer* rb);
```

---

## Usage Example

```c
#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>

#define UART_RX_BUFFER_SIZE 64U

static RingBuffer rx_rb;
static uint8_t rx_storage[UART_RX_BUFFER_SIZE];

void system_init(void)
{
    /* Initialize ring buffer with a power-of-2 static buffer */
    (void)ring_buffer_init(&rx_rb, rx_storage, UART_RX_BUFFER_SIZE);
}

/* Producer context: UART Interrupt Service Routine */
void UART_RX_IRQHandler(void)
{
    uint8_t received_byte = UART0->DR; /* Hardware read */
    (void)ring_buffer_push(&rx_rb, received_byte);
}

/* Consumer context: Main loop or background RTOS task */
void process_telemetry(void)
{
    uint8_t byte = 0U;
    while (ring_buffer_pop(&rx_rb, &byte)) {
        /* Process byte */
    }
}
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.