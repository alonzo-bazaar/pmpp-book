#pragma once

// if DEBUGGING is defined will define DEBUG_PRINT(F)(_ERR)
// to call error print functions
// the functions are turned into no-ops is debugging isn't defined
// so you can turn off error debugging on a program wide scale without needing
// to go arround commenting shit all over the place
// 
// PRINTF macros function accept a format and data to format
// they are just macros around printf
// it is an error to use them without passing anything
// PRINT functions do not format data, and are wrappers arround puts
// this is just an overly convoluted way not to have to depend on __VA_OPT__
// 
// ERR functions put debug outout to stderr
// otherwise they put it to stdout

#ifdef DEBUGGING 

#define DEBUG_PRINTF(fmt, ...)                          \
    fprintf(stdout, "[DEBUG] " fmt "\n", __VA_ARGS__)
#define DEBUG_PRINTF_ERR(fmt, ...)                          \
    fprintf(stderr, "[DEBUG ERROR] " fmt "\n", __VA_ARGS__)
#define DEBUG_PRINT(msg)                        \
    fputs(stdout, "[DEBUG] " msg)
#define DEBUG_PRINT_ERR(msg)                    \
    fputs(stderr, "[DEBUG ERROR] " msg)

#else

#define DEBUG_PRINTF(fmt, ...) (void)0
#define DEBUG_PRINTF_ERR(fmt, ...) (void)0
#define DEBUG_PRINT(msg) (void)0
#define DEBUG_PRINT_ERR(msg) (void)0

#endif
