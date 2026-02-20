// test file to include things in and use them a bit to see if I've fucked
// anything up, as I might change these libraries a fair amount and I need
// a quick way to check "hey does this still like, compile?"
#include<stdio.h>
#include<stdbool.h>

#define ARGPARSE_INCLUDE_IMPLEMENTATION
#include"argparse.h"

typedef struct {
	bool quiet;
	bool help;

	int pinco;
	unsigned int pallino;
	size_t pippo_baudo;

	float la_donna;
	double e_mobile;

	char* li_mortacci;
	FILE* tua;
} config;

void config_set_default(config* c) {
	c->quiet = false;
	c->help = false;

	c->pinco = 0;
	c->pallino = 1;
	c->pippo_baudo = 2;

	c->la_donna = 3.0;
	c->e_mobile = 4.0;

	c->li_mortacci = "";
	c->tua = stdout;
}

const char* render_file(const FILE* file) {
	if(file == stdout) return "stdout";
	if(file == stderr) return "stderr";
	if(file == stdin) return "stdin";
	return "idfk man";
}

void dump_config(const config* c) {
	printf("quiet %s\n"
		   "help %s\n"
		   "pinco %d\n"
		   "pallino %u\n"
		   "pippo baudo %zu\n"
		   "la donna %f\n"
		   "e mobile %f\n"
		   "li mortacci \"%s\"\n"
		   "tua %s\n",
		   c->quiet?"true":"false",
		   c->help?"true":"false",
		   c->pinco,
		   c->pallino,
		   c->pippo_baudo,
		   c->la_donna,
		   (float)c->e_mobile,
		   c->li_mortacci,
		   render_file(c->tua));
}

void test_argparse(int argc, char** argv, config*c) {
	arg_specs* specs = argparse_create_empty_specs();

	argparse_bind_flag(specs, "--quiet", &(c->quiet), "");
	argparse_bind_flag(specs, "--help" , &(c->help),  "");

	argparse_bind_int(specs		, "--pinco"      , &(c->pinco), "");
	argparse_bind_uint(specs	, "--pallino"    , &(c->pallino), "");
	argparse_bind_size_t(specs	, "--pippo-baudo", &(c->pippo_baudo), "");

	argparse_bind_float(specs,  "--la-donna", &(c->la_donna), "");
	argparse_bind_double(specs, "--e-mobile", &(c->e_mobile), "");

	argparse_bind_string(specs,  "--li-mortacci", &(c->li_mortacci), "");
	argparse_bind_fileptr(specs, "--tua",         &(c->tua), "");

	argparse_assign_values(specs, argc, argv);

	argparse_free(specs);
}

int main(int argc, char** argv) {
	// this is tested by running this thing a bunch of times and
	// seeing what the output is 
	// not ideal, but it works for now
	puts("hey, it compiles at least!");
	config c = {0};
	config_set_default(&c);
	test_argparse(argc, argv, &c);
	dump_config(&c);
	return 0;
}
