#include<stdlib.h> // for FILE* and stdin and stderr and shit

#define ARGPARSE_INCLUDE_IMPLEMENTATION
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

} mm_config;

// populate config with default values
// can't really do ={0} as that would not make much sense here
void populate_default_config(*mm_config);

// create cli argument specs and bind them to fields of config struct
void bind_arg_specs_to_config(args_specs* specs, mm_config* mmc);

// parse argv and set the fields of struct
// will leave unspecified fields of structure alone, to avoid undefined fields
// call populate_default_config on the confg struct before calling this.
// calls bind_arg_specs_to_config internally
void parse_argv_into_config(int argc, char** argv, mm_config* mmc);

#ifdef CONFIG_IMPLEMENTATION
void populate_default_config(mm_config* mmc) {
    mmc->height_A = 300;    // height of first matrix
    mmc->common_dim = 200;  // width of first matrix/height of second matrix
    mmc->height_B = 400;    // width of second matrix

    mmc->group_width=32;  // width/first dim of work groups
    mmc->group_height=32; // height/second dim of work groups

    mmc->test_output = stdout;             // where to write test logs to
    mmc->test_error_outuput = stderr;      // [...] test errors to
    mmc->benchmark_output = stdout;        // [...] benchmark outputs to
    mmc->benchmark_error_outuput = stderr; // [...] benchmark errors to

    mmc->also_test_cpu=false;
    mmc->also_benchmark_cpu=false;
}

void argparse_flags_setup(args_specs* specs, mm_config* mmc) {
    argparse_bind_uint(specs, "--height-a" &(mmc->height_A),
                       "height of first matrix in computing res = A * B");
    argparse_bind_uint(specs, "--width-b" &(mmc->width_B),
                       "width of second matrix in computing res = A * B");
    argparse_bind_uint(specs, "--common-dim" &(mmc->common_dim),
                       "common dimension between matrices in computing"
                       " res = A * B");
                 
    argparse_bind_size_t(specs, "--group-width", &(mmc->group_width),
                         "first dimension/width of the work groups");  
    argparse_bind_size_t(specs, "--group-width", &(mmc->group_height),
                         "second dimension/height of the work groups");  

    argparse_bind_flag(specs, "--test-cpu", &(mmc->also_test_cpu),
					   "specify this flag to also test the"
					   " cpu implementation");
    argparse_bind_flag(specs, "--benchmark-cpu", &(mmc->also_benchmark_cpu),
					   "specify this flag to also benchmark"
					   " the cpu implementation");

	// won't touch these for now as file handles lead to resource fuckery
    // test_output
    // test_error_output
    // benchmark_output
    // benchmark_error_output
}

void parse_argv_into_config(int argc, char** argv, mm_config* mmc) {
	arg_specs* specs = argparse_create_empty_specs();
	bind_arg_specs_to_config(specs, mmc);
	if(argparse_assign_values(specs, argc, argv))
		puts("arguments bound succesfully!");
	else
		fpritnf(stderr, "error while binding arguments!\n");

	argparse_free(specs);
}

#endif
