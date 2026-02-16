#include"argparse.h"

// so... this is most likely a very suboptimal way to do what I'm doing
// I am, sadly, not in a position to care rn
//
// but yeah it needs more error handling

arg_specs* argparse_create_empty_specs() {
	arg_specs* specs = (arg_specs*)malloc(sizeof(arg_specs));
	// zero it out
	memset(specs, 0, sizeof(arg_specs));
	// (this is not necessairy rn) ensure it is of ARGPARSE_EMPTY type
	specs->type = ARGPARSE_EMPTY;
	return specs;
}

void argparse_bind(arg_specs* specs, const char* argname,
				   void* ptr, const char* help, argparse_type type) {
	// appending into a linked list
	// empty case (does this even work? idfk)
	if(specs->type == ARGPARSE_EMPTY) {
		specs->flag_name = argname;
		specs->ptr = ptr;
		specs->help = help;
		specs->type = type;
		specs->next = NULL;
		return;
	}

	// otherwise
	arg_specs* next = (arg_specs*)malloc(sizeof(arg_specs));
	*next = (arg_specs){
		.flag_name = argname,
		.ptr = ptr,
		.help = help,
		.type = type,
		.next = NULL
	};

	// append into linked list 
	arg_specs* prev = specs;
	arg_specs* curr = prev->next;
	while(curr != NULL) {
		prev = prev->next;
		curr = curr->next;
	}
	prev->next=next;
}

void argparse_bind_flag(arg_specs* specs, const char* argname,
						bool* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_FLAG);
}

void argparse_bind_int(arg_specs* specs, const char* argname,
					   int* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_INT);
}

void argparse_bind_uint(arg_specs* specs, const char* argname,
						unsigned int* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_UINT);
}

void argparse_bind_size_t(arg_specs* specs, const char* argname,
						  size_t* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_SIZE_T);
}

void argparse_bind_float(arg_specs* specs, const char* argname,
						 float* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_FLOAT);
}

void argparse_bind_double(arg_specs* specs, const char* argname,
						  double* ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_DOUBLE);
}

void argparse_bind_string(arg_specs* specs, const char* argname,
						  char** ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_STRING);
}

void argparse_bind_fileptr(arg_specs* specs, const char* argname,
						   FILE** ptr, const char* help) {
	argparse_bind(specs, argname, (void*)ptr, help, ARGPARSE_FILEPTR);
}


const char* argparse_stringify_type(argparse_type type) {
	switch(type) {
	case ARGPARSE_INT: return "integer";
	case ARGPARSE_UINT: return "unsigned integer";
	case ARGPARSE_SIZE_T: return "size type";

	case ARGPARSE_FLOAT: return "float";
	case ARGPARSE_DOUBLE: return "double float";

	case ARGPARSE_STRING: return "string";
	case ARGPARSE_FILEPTR: return "file";
	default:
		fprintf(stderr, "argparse_stringify_type: unrecognized type");
		return "if you're reading this the author fucked something up";
	}
}

const char* argparse_strip_leading_dashes(const char* c) {
	const char* stripped = c;
	while(*stripped=='-')stripped++;
	return stripped;
}


arg_specs* argparse_find_spec(arg_specs* specs, const char* name) {
	for(arg_specs* as = specs; as; as = as->next) {
		if(strcmp(name, as->flag_name) == 0)
			return as;
	}
	fprintf(stderr,
			"argparse_get_spec: found no spec for name %s\n", name);
	return NULL;
}

// returns true on success and false on failure
bool argparse_assign_values(arg_specs* specs, int argc, char** argv) {
	int i = 1;

	while(i<argc) {
		arg_specs* spec = argparse_find_spec(specs, argv[i]);
		if(!spec) {
			fprintf(stderr,
					"oops! found '%s' argument but I don't know"
					" how to read it!"
					"\nargument parsing has ended prematurely and data that"
					" depends on command line arguments is likely invalid"
					" please rerun this with the correct flag, thanks\n",
					argv[i]);
			return false;
		}
			
		switch(spec->type) {
		case ARGPARSE_FLAG:
			*((bool*)spec->ptr) = true;
			i++;
			break;

		case ARGPARSE_INT:
			*((int*)spec->ptr) = atoi(argv[i+1]);
			i+=2;
			break;
		case ARGPARSE_UINT:
			*((unsigned int*)spec->ptr) = atoi(argv[i+1]);
			i+=2;
			break;
		case ARGPARSE_SIZE_T:
			*((size_t*)spec->ptr) = atoi(argv[i+1]);
			i+=2;
			break;

		case ARGPARSE_FLOAT:
			sscanf(argv[i+1], "%f", (float*)spec->ptr);
			i+=2;
			break;
		case ARGPARSE_DOUBLE:
			// bit of a hack, we lose some precision, but it works
			// going by k&r %e seems to be an output thing only
			// and using "%e" as input results in a warning with -Wpedantic
			// does scanf even handle doubles?
			{
				float f;
				sscanf(argv[i+1], "%e", &f);
				*(double*)spec->ptr = (double)f;
				i+=2;
				break;
			}

		case ARGPARSE_STRING:
			*((char**)spec->ptr) = argv[i+1];
			i+=2;
			break;
		case ARGPARSE_FILEPTR:
			if(strcmp(argv[i+1], "stdout") == 0)
				*((FILE**)spec->ptr) = stdout;
			else if(strcmp(argv[i+1], "stdin") == 0)
				*((FILE**)spec->ptr) = stdin;
			else if(strcmp(argv[i+1], "stderr") == 0)
				*((FILE**)spec->ptr) = stderr;
			else {
				fprintf(stderr,
						"passing arbitrary files as flags not supported,"
						" as of wiriting, because idfk when to close them,"
						" please use either stdin, stdout, stderr, or accept"
						" the file name as a string, thanks :)\n");
			}
			i+=2;
			break;
		case ARGPARSE_EMPTY:
			fprintf(stderr, "cannot pass empty argument specs to"
					" argparse_assign_values, please populate specs object"
					" with flag specifications before passing using it to"
					" assign values");
			return false;
		}
	}

	return true;
}


void argparse_print_usage(arg_specs* specs, const char* prog_name) {
	printf("usage: %s ", prog_name);
	for(arg_specs* as = specs; as; as = as->next) {
		printf("[%s %s]", as->flag_name,
			   argparse_strip_leading_dashes(as->flag_name));
	}
	putchar('\n');
}

void argparse_print_flag_help(const arg_specs* as) {
	printf("%s: default value[", as->flag_name);
	switch(as->type) {
	case ARGPARSE_FLAG:
		printf("%s", *((bool*)as->ptr)?"true":"false");
		break;

	case ARGPARSE_INT:
		printf("%d", *((int*)as->ptr));
		break;
	case ARGPARSE_UINT:
		printf("%u", *((unsigned int*)as->ptr));
		break;
	case ARGPARSE_SIZE_T:
		printf("%zu", *((size_t*)as->ptr));
		break;

	case ARGPARSE_FLOAT:
		printf("%f", *((float*)as->ptr));
		break;
	case ARGPARSE_DOUBLE:
		printf("%e", *((double*)as->ptr));
		break;

	case ARGPARSE_STRING:
		printf("\"%s\"", *((char**)as->ptr));
		break;

	case ARGPARSE_FILEPTR:
		{
		FILE* f = *((FILE**)as->ptr);
		if(f == stdin)
			printf("stdin");
		else if(f == stdin)
			printf("stdout");
		else if(f == stderr)
			printf("stderr");
		else
			printf("idfk");
		}
		break;

	default:
		fprintf(stderr, "argparse_print: unrecognized type");
	}
	printf("] type:[%s]", argparse_stringify_type(as->type));
	
	if(strcmp(as->help, "") != 0)
		printf("\n%s\n", as->help);
	else
		puts("");
}

void argparse_print_flag_help_name(arg_specs* specs, const char* name) {
	argparse_print_flag_help(argparse_find_spec(specs, name));
}

void argparse_print_flags_help(arg_specs* specs) {
	for(arg_specs* as = specs; as; as = as->next) {
		argparse_print_flag_help(as);
	}
}

void argparse_print_help(arg_specs* specs, char* prog_name) {
	argparse_print_usage(specs, prog_name);
	argparse_print_flags_help(specs);
}


void argparse_free(arg_specs* specs) {
	for(arg_specs* as = specs; as;) {
		arg_specs* old = as;
		as = as->next;
		free(old);
	}
}
