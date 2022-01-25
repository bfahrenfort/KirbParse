// Basic test file for all KirbParse functionality

#include "kirbparse.h"
#include <stdio.h>
#include <stdlib.h>

// MSVC compiler toolchains don't like a lot of insecure stdlib functions like fopen()
#if defined _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS
#endif // defined _MSC_VER || defined __clang__

int main(int argc, char* argv[])
{
    FILE *file = fopen("debug_log.txt", "w"); // Yes it's insecure but not everyone uses C11 and this is a publicly hosted library

    // Set prelims
    //kirbparse_info = stdout; // Could alternately be redirected to a file
    kirbparse_info = file; // Like this
    kirbparse_err = stderr;
    kirbparse_debug = 1;
    kirbparse_werror = 0;

    // Create inputs
    char flags[3] = "vh";
    char *long_flags[2];
    long_flags[0] = "verbose";
    long_flags[1] = "help";
    char values[2] = "o";
    char *long_values[1];
    long_values[0] = "output\0";
    int flags_results[2];
    char *values_results[1];
    char ***anon_results = malloc(sizeof(char**)); // No way to know how many anons the user will input, dynamic


    // Test Prep phase
    {
        // allocs
        char *blank_values = malloc(3);
        char *blank_flags = malloc(3);
        Kirb_prep(argc, argv,
                  2, blank_flags, long_flags,
                  1, blank_values, long_values,
                  1, 0);
        // Print inferred flags/values
        for (int i = 0; i < 2; ++i)
            printf("inferred flag: %c\n", blank_flags[i]);
        for (int i = 0; i < 1; ++i)
            printf("inferred value option: %c\n", blank_values[i]);
        free(blank_values);
        free(blank_flags);
    }

    // Test Mark phase
    {
        enum Mark marks[7];
        Kirb_mark(argc, argv, 1, values, long_values, marks);
        for (int i = 0; i < argc; ++i)
        {
            printf("marked argument %d: %d\n", i, (int) marks[i]);
        }
    }

    // Test Parsing
    {
        int num_anon;
        int res = Kirb_parse_all(argc, argv,
                                 2, flags, long_flags,
                                 0, 1, values, long_values,
                                 flags_results, values_results, &num_anon, anon_results);
        if(res == 0)
        {
            for(int i = 0; i < 2; ++i)
                printf("resulting flag bool for %c: %d\n", flags[i], flags_results[i]);
            for(int i = 0; i < 1; ++i)
                printf("resulting value for %c: %s\n", values[i], values_results[i]);
            for(int i = 0; i < num_anon; ++i)
                printf("anon value %s found\n", *anon_results[i]);
        }
    }

    fclose(file);
    free(anon_results);
    return 0;
}