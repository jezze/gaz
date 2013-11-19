#include "defs.h"

void copyname(char *name, char *s)
{

    strncpy(name, s, NAMELEN);

    name[NAMELEN] = 0;

}

void suspend(void)
{

}

void resume(void)
{

    clear(1);

}

void match(int t, char *what)
{

    if (Token == t)
    {

        Token = scan();

    }

    else
    {

        error("%s expected", what);

    }

}

void lparen(void)
{

    match(LPAREN, "'('");

}

void rparen(void)
{

    match(RPAREN, "')'");

}

void lbrace(void)
{

    match(LBRACE, "'{'");

}

void rbrace(void)
{

    match(RBRACE, "'}'");

}

void rbrack(void)
{

    match(RBRACK, "']'");

}

void semi(void)
{

    match(SEMI, "';'");

}

void colon(void)
{

    match(COLON, "':'");

}

void ident(void)
{

    match(IDENT, "identifier");

}

int eofcheck(void)
{

    if (XEOF == Token)
    {

        error("missing '}'", NULL);

        return 1;

    }

    return 0;

}

int inttype(int p)
{

    return PINT == p || PCHAR == p;

}

int comptype(int p)
{

    p &= STCMASK;

    return p == PSTRUCT || p == PUNION;

}

void error(char *s, char *a)
{

    if (Syntoken)
        return;

    fprintf(stderr, "error: %s: %d: ", File, Line);
    fprintf(stderr, s, a);
    fprintf(stderr, "\n");

    if (++Errors > 10)
    {

        Errors = 0;
        fatal("too many errors");

    }

}

void fatal(char *s)
{

    error(s, NULL);
    error("fatal error, stop", NULL);
    exit(EXIT_FAILURE);

}

void cerror(char *s, int c)
{

    char buf[32];

    if (isprint(c))
        sprintf(buf, "'%c' (\\x%x)", c, c);
    else
        sprintf(buf, "\\x%x", c);

    error(s, buf);

}

int synch(int syn)
{

    int t = scan();

    while (t != syn)
    {

        if (EOF == t)
            fatal("error recovery failed");

        t = next();

    }

    Syntoken = syn;

    return t;

}

void genraw(char *s)
{

    fprintf(stdout, "%s", s);

}

void gen(char *s)
{

    fprintf(stdout, "\t%s\n", s);

}

void ngen(char *s, char *inst, int n)
{

    fputc('\t', stdout);
    fprintf(stdout, s, inst, n);
    fputc('\n', stdout);

}

void ngen2(char *s, char *inst, int n, int a)
{

    fputc('\t', stdout);
    fprintf(stdout, s, inst, n, a);
    fputc('\n', stdout);

}

void lgen(char *s, char *inst, int n) {
    fputc('\t', stdout);
    fprintf(stdout, s, inst, LPREFIX, n);
    fputc('\n', stdout);
}

void lgen2(char *s, int v1, int v2)
{

    fputc('\t', stdout);
    fprintf(stdout, s, v1, LPREFIX, v2);
    fputc('\n', stdout);

}

void sgen(char *s, char *inst, char *s2)
{

    fputc('\t', stdout);
    fprintf(stdout, s, inst, s2);
    fputc('\n', stdout);

}

void sgen2(char *s, char *inst, int v, char *s2)
{

    fputc('\t', stdout);
    fprintf(stdout, s, inst, v, s2);
    fputc('\n', stdout);

}

void genlab(int id)
{

    fprintf(stdout, "%c%d:\n", LPREFIX, id);

}

