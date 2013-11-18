#include "defs.h"

int main(int argc, char *argv[])
{

    Line = 1;
    Putback = '\n';
    Rejected = -1;
    Errors = 0;
    Syntoken = 0;
    Isp = 0;
    Globs = 0;
    Locs = NSYMBOLS;
    Nbot = 0;
    Ntop = POOLSIZE;
    Bsp = 0;
    Csp = 0;
    Q_type = empty;
    Q_cmp = none;
    File = "(stdin)";

    addglob("", 0, 0, 0, 0, 0, NULL, 0);
    addglob("__SUBC__", 0, TMACRO, 0, 0, 0, globname(""), 0);
    genprelude();

    Token = scan();

    while (XEOF != Token)
        top();

    genpostlude();

    return 0;

}

