#include "../cc.h"
#include "../cgen.h"

int Acc = 0;

void clear(int q)
{

    Acc = 0;

    if (q)
        Q_type = empty;

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
    {

        gentext();
        cgpush();

    }

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
    copyname(&name[1], s);

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

static void commit_cmp(void)
{

    switch (Q_cmp)
    {

    case equal:
        cgeq();

        break;

    case not_equal:
        cgne();

        break;

    case less:
        cglt();

        break;

    case greater:
        cggt();

        break;

    case less_equal:
        cgle();

        break;

    case greater_equal:
        cgge();

        break;

    }

    Q_cmp = none;

}

void commit(void)
{

    if (Q_cmp != none)
    {

        commit_cmp();

        return;

    }

    if (empty == Q_type)
        return;

    spill();

    switch (Q_type)
    {

    case addr_auto:
        cgldla(Q_val);

        break;

    case addr_static:
        cgldsa(Q_val);

        break;

    case addr_globl:
        cgldga(gsym(Q_name));

        break;

    case addr_label:
        cgldlab(Q_val);

        break;

    case literal:
        cglit(Q_val);

        break;

    case auto_byte:
        cgclear();
        cgldlb(Q_val);

        break;

    case auto_word:
        cgldlw(Q_val);

        break;

    case static_byte:
        cgclear();
        cgldsb(Q_val);

        break;

    case static_word:
        cgldsw(Q_val);

        break;

    case globl_byte:
        cgclear();
        cgldgb(gsym(Q_name));

        break;

    case globl_word:
        cgldgw(gsym(Q_name));

        break;

    default:
        fatal("internal: unknown Q_type");

    }

    load();

    Q_type = empty;

}

static void queue(int type, int val, char *name)
{

    commit();

    Q_type = type;
    Q_val = val;

    if (name)
        copyname(Q_name, name);

    Q_cmp = none;

}

void genaddr(int y)
{

    gentext();

    if (CAUTO == symbols[y].stcl)
        queue(addr_auto, symbols[y].value, NULL);
    else if (CLSTATC == symbols[y].stcl)
        queue(addr_static, symbols[y].value, NULL);
    else
        queue(addr_globl, 0, symbols[y].name);

}

void genldlab(int id)
{

    gentext();
    queue(addr_label, id, NULL);

}

void genlit(int v)
{

    gentext();
    queue(literal, v, NULL);

}

void genand(void)
{

    gentext();
    cgsynand();

}

void genior(void)
{

    gentext();
    cgsynor();

}

void genxor(void)
{

    gentext();
    cgsynxor();

}

void genshl(int swapped)
{

    gentext();

    if (cgload() || !swapped)
        cgswap();

    cgshl();

}

void genshr(int swapped)
{

    gentext();

    if (cgload() || !swapped)
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

int genadd(int p1, int p2, int swapped)
{

    int rp = PINT, t;

    gentext();

    if (cgload() || !swapped)
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
                cgscale2by(objsize(deref(p1), TVARIABLE, 1));
            else
                cgscale2();

        }

        rp = p1;

    }

    else if (ptr(p2))
    {

        if (needscale(p2))
        {

            if ((p2 & STCMASK) == STCPTR || (p2 & STCMASK) == UNIPTR)
                cgscaleby(objsize(deref(p2), TVARIABLE, 1));
            else
                cgscale();

        }

        rp = p2;

    }

    cgadd();

    return rp;

}

int gensub(int p1, int p2, int swapped)
{

    int rp = PINT;

    gentext();

    if (cgload() || !swapped)
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
    cgload();
    cgmul();

}

void gendiv(int swapped)
{

    gentext();

    if (cgload() || !swapped)
        cgswap();

    cgdiv();

}

void genmod(int swapped)
{

    gentext();

    if (cgload() || !swapped)
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

static void queue_cmp(int op)
{

    Q_cmp = op;

}

int genbinop(int op, int p1, int p2)
{

    binopchk(op, p1, p2);

    switch (op)
    {

    case PLUS:
        return genadd(p1, p2, 1);

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
        queue_cmp(equal);

        break;

    case NOTEQ:
        queue_cmp(not_equal);

        break;

    case LESS:
        queue_cmp(less);

        break;

    case GREATER:
        queue_cmp(greater);

        break;

    case LTEQ:
        queue_cmp(less_equal);

        break;

    case GTEQ:
        queue_cmp(greater_equal);

        break;

    }

    return PINT;

}

void genbool(void)
{

    gentext();
    commit();
    cgbool();

}

void genind(int p)
{

    gentext();
    commit();

    if (PCHAR == p)
        cgindb();
    else
        cgindw();

}

void genlognot(void)
{

    gentext();
    commit();
    cglognot();

}

void genneg(void)
{

    gentext();
    commit();
    cgneg();

}

void gennot(void)
{

    gentext();
    commit();
    cgnot();

}

void genscale(void)
{

    gentext();
    commit();
    cgscale();

}

void genscale2(void)
{

    gentext();
    commit();
    cgscale2();

}

void genjump(int dest)
{

    gentext();
    commit();
    cgjump(dest);

}

void genbranch(int dest, int inv)
{

    if (inv)
    {

        switch (Q_cmp)
        {

        case equal:
            cgbrne(dest);

            break;

        case not_equal:
            cgbreq(dest);

            break;

        case less:
            cgbrge(dest);

            break;

        case greater:
            cgbrle(dest);

            break;

        case less_equal:
            cgbrgt(dest);

            break;

        case greater_equal:
            cgbrlt(dest);

            break;

        }

    }

    else
    {

        switch (Q_cmp)
        {

        case equal:
            cgbreq(dest);

            break;

        case not_equal:
            cgbrne(dest);

            break;

        case less:
            cgbrlt(dest);

            break;

        case greater:
            cgbrgt(dest);

            break;

        case less_equal:
            cgbrle(dest);

            break;

        case greater_equal:
            cgbrge(dest);

            break;

        }

    }

    Q_cmp = none;

}

void genbrfalse(int dest)
{

    gentext();

    if (Q_cmp != none)
    {

        genbranch(dest, 0);

        return;

    }

    commit();
    cgbrfalse(dest);

}

void genbrtrue(int dest)
{

    gentext();

    if (Q_cmp != none)
    {

        genbranch(dest, 1);

        return;

    }

    commit();
    cgbrtrue(dest);

}

void gencall(int y)
{

    gentext();
    commit();
    cgcall(gsym(symbols[y].name));
    load();

}

void gencalr(void)
{

    gentext();
    commit();
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
    commit();
    cgpush();

}

void genpushlit(int n)
{

    gentext();
    commit();
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

    int y;
    int size = objsize(deref(lv[LVPRIM]), TVARIABLE, 1);

    gentext();

    y = lv[LVSYM];

    commit();

    if (!y && !pre)
        cgldinc();

    if (!pre)
    {

        rvalue(lv);
        commit();

    }

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

    commit();

    if (!y && !pre)
        cgldinc();

    if (!pre)
    {

        rvalue(lv);
        commit();

    }

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

void genasop(int op, int p1, int p2, int swapped)
{

    binopchk(op, p1, p2);

    switch (op)
    {

    case ASDIV:
        gendiv(swapped);

        break;

    case ASMUL:
        genmul();

        break;

    case ASMOD:
        genmod(swapped);

        break;

    case ASPLUS:
        genadd(p1, p2, swapped);

        break;

    case ASMINUS:
        gensub(p1, p2, swapped);

        break;

    case ASLSHIFT:
        genshl(swapped);

        break;

    case ASRSHIFT:
        genshr(swapped);

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

    int swapped = 1;

    if (NULL == lv)
        return;

    gentext();

    if (ASSIGN != op)
    {

        if (lv[LVSYM])
        {

            rvalue(lv);

            swapped = 0;

        }

        genasop(op, lv[LVPRIM], lv2[LVPRIM], swapped);

    }

    commit();

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

        if (PCHAR == lv[LVPRIM])
            queue(auto_byte, symbols[lv[LVSYM]].value, NULL);
        else
            queue(auto_word, symbols[lv[LVSYM]].value, NULL);

    }

    else if (CLSTATC == symbols[lv[LVSYM]].stcl)
    {

        if (PCHAR == lv[LVPRIM])
            queue(static_byte, symbols[lv[LVSYM]].value, NULL);
        else
            queue(static_word, symbols[lv[LVSYM]].value, NULL);

    }

    else
    {

        if (PCHAR == lv[LVPRIM])
            queue(globl_byte, 0, symbols[lv[LVSYM]].name);
        else
            queue(globl_word, 0, symbols[lv[LVSYM]].name);

    }

}

