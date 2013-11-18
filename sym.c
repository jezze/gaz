#include "defs.h"
#include "cgen.h"

static int findglob(char *s)
{

    int i;

    for (i = 0; i < Globs; i++)
    {

        if (Types[i] != TMACRO && Stcls[i] != CMEMBER && *s == *Names[i] && !strcmp(s, Names[i]))
            return i;

    }

    return 0;

}

static int findloc(char *s)
{

    int i;

    for (i = Locs; i < NSYMBOLS; i++)
    {

        if (Stcls[i] != CMEMBER && *s == *Names[i] && !strcmp(s, Names[i]))
            return i;
    }

    return 0;

}

int findsym(char *s)
{

    int y;

    if ((y = findloc(s)) != 0)
        return y;

    return findglob(s);

}

int findstruct(char *s)
{

    int i;

    for (i = Locs; i < NSYMBOLS; i++)
        if (TSTRUCT == Types[i] && *s == *Names[i] && !strcmp(s, Names[i]))
            return i;

    for (i = 0; i < Globs; i++)
        if (TSTRUCT == Types[i] && *s == *Names[i] && !strcmp(s, Names[i]))
            return i;

    return 0;

}

int findmem(int y, char *s)
{

    y++;

    while (y < Globs || y >= Locs && y < NSYMBOLS && CMEMBER == Stcls[y])
    {

        if (*s == *Names[y] && !strcmp(s, Names[y]))
            return y;

        y++;

    }

    return 0;

}

static int newglob(void)
{

    int p;

    if ((p = Globs++) >= Locs)
        fatal("too many global symbols");

    return p;

}

static int newloc(void)
{

    int p;

    if ((p = --Locs) <= Globs)
        fatal("too many local symbols");

    return p;

}

char *galloc(int k)
{

    int p;

    if (Nbot + k >= Ntop)
        fatal("out of space for symbol names");

    p = Nbot;
    Nbot += k;

    return &Nlist[p];

}

char *globname(char *s)
{

    char *p = galloc(strlen(s) + 1);

    strcpy(p, s);

    return p;

}

static char *locname(char *s)
{

    int k = strlen(s) + 1;
    int p;

    if (Nbot + k >= Ntop)
        fatal("out of space for symbol names");

    Ntop -= k;
    p = Ntop;

    strcpy(&Nlist[p], s);

    return &Nlist[p];

}

static void defglob(char *name, int prim, int type, int size, int val, int scls, int init)
{

    if (TCONSTANT == type || TFUNCTION == type)
        return;

    gendata();

    if (CPUBLIC == scls)
        genpublic(name);

    if (init && TARRAY == type)
        return;

    if (TARRAY != type && !(prim & STCMASK))
        genname(name);

    if (prim & STCMASK)
    {

        if (TARRAY == type)
            genbss(gsym(name), objsize(prim, TARRAY, size));
        else
            genbss(gsym(name), objsize(prim, TVARIABLE, size));

    }

    else if (PCHAR == prim)
    {

        if (TARRAY == type)
        {

            genbss(gsym(name), size);

        }
        else
        {

            gendefb(val);
            genalign(1);

        }

    }

    else if (PINT == prim)
    {

        if (TARRAY == type)
            genbss(gsym(name), size*arch_intsize());
        else
            gendefw(val);

    }

    else
    {

        if (TARRAY == type)
            genbss(gsym(name), size*arch_pointersize());
        else
            gendefp(val);

    }

}

static int redeclare(char *name, int oldcls, int newcls)
{

    switch (oldcls)
    {

    case CEXTERN:
        if (newcls != CPUBLIC && newcls != CEXTERN)
            error("extern symbol redeclared static: %s", name);

        return newcls;

    case CPUBLIC:
        if (CEXTERN == newcls)
            return CPUBLIC;

        if (newcls != CPUBLIC)
        {

            error("extern symbol redeclared static: %s", name);

            return CPUBLIC;

        }

        break;

    case CSPROTO:
        if (newcls != CSTATIC && newcls != CSPROTO)
            error("static symbol redeclared extern: %s", name);

        return newcls;

    case CSTATIC:
        if (CSPROTO == newcls)
            return CSTATIC;

        if (newcls != CSTATIC)
        {

            error("static symbol redeclared extern: %s", name);

            return CSTATIC;

        }

        break;

    }

    error("redefined symbol: %s", name);

    return newcls;

}

int addglob(char *name, int prim, int type, int scls, int size, int val, char *mtext, int init)
{

    int y;

    if ((y = findglob(name)) != 0)
    {

        scls = redeclare(name, Stcls[y], scls);

        if (TFUNCTION == Types[y])
            mtext = Mtext[y];

    }

    if (0 == y)
    {

        y = newglob();
        Names[y] = globname(name);

    }

    else if (TFUNCTION == Types[y] || TMACRO == Types[y])
    {

        if (Prims[y] != prim || Types[y] != type)
            error("redefinition does not match prior type: %s", name);

    }

    if (CPUBLIC == scls || CSTATIC == scls)
        defglob(name, prim, type, size, val, scls, init);

    Prims[y] = prim;
    Types[y] = type;
    Stcls[y] = scls;
    Sizes[y] = size;
    Vals[y] = val;
    Mtext[y] = mtext;

    return y;

}

static void defloc(int prim, int type, int size, int val, int init)
{

    gendata();

    if (type != TARRAY && !(prim &STCMASK))
        genlab(val);

    if (prim & STCMASK)
    {

        if (TARRAY == type)
            genbss(labname(val), objsize(prim, TARRAY, size));
        else
            genbss(labname(val), objsize(prim, TVARIABLE, size));

    }

    else if (PCHAR == prim)
    {

        if (TARRAY == type)
        {

            genbss(labname(val), size);

        }

        else
        {

            gendefb(init);
            genalign(1);

        }

    }

    else if (PINT == prim)
    {

        if (TARRAY == type)
            genbss(labname(val), size*arch_intsize());
        else
            gendefw(init);

    }

    else
    {

        if (TARRAY == type)
            genbss(labname(val), size*arch_pointersize());
        else
            gendefp(init);

    }

}

int addloc(char *name, int prim, int type, int scls, int size, int val, int init)
{

    int y;

    if (findloc(name))
        error("redefinition of: %s", name);

    y = newloc();

    if (CLSTATC == scls)
        defloc(prim, type, size, val, init);

    Names[y] = locname(name);
    Prims[y] = prim;
    Types[y] = type;
    Stcls[y] = scls;
    Sizes[y] = size;
    Vals[y] = val;

    return y;

}

void clrlocs(void)
{

    Ntop = POOLSIZE;
    Locs = NSYMBOLS;

}

int objsize(int prim, int type, int size)
{

    int k = 0;
    int sp = prim & STCMASK;

    if (PINT == prim)
        k = arch_intsize();
    else if (PCHAR == prim)
        k = CHARSIZE;
    else if (INTPTR == prim || CHARPTR == prim || VOIDPTR == prim)
        k = arch_pointersize();
    else if (INTPP == prim || CHARPP == prim || VOIDPP == prim)
        k = arch_pointersize();
    else if (STCPTR == sp || STCPP == sp)
        k = arch_pointersize();
    else if (UNIPTR == sp || UNIPP == sp)
        k = arch_pointersize();
    else if (PSTRUCT == sp || PUNION == sp)
        k = Sizes[prim & ~STCMASK];
    else if (FUNPTR == prim)
        k = arch_pointersize();

    if (TFUNCTION == type || TCONSTANT == type || TMACRO == type)
        return 0;

    if (TARRAY == type)
        k *= size;

    return k;

}

