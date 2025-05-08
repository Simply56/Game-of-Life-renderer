#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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

/*Reads from stdin until the buffer is full*/
ssize_t read_buff(uint8_t *buffer, size_t size)
{
    ssize_t total_read = 0;
    while (total_read < sizeof(buffer)) {
        ssize_t just_read = read(STDIN_FILENO, buffer + total_read, size - total_read);
        if (just_read <= 0) {
            return -1;
        }
        total_read += just_read;
    }
    return total_read;
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

int dense_non_sparse(uint32_t *pixels, size_t width, size_t height)
{
    // one byte per cell, all cells included
    uint8_t buffer[width];

    while (true) {
        for (size_t i = 0; i < height; i++) {
            if (read_buff(buffer, width) == -1) {
                return -1;
            }
            for (size_t j = 0; j < width; j++) {
                pixels[i * width + j] = decode_state(buffer[j]);
            }
        }

        render_frame(pixels, width, height);
        print_fps();
    }
}

/*Reads data from the stdin and renders it derictly without interpretation*/
int raw_render(uint32_t *pixels, uint32_t width, uint32_t height)
{
    while (true) {
        if (read_buff(pixels, width * height * 4) == -1) {
            return -1;
        }
        render_frame(pixels, width, height);
        print_fps();
    }
}

int main(int argc, char *argv[])
{
    int rv = 0;
    uint32_t width = 400;
    uint32_t height = 400;

    char *endptr;
    int option;
    while ((option = getopt(argc, argv, "w:h:")) != -1) {
        if (option == '?' || option == ':') {
            // getopt prints error text
            return EXIT_FAILURE;
        }

        switch (option) {
        case 'w':
            width = (uint32_t) strtoumax(optarg, &endptr, 10);
            break;
        case 'h':
            height = (uint32_t) strtoumax(optarg, &endptr, 10);
            break;
        }
    }

    init_window(width, height);
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    if (!pixels) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    rv = dense_non_sparse(pixels, width, height);
    // rv = raw_render(pixels, width, height);

    free(pixels);
    return rv;
}
