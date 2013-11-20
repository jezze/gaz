#include "../cc.h"
#include "../cgen.h"

int Acc = 0;

void clear(int q)
{

    q = 0;
    Acc = 0;

}

void load(void)
{

    Acc = 1;

}

int label(void)
{

    static int id = 1;

    return id++;

}

void spill(void)
{

    if (Acc)
        genpush();

}

char *labname(int id)
{

    static char name[100];

    sprintf(name, "%c%d", LPREFIX, id);

    return name;


}

char *gsym(char *s)
{

    static char name[NAMELEN + 2];

    name[0] = PREFIX;
    name[1] = 0;

    strcat(name, s);

    return name;

}

void gendata(void)
{

    if (Textseg)
        cgdata();

    Textseg = 0;

}

void gentext(void)
{

    if (!Textseg)
        cgtext();

    Textseg = 1;

}

void genprelude(void)
{

    Textseg = 0;

    gentext();
    cgprelude();

}

void genpostlude(void)
{

    cgpostlude();

}

void genname(char *name)
{

    genraw(gsym(name));
    genraw(":\n");

}

void genpublic(char *name)
{

    cgpublic(gsym(name));

}

void commit(void)
{

}

void genaddr(int y)
{

    gentext();
    spill();

    if (CAUTO == symbols[y].stcl)
        cgldla(symbols[y].value);
    else if (CLSTATC == symbols[y].stcl)
        cgldsa(symbols[y].value);
    else
        cgldga(gsym(symbols[y].name));

    load();

}

void genldlab(int id)
{

    gentext();
    spill();
    cgldlab(id);
    load();

}

void genlit(int v)
{

    gentext();
    spill();
    cglit(v);
    load();

}

void genand(void)
{

    gentext();
    cgpop2();
    cgand();

}

void genior(void)
{

    gentext();
    cgpop2();
    cgior();

}

void genxor(void)
{

    gentext();
    cgpop2();
    cgxor();

}

void genshl(int swap)
{

    gentext();
    cgpop2();

    if (swap)
        cgswap();

    cgshl();

}

void genshr(int swap)
{

    gentext();
    cgpop2();

    if (swap)
        cgswap();

    cgshr();

}

static int ptr(int p)
{

    int sp = p & STCMASK;

    return INTPTR == p || INTPP == p || CHARPTR == p || CHARPP == p || VOIDPTR == p || VOIDPP == p || STCPTR == sp || STCPP == sp || UNIPTR == sp || UNIPP == sp || FUNPTR == p;

}

static int needscale(int p)
{

    int sp = p & STCMASK;

    return INTPTR == p || INTPP == p || CHARPP == p || VOIDPP == p || STCPTR == sp || STCPP == sp || UNIPTR == sp || UNIPP == sp;

}

int genadd(int p1, int p2, int swap)
{

    int rp = PINT, t;

    gentext();

    if (swap)
    {

        t = p1;
        p1 = p2;
        p2 = t;

    }

    if (ptr(p1))
    {

        if (needscale(p1))
        {

            if ((p1 & STCMASK) == STCPTR || (p1 & STCMASK) == UNIPTR)
                cgscaleby(objsize(deref(p1), TVARIABLE, 1));
            else
                cgscale();

        }

        cgpop2();

        rp = p1;

    }

    else if (ptr(p2))
    {

        cgpop2();

        if (needscale(p2))
        {

            if ((p2 & STCMASK) == STCPTR || (p2 & STCMASK) == UNIPTR)
                cgscale2by(objsize(deref(p2), TVARIABLE, 1));
            else
                cgscale2();

        }

        rp = p2;

    }

    else
    {

        cgpop2();

    }

    cgadd();

    return rp;

}

int gensub(int p1, int p2, int swap)
{

    int rp = PINT;

    gentext();
    cgpop2();

    if (swap)
        cgswap();

    if (!inttype(p1) && !inttype(p2) && p1 != p2)
        error("incompatible pointer types in binary '-'", NULL);

    if (ptr(p1) && !ptr(p2))
    {

        if (needscale(p1))
        {

            if ((p1 & STCMASK) == STCPTR || (p1 & STCMASK) == UNIPTR)
                cgscale2by(objsize(deref(p1), TVARIABLE, 1));
            else
                cgscale2();

        }

        rp = p1;

    }

    cgsub();

    if (needscale(p1) && needscale(p2))
    {

        if ((p1 & STCMASK) == STCPTR || (p1 & STCMASK) == UNIPTR)
            cgunscaleby(objsize(deref(p1), TVARIABLE, 1));
        else
            cgunscale();

    }

    return rp;

}

void genmul(void)
{

    gentext();
    cgpop2();
    cgmul();

}

void gendiv(int swap)
{

    gentext();
    cgpop2();

    if (swap)
        cgswap();

    cgdiv();

}

void genmod(int swap)
{

    gentext();
    cgpop2();

    if (swap)
        cgswap();

    cgmod();

}

static void binopchk(int op, int p1, int p2)
{

    if (ASPLUS == op)
        op = PLUS;
    else if (ASMINUS == op)
        op = MINUS;

    if (inttype(p1) && inttype(p2))
        return;
    else if (comptype(p1) || comptype(p2));
    else if (PLUS == op && (inttype(p1) || inttype(p2)))
        return;
    else if (MINUS == op && (!inttype(p1) || inttype(p2)))
        return;
    else if ((EQUAL == op || NOTEQ == op || LESS == op || GREATER == op || LTEQ == op || GTEQ == op) && (p1 == p2 || VOIDPTR == p1 && !inttype(p2) || VOIDPTR == p2 && !inttype(p1)))
        return;

    error("invalid operands to binary operator", NULL);

}

int genbinop(int op, int p1, int p2)
{

    binopchk(op, p1, p2);

    switch (op)
    {

    case PLUS:
        return genadd(p1, p2, 0);

    case MINUS:
        return gensub(p1, p2, 1);

    case STAR:
        genmul();

        break;

    case SLASH:
        gendiv(1);

        break;

    case MOD:
        genmod(1);

        break;

    case LSHIFT:
        genshl(1);

        break;

    case RSHIFT:
        genshr(1);

        break;

    case AMPER:
        genand();

        break;

    case CARET:
        genxor();

        break;

    case PIPE:
        genior();

        break;

    case EQUAL:
        cgeq();

        break;

    case NOTEQ:
        cgne();

        break;

    case LESS:
        cglt();

        break;

    case GREATER:
        cggt();

        break;

    case LTEQ:
        cgle();

        break;

    case GTEQ:
        cgge();

        break;

    }

    return PINT;

}

void genbool(void)
{

    gentext();
    cgbool();

}

void genind(int p)
{

    gentext();

    if (PCHAR == p)
        cgindb();
    else
        cgindw();

}

void genlognot(void)
{

    gentext();
    cglognot();

}

void genneg(void)
{

    gentext();
    cgneg();

}

void gennot(void)
{

    gentext();
    cgnot();

}

void genscale(void)
{

    gentext();
    cgscale();

}

void genscale2(void)
{

    gentext();
    cgscale2();

}

void genjump(int dest)
{

    gentext();
    cgjump(dest);

}

void genbrfalse(int dest)
{

    gentext();
    cgbrfalse(dest);

}

void genbrtrue(int dest)
{

    gentext();
    cgbrtrue(dest);

}

void gencall(int y)
{

    gentext();
    cgcall(gsym(symbols[y].name));
    load();

}

void gencalr(void)
{

    gentext();
    cgcalr();
    load();

}

void genentry(void)
{

    gentext();
    cgentry();

}

void genexit(void)
{

    gentext();
    cgexit();

}

void genpush(void)
{

    gentext();
    cgpush();

}

void genpushlit(int n)
{

    gentext();
    spill();
    cgpushlit(n);

}

void genstack(int n)
{

    if (n)
    {

        gentext();
        cgstack(n);

    }

}

void genlocinit(void)
{

    int i;

    gentext();

    for (i = 0; i < Nli; i++)
        cginitlw(LIval[i], LIaddr[i]);

}

void genbss(char *name, int len)
{

    gendata();
    cgbss(name, (len + arch_intsize() - 1) / arch_intsize() * arch_intsize());

}

void genalign(int k)
{

    gendata();

    while (k++ % arch_intsize())
        cgdefb(0);

}

void gendefb(int v)
{

    gendata();
    cgdefb(v);

}

void gendefl(int id)
{

    gendata();
    cgdefl(id);

}

void gendefp(int v)
{

    gendata();
    cgdefp(v);

}

void gendefs(char *s, int len)
{

    int i;

    gendata();

    for (i = 1; i < len - 1; i++)
    {

        if (isalnum(s[i]))
            cgdefc(s[i]);
        else
            cgdefb(s[i]);

    }

}

void gendefw(int v)
{

    gendata();
    cgdefw(v);

}

static void genincptr(int *lv, int inc, int pre)
{

    int size = objsize(deref(lv[LVPRIM]), TVARIABLE, 1);
    int y;

    gentext();

    y = lv[LVSYM];

    if (!y && !pre)
        cgldinc();

    if (!pre)
        rvalue(lv);

    if (!y)
    {

        if (pre)
        {

            if (inc)
                cginc1pi(size);
            else
                cgdec1pi(size);

        }

        else
        {

            if (inc)
                cginc2pi(size);
            else
                cgdec2pi(size);

        }

    }

    else if (CAUTO == symbols[y].stcl)
    {

        if (inc)
            cgincpl(symbols[y].value, size);
        else
            cgdecpl(symbols[y].value, size);

    }

    else if (CLSTATC == symbols[y].stcl)
    {

        if (inc)
            cgincps(symbols[y].value, size);
        else
            cgdecps(symbols[y].value, size);

    }

    else
    {

        if (inc)
            cgincpg(gsym(symbols[y].name), size);
        else
            cgdecpg(gsym(symbols[y].name), size);

    }

    if (pre)
        rvalue(lv);

}

void geninc(int *lv, int inc, int pre)
{

    int y, b;

    gentext();

    y = lv[LVSYM];

    if (needscale(lv[LVPRIM]))
    {

        genincptr(lv, inc, pre);

        return;

    }

    b = PCHAR == lv[LVPRIM];

    if (!y && !pre)
        cgldinc();

    if (!pre)
        rvalue(lv);

    if (!y)
    {

        if (pre)
        {

            if (inc)
                b ? cginc1ib() : cginc1iw();
            else
                b ? cgdec1ib() : cgdec1iw();
        }

        else
        {

            if (inc)
                b ? cginc2ib() : cginc2iw();
            else
                b ? cgdec2ib() : cgdec2iw();

        }

    }

    else if (CAUTO == symbols[y].stcl)
    {

        if (inc)
            b ? cginclb(symbols[y].value) : cginclw(symbols[y].value);
        else
            b ? cgdeclb(symbols[y].value) : cgdeclw(symbols[y].value);

    }

    else if (CLSTATC == symbols[y].stcl)
    {

        if (inc)
            b ? cgincsb(symbols[y].value) : cgincsw(symbols[y].value);
        else
            b ? cgdecsb(symbols[y].value) : cgdecsw(symbols[y].value);

    }

    else
    {

        if (inc)
            b ? cgincgb(gsym(symbols[y].name)) : cgincgw(gsym(symbols[y].name));
        else
            b ? cgdecgb(gsym(symbols[y].name)) : cgdecgw(gsym(symbols[y].name));

    }

    if (pre)
        rvalue(lv);

}

void genswitch(int *vals, int *labs, int nc, int dflt)
{

    int i;
    int ltbl = label();

    gentext();
    cgldswtch(ltbl);
    cgcalswtch();
    gendata();
    genlab(ltbl);
    gendefw(nc);

    for (i = 0; i < nc; i++)
        cgcase(vals[i], labs[i]);

    gendefl(dflt);

}

void genasop(int op, int p1, int p2, int swap)
{

    binopchk(op, p1, p2);

    switch (op)
    {

    case ASDIV:
        gendiv(0);

        break;

    case ASMUL:
        genmul();

        break;

    case ASMOD:
        genmod(0);

        break;

    case ASPLUS:
        genadd(p2, p1, swap);

        break;

    case ASMINUS:
        gensub(p1, p2, swap);

        break;

    case ASLSHIFT:
        genshl(0);

        break;

    case ASRSHIFT:
        genshr(0);

        break;

    case ASAND:
        genand();

        break;

    case ASXOR:
        genxor();

        break;

    case ASOR:
        genior();

        break;

    }

}

void genstore(int op, int *lv, int *lv2)
{

    int swap = 1;

    if (NULL == lv)
        return;

    gentext();

    if (ASSIGN != op)
    {

        if (lv[LVSYM])
        {

            rvalue(lv);
            swap = 0;

        }

        genasop(op, lv[LVPRIM], lv2[LVPRIM], swap);

    }

    if (!lv[LVSYM])
    {

        cgpopptr();

        if (PCHAR == lv[LVPRIM])
            cgstorib();
        else
            cgstoriw();

    }

    else if (CAUTO == symbols[lv[LVSYM]].stcl)
    {

        if (PCHAR == lv[LVPRIM])
            cgstorlb(symbols[lv[LVSYM]].value);
        else
            cgstorlw(symbols[lv[LVSYM]].value);

    }

    else if (CLSTATC == symbols[lv[LVSYM]].stcl)
    {

        if (PCHAR == lv[LVPRIM])
            cgstorsb(symbols[lv[LVSYM]].value);
        else
            cgstorsw(symbols[lv[LVSYM]].value);

    }

    else
    {

        if (PCHAR == lv[LVPRIM])
            cgstorgb(gsym(symbols[lv[LVSYM]].name));
        else
            cgstorgw(gsym(symbols[lv[LVSYM]].name));

    }

}

void rvalue(int *lv)
{

    if (NULL == lv)
        return;

    gentext();

    if (!lv[LVSYM])
    {

        genind(lv[LVPRIM]);

    }

    else if (CAUTO == symbols[lv[LVSYM]].stcl)
    {

        spill();

        if (PCHAR == lv[LVPRIM])
        {

            cgclear();
            cgldlb(symbols[lv[LVSYM]].value);

        }

        else
        {

            cgldlw(symbols[lv[LVSYM]].value);

        }

    }

    else if (CLSTATC == symbols[lv[LVSYM]].stcl)
    {

        spill();

        if (PCHAR == lv[LVPRIM])
        {

            cgclear();
            cgldsb(symbols[lv[LVSYM]].value);

        }

        else
        {

            cgldsw(symbols[lv[LVSYM]].value);

        }

    }

    else
    {

        spill();

        if (PCHAR == lv[LVPRIM])
        {

            cgclear();
            cgldgb(gsym(symbols[lv[LVSYM]].name));

        }

        else
        {

            cgldgw(gsym(symbols[lv[LVSYM]].name));

        }

    }

    load();

}

