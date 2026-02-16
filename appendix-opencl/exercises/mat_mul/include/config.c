#include"config.h"

void populate_default_config(mm_config* mmc) {
    mmc->height_A = 300;    // height of first matrix
    mmc->common_dim = 200;  // width of first matrix/height of second matrix
    mmc->width_B = 400;    // width of second matrix

    mmc->group_width=32;  // width/first dim of work groups
    mmc->group_height=32; // height/second dim of work groups

    mmc->test_output = stdout;             // where to write test logs to
    mmc->test_error_output = stderr;      // [...] test errors to
    mmc->benchmark_output = stdout;        // [...] benchmark outputs to
    mmc->benchmark_error_output = stderr; // [...] benchmark errors to

    mmc->also_test_cpu=false;
    mmc->also_benchmark_cpu=false;
}

void bind_arg_specs_to_config(arg_specs* specs, mm_config* mmc) {
    argparse_bind_uint(specs, "--height-a", &(mmc->height_A),
                       "height of first matrix in computing res = A * B");
    argparse_bind_uint(specs, "--width-b", &(mmc->width_B),
                       "width of second matrix in computing res = A * B");
    argparse_bind_uint(specs, "--common-dim", &(mmc->common_dim),
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

	// won't touch the following atm as file handles lead to resource fuckery
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
		fprintf(stderr, "error while binding arguments!\n");

	argparse_free(specs);
}
