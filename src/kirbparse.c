// kirbparse.c
// Implementations for library

#include "kirbparse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int kirbparse_debug = 0;
int kirbparse_werror = 0;
FILE *kirbparse_info = NULL;
FILE *kirbparse_err = NULL;

// File-scope constants
static const char dash_string[] = "-";

// File-scope helper functions
static int crossover_check(int argc, char **argv, char opt, char *long_opt);
static int match_short(int num_opts, char *opts, char *arg);
static int match_long(int num_long_opts, char **long_opts, char *arg);

int Kirb_parse_all(int argc, char **argv,
                   int num_flags, char *flags, char **flags_long, int infer, int allow_crossover,
                   int num_value_opts, char *value_opts, char **value_opts_long,
                   int *flags_out, char **values_out, int *num_anon, char ***anon_out)
{
    int prep_ret, mark_ret;
    enum Mark marks[argc * sizeof(enum Mark)];
    int anon_fronts = 0;

    // Preliminary checks and init
    for(int i = 0; i < num_flags; ++i)
    {
        *(flags_out + i) = 0;
    }
    for(int i = 0; i < num_value_opts; ++i)
    {
        *(values_out + i) = NULL;
    }
    if(kirbparse_info == NULL)
    {
        fprintf(kirbparse_err == NULL ? stderr : kirbparse_err,
                "KIRBPARSE ERROR: information logging file not specified. "
                        "If this is intended, set kirbparse_info to stdout.\n");
        return -1;
    }
    if(kirbparse_err == NULL)
    {
        if(kirbparse_werror)
        {
            fprintf(stderr, "ERROR: KIRBPARSE: error logging file not specified. "
                            "If this is intended, set kirbparse_err to stderr.\n");
            return -1;
        }

        fprintf(kirbparse_info, "WARN: KIRBPARSE: error logging file not specified. "
                                "Will automatically be set to stderr.\n");
        kirbparse_err = stderr;
    }
    if(*anon_out != NULL)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_err, "KIRBPARSE: ERROR: Parse Error: Non-NULL pointer at *anon_out\n");
    }

    // Prep args
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: Beginning Prep phase\n");
    prep_ret = Kirb_prep(argc, argv,
                         num_flags, flags, flags_long,
                         num_value_opts, value_opts, value_opts_long,
                         infer, allow_crossover);
    if(prep_ret == -1)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_err, "ERROR: KIRBPARSE: Prep Error: Unallocated inference pointers\n");
        return -1;
    }
    else if(prep_ret == 1)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_err, "ERROR: KIRBPARSE: Prep Error: Duplicate or crossover found\n");
        return 1;
    }
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: End Prep phase\n");

    // Mark args
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: Begin Mark phase\n");
    mark_ret = Kirb_mark(argc, argv, num_value_opts, value_opts, value_opts_long, marks);
    if(mark_ret == -1)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_err, "ERROR: KIRBPARSE: Mark Error: Uninitialized mark pointer\n");
        return -1;
    }
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: End Mark phase\n");

    // Parse args
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: Begin Parse phase\n");
    // Count leading anonymous vals
    for(int i = 1; i < argc; ++i)
    {
        if(marks[i] == 4)
            ++anon_fronts;
        else if(marks[i] >= 1 && marks[i] <= 3)
            break; // Previous iteration was the last anon front
    }
    // Iterate over the rest of the args to build the lists
    int f, v;
    for(int i = anon_fronts + 1; i < argc; ++i)
    {
        if(marks[i] == 1) // Short option
        {
            f = match_short(num_flags, flags, argv[i]);
            v = match_short(num_value_opts, value_opts, argv[i]);

            if(f > -1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: Found short flag at %s\n", argv[i]);

                // Set flag boolean to true
                *(flags_out + f) = 1;
            }
            else if(v > -1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: Found short value option at %s\n", argv[i]);

                if(marks[i + 1] == 3)
                {
                    // Add value to output list
                    *(values_out + v) = argv[++i]; // add and skip the value

                    if(kirbparse_debug)
                        fprintf(kirbparse_info, "INFO: KIRBPARSE: Added %s to results\n", argv[i]);
                }
                else
                {
                    if(kirbparse_debug)
                        fprintf(kirbparse_err, "ERROR: KIRBPARSE: Parse Error: value option missing value\n");
                    return 1;
                }
            }
        }
        else if(marks[i] == 2) // Long option
        {
            f = match_long(num_flags, flags_long, argv[i]);
            v = match_long(num_value_opts, value_opts_long, argv[i]);

            if(f > -1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: Found long flag at %s\n", argv[i]);

                // Set flag boolean to true
                *(flags_out + f) = 1;
            }
            else if(v > -1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: Found long value option at %s\n", argv[i]);

                if(marks[i + 1] == 3)
                {
                    // Add value to output list
                    *(values_out + v) = argv[++i]; // add and skip the value

                    if(kirbparse_debug)
                        fprintf(kirbparse_info, "INFO: KIRBPARSE: Added %s to value option results\n", argv[i]);
                }
                else
                {
                    if(kirbparse_debug)
                        fprintf(kirbparse_err, "ERROR: KIRBPARSE: Parse Error: value option missing value\n");
                    return 1;
                }
            }
        }
    }
    // Pick up all the anonymous values
    *anon_out = malloc(mark_ret * sizeof(char*));
    *num_anon = mark_ret;
    int place_in_anon = 0;
    for(int i = 1; i < argc; ++i)
    {
        if(marks[i] == 4) // An anonymous value
        {
            *(anon_out + place_in_anon) = &argv[i]; // Reference in the output
            ++place_in_anon;

            if(kirbparse_debug)
                fprintf(kirbparse_info, "INFO: KIRBPARSE: Found anonymous value %s\n", argv[i]);
        }
    }
    if(kirbparse_debug)
        fprintf(kirbparse_info, "INFO: KIRBPARSE: End Parse phase\n");

    return 0;
}

int Kirb_prep(int argc, char **argv,
              int num_flags, char *flags, char **flags_long,
              int num_value_opts, char *value_opts, char **value_opts_long,
              int infer, int allow_crossover)
{
    if(infer == 1) // Generate short options based on the first character of the corresponding long option
    {
        if(flags == NULL || value_opts == NULL)
            return -1; // can't modify anything

        if(kirbparse_debug)
            fprintf(kirbparse_info, "INFO: KIRBPARSE: Beginning inference\n");
        for (int i = 0; i < num_flags; ++i)
        {
            if(kirbparse_debug)
                fprintf(kirbparse_info, "inferring %c from %s\n", flags_long[i][0], flags_long[i]);
            *(flags + i) = flags_long[i][0];

        }

        for (int i = 0; i < num_value_opts; ++i)
        {
            if(kirbparse_debug)
                fprintf(kirbparse_info, "inferring %c from %s\n", value_opts_long[i][0], value_opts_long[i]);
            *(value_opts + i) = value_opts_long[i][0];
        }

        if(kirbparse_debug)
            fprintf(kirbparse_info, "INFO: KIRBPARSE: End inference\n");
    }

    if(allow_crossover == 0)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_info, "INFO: KIRBPARSE: Begin crossover\n");
        // Check flag crossover
        // Separate from value option crossover because duplicate flags are less worrisome than duplicate value options
        int ret;
        for(int i = 0; i < num_flags; ++i)
        {
            ret = crossover_check(argc, argv, flags[i], flags_long[i]);

            // If crossover, return early with the error
            if(ret == 1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_err, "ERROR: KIRBPARSE: crossover found\n");
                return ret;
            }
            // If duplicate, maybe return early
            else if(ret > 1)
            {
                if(kirbparse_werror)
                {
                    fprintf(kirbparse_info, "ERROR: KIRBPARSE: duplicate flag found\n");
                    return 1;
                }
                if(kirbparse_debug)
                {
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: duplicate flag found\n");
                    return 2;
                }
            }
            else
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: No crossover or duplicates for flag option %c/%s\n", flags[i], flags_long[i]);
            }
        }

        // Check value option crossover
        for(int i = 0; i < num_value_opts; ++i)
        {
            ret = crossover_check(argc, argv, value_opts[i], value_opts_long[i]);

            // If crossover, return early with the error
            if(ret == 1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_err, "ERROR: KIRBPARSE: crossover found\n");
                return ret;
            }
            // If duplicate, return early with the error
            else if(ret > 1)
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_err, "ERROR: KIRBPARSE: duplicate value option found\n");
                return 1;
            }
            else
            {
                if(kirbparse_debug)
                    fprintf(kirbparse_info, "INFO: KIRBPARSE: No crossover or duplicates for value option %c/%s\n", value_opts[i], value_opts_long[i]);
            }
        }

        if(kirbparse_debug)
            fprintf(kirbparse_info, "INFO: KIRBPARSE: End crossover\n");
    }

    return 0;
}

int Kirb_mark(int argc, char **argv,
              int num_value_opts, char *value_opts, char **value_opts_long,
              enum Mark *marks)
{
    int num_anon = 0;

    if(marks == NULL)
    {
        if(kirbparse_debug)
            fprintf(kirbparse_err, "KIRBPARSE: ERROR: Mark phase array is uninitialized\n");
        return -1;
    }
    marks[0] = PROGRAM; // Will always be a program name

    // Sequentially mark the arguments
    for(int i = 1; i < argc; ++i)
    {
        // If it starts with a -, it's an option
        if(argv[i][0] == '-')
        {
            // If it has a second dash, it's a long option
            if(argv[i][1] == '-')
                *(marks + i) = OPTION_LONG;
            else
                *(marks + i) = OPTION_SHORT;
        }
        // If not preceded by a value option, it's anonymous
        else if(match_long(num_value_opts, value_opts_long, argv[i - 1]) == -1 &&
                match_short(num_value_opts, value_opts, argv[i - 1]) == -1)
        {
            ++num_anon;
            *(marks + i) = ANONYMOUS;
        }
        else
            *(marks + i) = VALUE;
    }
    return num_anon;
}

// Look for crossover (-v and --verbose both present) and duplicates (-v -v or --verbose --verbose)
static int crossover_check(int argc, char **argv, char opt, char *long_opt)
{
    char opt_str[] = { '-', opt, '\0'};
    char long_str[strlen(long_opt) + 2]; // option plus dash and null char
    strcpy(long_str, dash_string);
    strcat(long_str, long_opt);
    int opt_present = 0, long_present = 0;

    for(int i = 0; i < argc; ++i)
    {
        if(strcmp(opt_str, argv[i]) == 0)
            ++opt_present; // Increment rather than set to find duplicates (and warn accordingly)
        else if(strcmp(long_str, argv[i]) == 0)
            ++long_present;
    }

    if(opt_present && long_present)
        return 1; // ERROR, both a long and a short are used
    if(opt_present > 1 || long_present > 1)
        return 2; // Duplicate present, such as being called with -v -v
    return 0;
}

// Returns index in opts of the matched arg
static int match_short(int num_opts, char *opts, char *arg)
{
    if(strlen(arg) == 2) // arg is just a dash and a character, so a short option
    {
        for(int i = 0; i < num_opts; ++i)
        {
            if(opts[i] == arg[1])
                return i; // Index in opts that matches the arg
        }
    }

    return -1; // Not found, either because it wasn't a short op or because it didn't match any in opts
}

// Returns index in long_opts of the matched arg
static int match_long(int num_long_opts, char **long_opts, char *arg)
{
    char *arg_minus_dash = arg + 1;
    for(int i = 0; i < num_long_opts; ++i)
    {
        if(strcmp(long_opts[i], arg_minus_dash) == 0)
            return i; // Index in long_opts that matched arg
    }

    return -1; // No matches
}
