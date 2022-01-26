# KirbParse
A speedy, lightweight, cross platform argument parser. 

I got sick of writing really long `getopt` loops for every command line utility and wanted something I could just provide rules to and get results for in a single function call, no messing with complicated macros or building entire command structures.

## Usage
Download and link against the libraries in the 'Releases' tab or compile from source with `cmake`.  

It's pretty small, so statically linking isn't going to increase your binary size much (which was one of my goals). 

## How it Works
KirbParse requires at a minimum your command line arguments, two lists of long-form options (one for boolean flags and one for value-returning options), a few customization booleans and file pointers for where to put your status messages, and two places to put the results (`int*` and `char***` respectively). Whatever index your option was in its option list will be the index of its result. Null pointers are placed in the value-returning list for value options that were not present in the arguments. See documentation (WIP) for calling conventions.

Status codes are raised (and if `kirbparse_debug` is set, status messages are printed) if an undefined option is encountered, a flag is given a value, a value option has no trailing values, and more. See documentation (WIP) for a full list.

KirbParse runs in three phases:

### Preparation
If desired, infer short form arguments from long-form arguments (say, `-v` from `--verbose`).

If desired, check for duplication (`-v -v`, etc) and long-short collision (`-v` and `--verbose` both in the argument string). Note that this doesn't necessarily rely on inference: if you really want to, you could specify a different long-form that can collide with a short-form by putting them in the same positions in your rules lists (`-v` as short-form rule #1 and `--help` as long-form rule #1 would collide). 

### Marking
Iterate over the argument list. 
* The first element is marked as the program name
* Anything starting with a - is marked as an option
* -- denotes a long-form option
* If not an option and not preceded by a value option, marked as an anonymous value
* All others are marked as values for value-returning options

### Parsing/Extraction
Iterate again.
* compare each option to the rules. 
* if a flag (can exist or not exist) option: set its Boolean in the corresponding output list. 
* if a value option (followed by additional arguments) option: set pointer in output array at relevant index to this value.
* if an anonymous value (not preceded by a value option, see docs), add to anonymous value list
* return. 


## To Be Implemented
* Fix alternate methods in header and implement
* Remove unused code
* Make more efficient

## Notes
All of the phases are exposed, so it's possible to just use one component of my implementation if you need additional constraint checking etc.

Enjoy!
