/*
 * ON THE DECISION TO WORK WITHOUT DASHES
 * Arguments in the command line are typically passed with dashes preceding a letter or word.
 * However, this library takes lists of options without dashes and the procedures will go out of their way to
 *  exclude dashes in their comparisons. Why? waste. We know that character will always be
 *  the same, so why waste the computational or space complexity on it?
 *
 * DEFINITIONS
 * An 'option' is a text argument passed to a program via the command line preceded by a - or --
     to denote some desired alternate functionality.
 * A 'value' is a text argument passed to a program via the command line, typically after some option.
 * A 'flag' is a type of option that has no values preceding it.
 * An 'anonymous value' is a value not passed to an option
 *
 * CALLING CONVENTIONS
 * flags: for -v -h pass "vh" (results: boolean array)
 * flags_long: assumed to have the same number of entries as flags!! for --verbose and --help pass ["verbose", "help"]
     (results: boolean array)
 * allow_crossover: if 1, will allow -v and --verbose to be present in the same command, if 0 otherwise will reject this
 * value_opts: for -o test.c, pass "o" (results: "test.c").
 * value_opts_long: assumed to have the same number of entries as value_opts!! for --terminate Ubuntu, pass ["terminate"]
     (results: "Ubuntu")
 * flags_out: boolean array of flag presences. Assumed to be same length as flags!!
 * values_out: array strings. Assumed to be the same length as value_opts!!
 * num_anon: will contain number of returned anonymous values so you can traverse anon_out.
 * anon_out: pointer able to store a value of size sizeof(char**). ex: ./myprogram hello.c -o hello will have num_anon 1 and *anon_out [["hello.c"]]
 *
 * RETURN CODES
 * Special case: my matching helper functions will return an index as opposed to a return code or -1 if not found
 * -1: (error) The programmer did something wrong
 *  1: (error) The user did something wrong (and you probably want to display your usage or help message)
 *  2: (warning) Somebody did something wrong, but it's not worth worrying about
 */

#include <stdio.h>

//#include <stdarg.h>
#ifndef KIRBPARSE_LIBRARY_H
#define KIRBPARSE_LIBRARY_H

// Customization allocated and checked for in the implementation file
extern FILE *kirbparse_info; // Where to print warnings, REQUIRED
extern FILE *kirbparse_err;  // Where to print errors, REQUIRED
extern int kirbparse_debug;  // Write helpful info and error messages to the supplied files alongside returning error codes
extern int kirbparse_werror; // Treat warning codes (say, a flag was included twice) as errors

// Enumeration for marking
enum Mark { PROGRAM = 0, OPTION_SHORT = 1, OPTION_LONG = 2, VALUE = 3, ANONYMOUS = 4 };

// Preparation phase
int Kirb_prep(int argc, char **argv,
              int num_flags, char *flags, char **flags_long,
              int num_value_opts, char *value_opts, char **value_opts_long,
              int infer, int allow_crossover);

// Marking phase
int Kirb_mark(int argc, char **argv, // Args
              int num_value_opts, char *value_opts, char **value_opts_long, // Value opts for anonymity detection
              enum Mark *marks); // Output, marks should be the same length as argv!!

// With all options
int Kirb_parse_all(int argc, char **argv,  // String to parse
                int num_flags, char *flags, char **flags_long, int infer, int allow_crossover, // Flag options
                int num_value_opts, char *value_opts, char **value_opts_long, // Value options, see documentation
                int *flags_out, char **values_out, int *num_anon, char ***anon_out); // Outputs

// With short options only
int Kirb_parse_short(int argc, char **argv,  // String to parse
                int num_flags, char *flags, // Flag options
                int num_value_opts, char **value_opts, // Value options, see documentation
                int *flags_out, char **values_out, int *num_anon, char ***anon_out); // Outputs

// With long options only
// Usually using this with infer=1 is suitable for most applications
// infer: if 1, will automatically generate short flags based on the long flags, otherwise will not check short flags
//   (will fail if there are collisions in long flag first letters), suitable for most cases. Otherwise,
//   will only look for long flags.
int Kirb_parse_long(int argc, char **argv, int infer, // String to parse
                int num_flags, char **flags_long, // Flag options
                int num_value_opts, char **value_opts_long, int anon_front, // Value options, see documentation
                int *flags_out, char **values_out, int *num_anon, char ***anon_out); // Outputs

#endif //KIRBPARSE_LIBRARY_H
