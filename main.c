#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>
#include <string.h>

void timespec_diff(struct timespec* timespec1, struct timespec* timespec2) {
    timespec1->tv_nsec = timespec1->tv_nsec - timespec2->tv_nsec;
    if (timespec1->tv_nsec < 0) {
        timespec1->tv_nsec += ONE_BILLION;
        timespec1->tv_sec--;
    }

    timespec1->tv_sec = timespec1->tv_sec-timespec2->tv_sec;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        puts("Please Provide a tuimov File!");
        return EXIT_FAILURE;
    }

    int exit_code = EXIT_FAILURE;

    char* tui_file_path = argv[1];
    FILE* tui_file = fopen(tui_file_path, "rb");

    if (tui_file == NULL) {
        puts("Could not open tuimov File!");
        return EXIT_FAILURE;
    }

    int c;
    bool read_sleep = false;

    unsigned char second_buf[8] = {0};
    unsigned char nanos_buf[8] = {0};
    
    size_t read_iter = 0;
    struct timespec sleep_time, frame_start, frame_end;

    timespec_get(&frame_start, TIME_UTC);
    printf(CLEAR_SCREEN CURSOR_INVISIBLE);

    while((c = fgetc(tui_file)) != EOF) {
        if (read_sleep) {
            if (read_iter < 4) {
                second_buf[read_iter] = c;
            } else if (read_iter < 8) {
                nanos_buf[read_iter-4] = c;
            }

            read_iter++;
            if (read_iter==8) {
                memcpy(&sleep_time.tv_sec, second_buf, sizeof(__time_t));
                memcpy(&sleep_time.tv_nsec, nanos_buf, sizeof(__syscall_slong_t));
                read_sleep = false;
                read_iter = 0;
                timespec_get(&frame_end, TIME_UTC);
                timespec_diff(&frame_end, &frame_start);
                timespec_diff(&sleep_time, &frame_end);
                thrd_sleep(&sleep_time, NULL);
                timespec_get(&frame_start, TIME_UTC);
            }
            
        } else {
            if (c == 0) {
                fflush(stdout);
                read_sleep = true;
            }
            else putchar(c);
        }
    }

    if (ferror(tui_file)) puts("Error Playing Back tuimov File");
    else if (feof(tui_file)) {
        exit_code = EXIT_SUCCESS;
    }

    return exit_code;
}