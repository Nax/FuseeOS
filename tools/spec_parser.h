#ifndef SPEC_PARSER_H
#define SPEC_PARSER_H

#include <stdio.h>

typedef struct Spec
{
    char type;
    char src[4096];
    char dst[4096];
} Spec;

int parse_spec(Spec* spec, FILE* f);

#endif /* SPEC_PARSER_H */
