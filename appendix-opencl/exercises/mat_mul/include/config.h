#pragma once

#include<stdlib.h>  // for FILE* and stdin and stderr and shit
#include<stdbool.h>

#ifdef CONFIG_INCLUDE_IMPLEMENTATION
#define ARGPARSE_INCLUDE_IMPLEMENTATION
#endif
#include"argparse.h"

// the parameters for the various test and benchmark functions are getting
// out of hand, any edit to these motherfuckers requires a thesis worth of
// change this, change that, edit this, edit that
// so I'm making one big struct and putting everything in here
// so I can just edit the struct, pass the same struct around, and fuck yall
typedef struct {
    unsigned int height_A;
    unsigned int width_B;
    unsigned int common_dim; // = width_A = height_B
                 
    size_t group_width;  
    size_t group_height;  // width and height of opencl work groups
    // (global dimensions will be height_A * width_B)

    FILE* test_output;           // where to write test logs to
    FILE* test_error_output;     // [...] test errors to
    FILE* benchmark_output;      // [...] benchmark outputs to
    FILE* benchmark_error_output;// [...] benchmark errors to

	bool also_test_cpu;
	bool also_benchmark_cpu;
} mm_config;

// populate config with default values
// can't really do ={0} as that would not make much sense here
void populate_default_config(mm_config* mmc);

// create cli argument specs and bind them to fields of config struct
void bind_arg_specs_to_config(arg_specs* specs, mm_config* mmc);

// parse argv and set the fields of struct
// will leave unspecified fields of structure alone, to avoid undefined fields
// call populate_default_config on the confg struct before calling this.
// calls bind_arg_specs_to_config internally
void parse_argv_into_config(int argc, char** argv, mm_config* mmc);

#ifdef CONFIG_INCLUDE_IMPLEMENTATION
#include "config.c"
#endif
