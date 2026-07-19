#include "macros.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <threads.h>
#include <time.h>
#include <string.h>
#ifdef _MSC_VER
    #include <locale.h>
#endif

void timespec_add(struct timespec* timespec1, struct timespec* timespec2) {
    timespec1->tv_nsec = timespec1->tv_nsec + timespec2->tv_nsec;
    if (timespec1->tv_nsec > ONE_BILLION) {
        timespec1->tv_nsec -= ONE_BILLION;
        timespec1->tv_sec++;
    }

    timespec1->tv_sec = timespec1->tv_sec + timespec2->tv_sec;
}

void timespec_diff(struct timespec* timespec1, struct timespec* timespec2) {
    timespec1->tv_nsec = timespec1->tv_nsec - timespec2->tv_nsec;
    if (timespec1->tv_nsec < 0) {
        timespec1->tv_nsec += ONE_BILLION;
        timespec1->tv_sec--;
    }

    timespec1->tv_sec = timespec1->tv_sec - timespec2->tv_sec;
}

union time_onion {
    long long_val;
    time_t time_val;
    unsigned char buf[8];
};

int main(int argc, char** argv) {
    if (argc == 1) {
        puts("Please Provide a tuimov File!");
        return EXIT_FAILURE;
    }
    #ifdef _MSC_VER
        char buf[65536];
        setvbuf(stdout, buf, _IOFBF, 65536);
        setlocale(LC_ALL, ".UTF8");
    #endif
    bool do_loop = false;

    if (argc >= 3) {
        if (strcmp("-l", argv[2]) == 0) do_loop = true;
    }

    int exit_code = EXIT_FAILURE;

    char* tui_file_path = argv[1];
    FILE* tui_file = fopen(tui_file_path, "rb");

    if (tui_file == NULL) {
        puts("Could not open tuimov File!");
        return EXIT_FAILURE;
    }
    
    int c;

    union time_onion time_onion;
    
    struct timespec sleep_time, current_time, next_time;
    timespec_get(&next_time, TIME_UTC);
    printf(CLEAR_SCREEN CURSOR_INVISIBLE);
    fflush(stdout);
    
    
    while((c = fgetc(tui_file)) != EOF || do_loop) {
        if (c == EOF && do_loop) {
            rewind(tui_file);
            c = fgetc(tui_file);
            printf(CLEAR_SCREEN CURSOR_INVISIBLE);
        }

        if (c == 0) {
            fflush(stdout);

            fread(time_onion.buf, 4, 1, tui_file);
            sleep_time.tv_sec = time_onion.time_val;
            fread(time_onion.buf,  4, 1, tui_file);
            sleep_time.tv_nsec = time_onion.long_val;

            timespec_add(&next_time, &sleep_time);
            sleep_time = next_time;
            timespec_get(&current_time, TIME_UTC);
            timespec_diff(&sleep_time, &current_time);
            
            if (sleep_time.tv_sec >= 0) 
                thrd_sleep(&sleep_time, NULL);
        }
        else putchar(c);
    }

    if (ferror(tui_file)) puts("Error Playing Back tuimov File");
    else if (feof(tui_file)) {
        exit_code = EXIT_SUCCESS;
    }

    return exit_code;
}