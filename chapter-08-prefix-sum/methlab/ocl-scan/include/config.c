#include"config.h"

void populate_default_config(ps_config* psc) {
    psc->array_length = 256; // will keep a power of 2 = group length for first two examples
    psc->group_length = 256; // does an intel iris even have warps?
                             // idk, but keeping everythig multiples of
                             // 2^something is probably still a good idea

    psc->test_output = stdout;            // where to write test logs to
    psc->test_error_output = stderr;      // [...] test errors to
    psc->benchmark_output = stdout;       // [...] benchmark outputs to
    psc->benchmark_error_output = stderr; // [...] benchmark errors to

    psc->also_test_cpu=true;
    psc->also_benchmark_cpu=true;
}

void bind_arg_specs_to_config(arg_specs* specs, ps_config* psc) {
    argparse_bind_size_t(specs, "--arr-len", &(psc->array_length),
                         "length of the array we'll do the prefix sum over");
    argparse_bind_size_t(specs, "--group-length", &(psc->group_length),
                         "length of the work groups we will use to do the sum");

    argparse_bind_flag(specs, "--test-cpu", &(psc->also_test_cpu),
					   "specify this flag to also test the"
					   " cpu implementation");
    argparse_bind_flag(specs, "--benchmark-cpu", &(psc->also_benchmark_cpu),
					   "specify this flag to also benchmark"
					   " the cpu implementation");

	// won't touch the following atm as file handles lead to resource fuckery
	// they will be set to the default shit
    // test_output
    // test_error_output
    // benchmark_output
    // benchmark_error_output
}

void parse_argv_into_config(int argc, char** argv, ps_config* psc) {
	arg_specs* specs = argparse_create_empty_specs();
	bind_arg_specs_to_config(specs, psc);
	if(argparse_assign_values(specs, argc, argv))
		puts("arguments bound succesfully!");
	else
		fprintf(stderr, "error while binding arguments!\n");

	argparse_free(specs);
}
