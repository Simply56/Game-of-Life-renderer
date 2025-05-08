#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const size_t WIDTH = 400;
const size_t HEIGHT = 400;

// Rust functions
void init_window(size_t width, size_t height);
void render_frame(const uint32_t *pixels, size_t width, size_t height);

void print_fps()
{
    static int frame_count = 0;
    static double last_time = 0;
    static double fps = 0.0;

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    double now = current_time.tv_sec + (current_time.tv_nsec / 1e9);

    frame_count++;

    // Calculate FPS every second
    if (now - last_time >= 1.0) {
        fps = frame_count / (now - last_time);
        frame_count = 0;
        last_time = now;
        printf("FPS: %.2f\n", fps);
    }
}

uint32_t decode_state(uint8_t state)
{
    const uint8_t blue = 1;
    const uint8_t orange = 2;
    const uint8_t dead = 3;

    switch (state) {
    case blue:
        return 0x0099FF;
    case orange:
        return 0xFF9900;
    case dead:
        return 0x6666666;
    case dead + 1:
        return 0x7f7f7f;
    case dead + 2:
        return 0x999999;
    default: // empty
        return 0xFFFFFF;
    }
}

void dense_non_sparse(uint32_t *pixels)
{
    // one byte per cell, all cells included
    uint8_t buffer[WIDTH];

    while (true) {
        for (size_t i = 0; i < HEIGHT; i++) {
            ssize_t total_read = 0;
            while (total_read < sizeof(buffer)) {
                ssize_t just_read = read(STDIN_FILENO, buffer + total_read, WIDTH - total_read);
                if (just_read <= 0) {
                    exit(1);
                }
                total_read += just_read;
            }

            for (size_t j = 0; j < WIDTH; j++) {
                pixels[i * WIDTH + j] = decode_state(buffer[j]);
            }
        }

        render_frame(pixels, WIDTH, HEIGHT);
        print_fps();
    }
}

int main()
{
    uint32_t *pixels = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    init_window(WIDTH, HEIGHT);

    dense_non_sparse(pixels);

    for (int i = 0; i < 3000; ++i) {
        if (read(STDIN_FILENO, pixels, WIDTH * HEIGHT * 4) != WIDTH * HEIGHT * 4) {
            fprintf(stderr, "short read\n");
            goto err;
        }
        render_frame(pixels, WIDTH, HEIGHT);
        print_fps();
    }

    int rv = 0;
    if (false) {
    err:
        rv = 1;
    }

    free(pixels);
    return rv;
}
