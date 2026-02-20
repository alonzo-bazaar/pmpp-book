#ifndef BOIDS_ARGPARSE_H
#define BOIDS_ARGPARSE_H

/**
 * functions for
 * creating command line flag schema
 * parsing (and validating) argv according to schema in question 
 * set program variables to values specified in argv
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

typedef enum {
	// special type used to mark an empty argparse list
	ARGPARSE_EMPTY = 0,

	// an ARGPARSE_FLAG type argument does not expect a value after it
	// used for yes/no things like --help --quiet --verbose
	ARGPARSE_FLAG,
		       
	// all these other types always do expect a value if they're specified
	// used for things like --verbose-level
	ARGPARSE_INT, ARGPARSE_UINT, ARGPARSE_SIZE_T,
	ARGPARSE_FLOAT, ARGPARSE_DOUBLE,
	ARGPARSE_STRING, ARGPARSE_FILEPTR
} argparse_type;

// can't be fucked to make a has map in c
// we're doing this alist style, the way god intended
struct arg_specs {
	const char* flag_name;
	void* ptr;
	const char* help;
	argparse_type type;
	struct arg_specs* next;
};

typedef struct arg_specs arg_specs;

arg_specs* argparse_create_empty_specs();

// so yeah, this is what generics were invented for
// I could probably make a macro for this tbh, or just rawdog void pointers
// but... I can't be fucked to test that
// repetition is better than the wrong abstraction, I suppose
// (even tho a macro could output the same code I've written here, but faster)
void argparse_bind(arg_specs* specs, const char* argname,
				   void* ptr, const char* help, argparse_type type);

void argparse_bind_flag(arg_specs* specs, const char* argname,
						bool* ptr, const char* help);

void argparse_bind_int(arg_specs* specs, const char* argname,
					   int* ptr, const char* help);
void argparse_bind_uint(arg_specs* specs, const char* argname,
						unsigned int* ptr, const char* help);
void argparse_bind_size_t(arg_specs* specs, const char* argname,
						  size_t* ptr, const char* help);

void argparse_bind_float(arg_specs* specs, const char* argname,
						 float* ptr, const char* help);
void argparse_bind_double(arg_specs* specs, const char* argname,
						  double* ptr, const char* help);

void argparse_bind_string(arg_specs* specs, const char* argname,
						  char** ptr, const char* help);
void argparse_bind_fileptr(arg_specs* specs, const char* argname,
						   FILE** ptr, const char* help);

const char* argparse_stringify_type(argparse_type type);
const char* argparse_strip_leading_dashes(const char* c);

arg_specs* argparse_find_spec(arg_specs* specs, const char* name);
bool argparse_assign_values(arg_specs* specs, int argc, char** argv);

void argparse_print_usage(arg_specs* specs, const char* argv0);
void argparse_print_flag_help(const arg_specs* as);
void argparse_print_flag_help_name(arg_specs* specs, const char* name);
void argparse_print_flags_help(arg_specs* specs);
void argparse_print_help(arg_specs* specs, char* argv0);

void argparse_free(arg_specs* specs);

#ifdef ARGPARSE_INCLUDE_IMPLEMENTATION
#include"argparse.c"
#endif

#endif
