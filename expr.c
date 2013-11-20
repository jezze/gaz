#include "defs.h"

static int precedence[] = {

    7, /* SLASH */
    7, /* STAR */
    7, /* MOD */
    6, /* PLUS */
    6, /* MINUS */
    5, /* LSHIFT */
    5, /* RSHIFT */
    4, /* GREATER */
    4, /* GTEQ */
    4, /* LESS */
    4, /* LTEQ */
    3, /* EQUAL */
    3, /* NOTEQ */
    2, /* AMPER */
    1, /* CARET */
    0, /* PIPE */

};

static int asgmnt(int *lv);
static int cast(int *lv);
static int prefix(int *lv);

/*
 * cfactor :=
 *      INTLIT
 *    | IDENT
 *    | - cfactor
 *    | ~ cfactor
 *    | ( constexpr )
 *
 * cterm :=
 *      cfactor
 *      | cterm * cprefix
 *      | cterm / cprefix
 *      | cterm % cprefix
 *
 * csum :=
 *      cterm
 *      | csum + cterm
 *      | csum - cterm
 *
 * cshift :=
 *      csum
 *      | cshift << csum
 *      | cshift >> csum
 *
 * crelation :=
 *        cshift
 *        | crelation < cshift
 *        | crelation > cshift
 *        | crelation <= cshift
 *        | crelation >= cshift
 *
 * cequation :=
 *        crelation
 *        | cequation == crelation
 *        | cequation != crelation
 *
 * cbinand :=
 *        cequation
 *        | cbinand & cequation
 *
 * cbinxor :=
 *        cbinand
 *        | cbinxor ^ cbinand
 *
 * cbinor :=
 *        cbinxor
 *        | cbinor ^ cbinxor
 *
 * constexpr :=
 *     cbinor
 */

static int constfac(void)
{

    int y, v = Value;

    if (INTLIT == Token)
    {

        Token = scan();

        return v;

    }

    if (MINUS == Token)
    {

        Token = scan();

        return -constfac();

    }

    if (TILDE == Token)
    {

        Token = scan();

        return ~constfac();

    }

    if (LPAREN == Token)
    {

        Token = scan();
        v = constexpr();

        rparen();

        return v;

    }

    if (Token == IDENT)
    {

        y = findsym(Text);

        if (!y || symbols[y].type != TCONSTANT)
            error("not a constant: %s", Text);

        Token = scan();

        return y ? symbols[y].value : 0;

    }

    else
    {

        error("constant expression expected at: %s", Text);

        Token = scan();

        return 1;

    }

}

static int constop(int op, int v1, int v2)
{

    if ((SLASH == op || MOD == op) && 0 == v2)
    {

        error("constant divide by zero", NULL);

        return 0;

    }

    switch (op)
    {

    case SLASH:
        v1 /= v2;

        break;

    case STAR:
        v1 *= v2;

        break;

    case MOD:
        v1 %= v2;

        break;

    case PLUS:
        v1 += v2;

        break;

    case MINUS:
        v1 -= v2;

        break;

    case LSHIFT:
        v1 <<= v2;

        break;

    case RSHIFT:
        v1 >>= v2;

        break;

    case GREATER:
        v1 = v1 > v2;

        break;

    case GTEQ:
        v1 = v1 >= v2;

        break;

    case LESS:
        v1 = v1 < v2;

        break;

    case LTEQ:
        v1 = v1 <= v2;

        break;

    case EQUAL:
        v1 = v1 == v2;

        break;

    case NOTEQ:
        v1 = v1 != v2;

        break;

    case AMPER:
        v1 &= v2;

        break;

    case CARET:
        v1 ^= v2;

        break;

    case PIPE:
        v1 |= v2;

        break;

    }

    return v1;

}

int constexpr(void)
{

    int v, ops[9], vals[10], sp = 0;

    vals[0] = constfac();

    while (SLASH == Token || STAR == Token || MOD == Token || PLUS == Token || MINUS == Token || LSHIFT == Token || RSHIFT == Token || GREATER == Token || GTEQ == Token || LESS == Token || LTEQ == Token || EQUAL == Token || NOTEQ == Token || AMPER == Token || CARET == Token || PIPE == Token)
    {

        while (sp > 0 && precedence[Token] <= precedence[ops[sp - 1]])
        {

            v = constop(ops[sp - 1], vals[sp - 1], vals[sp]);
            vals[--sp] = v;

        }

        ops[sp++] = Token;
        Token = scan();
        vals[sp] = constfac();

    }

    while (sp > 0)
    {

        v = constop(ops[sp - 1], vals[sp - 1], vals[sp]);
        vals[--sp] = v;

    }

    return vals[0];

}

/*
 * primary :=
 *      IDENT
 *    | INTLIT
 *    | string
 *    | ( expr )
 *
 * string :=
 *      STRLIT
 *    | STRLIT string
 */

static int primary(int *lv)
{

    int a, y, lab, k;
    char name[NAMELEN + 1];

    lv[LVSYM] = lv[LVPRIM] = 0;

    switch (Token)
    {

    case IDENT:
        y = findsym(Text);

        copyname(name, Text);

        Token = scan();

        if (!y)
        {

            if (LPAREN == Token)
            {

                y = addglob(name, PINT, TFUNCTION, CEXTERN, -1, 0, NULL, 0);

            }

            else
            {

                error("undeclared variable: %s", name);

                y = addloc(name, PINT, TVARIABLE, CAUTO, 0, 0, 0);

            }

        }

        lv[LVSYM] = y;
        lv[LVPRIM] = symbols[y].prims;

        if (TFUNCTION == symbols[y].type)
        {

            if (LPAREN != Token)
            {

                lv[LVPRIM] = FUNPTR;

                genaddr(y);

            }

            return 0;

        }

        if (TCONSTANT == symbols[y].type)
        {

            genlit(symbols[y].value);

            return 0;

        }

        if (TARRAY == symbols[y].type)
        {

            genaddr(y);

            lv[LVPRIM] = pointerto(lv[LVPRIM]);

            return 0;

        }

        if (comptype(symbols[y].prims))
        {

            genaddr(y);

            lv[LVSYM] = 0;

            return 0;

        }

        return 1;

    case INTLIT:
        genlit(Value);

        Token = scan();
        lv[LVPRIM] = PINT;

        return 0;

    case STRLIT:
        gendata();

        lab = label();

        genlab(lab);

        k = 0;

        while (STRLIT == Token)
        {

            gendefs(Text, Value);

            k += Value - 2;

            Token = scan();

        }

        gendefb(0);
        genalign(k + 1);
        genldlab(lab);

        lv[LVPRIM] = CHARPTR;

        return 0;

    case LPAREN:
        Token = scan();
        a = expr(lv);

        rparen();

        return a;

    default:
        error("syntax error at: %s", Text);

        Token = synch(SEMI);

        return 0;

    }

}

int typematch(int p1, int p2)
{

    if (p1 == p2)
        return 1;

    if (inttype(p1) && inttype(p2))
        return 1;

    if (!inttype(p1) && VOIDPTR == p2)
        return 1;

    if (VOIDPTR == p1 && !inttype(p2))
        return 1;

    return 0;

}

/*
 * fnargs :=
 *      asgmnt
 *    | asgmnt , fnargs
 */

static int fnargs(int fn)
{

    int lv[LV];
    int na = 0;
    int *types;
    char msg[100];
    int sgn[MAXFNARGS + 1];

    types = (int *) (fn ? symbols[fn].mtext: NULL);

    na = 0;

    while (RPAREN != Token)
    {

        if (asgmnt(lv))
            rvalue(lv);

        if (comptype(lv[LVPRIM]))
        {

            error("struct/union passed by reference", NULL);

            lv[LVPRIM] = pointerto(lv[LVPRIM]);

        }

        if (types && *types)
        {

            if (!typematch(*types, lv[LVPRIM]))
            {

                sprintf(msg, "wrong type in argument %d of call to: %%s", na + 1);
                error(msg, symbols[fn].name);

            }

            types++;

        }

        if (na < MAXFNARGS)
            sgn[na] = lv[LVPRIM], sgn[na + 1] = 0;

        na++;

        if (COMMA == Token)
            Token = scan();
        else
            break;

    }

    if (fn && TFUNCTION == symbols[fn].type && !symbols[fn].mtext)
    {

        symbols[fn].mtext = galloc((na + 1) * sizeof(int));
        memcpy(symbols[fn].mtext, sgn, (na + 1) * sizeof(int));

    }

    rparen();
    genpushlit(na);

    return na;

}

int deref(int p)
{

    int y;

    switch (p)
    {

    case INTPP:
        return INTPTR;

    case INTPTR:
        return PINT;

    case CHARPP:
        return CHARPTR;

    case CHARPTR:
        return PCHAR;

    case VOIDPP:
        return VOIDPTR;

    case VOIDPTR:
        return PCHAR;

    case FUNPTR:
        return PCHAR;

    }

    y = p & ~STCMASK;

    switch (p & STCMASK)
    {

    case STCPP:
        return STCPTR | y;

    case STCPTR:
        return PSTRUCT | y;

    case UNIPP:
        return UNIPTR | y;

    case UNIPTR:
        return PUNION | y;

    }

    return -1;

}

static int indirection(int a, int *lv)
{

    int p;

    if (a)
        rvalue(lv);

    if (VOIDPTR == lv[LVPRIM])
        error("dereferencing void pointer", NULL);

    if ((p = deref(lv[LVPRIM])) < 0)
    {

        if (lv[LVSYM])
            error("indirection through non-pointer: %s", symbols[lv[LVSYM]].name);
        else
            error("indirection through non-pointer", NULL);

        p = lv[LVPRIM];

    }

    lv[LVPRIM] = p;
    lv[LVSYM] = 0;

    return p;

}

static void badcall(int *lv)
{

    if (lv[LVSYM])
        error("call of non-function: %s", symbols[lv[LVSYM]].name);
    else
        error("call of non-function", NULL);

}

static int argsok(int na, int nf)
{

    return na == nf || nf < 0 && na >= -nf - 1;

}

static int stc_access(int *pprim, int ptr)
{

    int y, p, a = 1;

    p = *pprim & STCMASK;

    if (IDENT != Token)
    {

        Token = scan();

        error("struct/union member name expected after '%s'", ptr ? "->" : ".");

        return PINT;

    }

    y = findmem(*pprim & ~STCMASK, Text);

    if (0 == y)
        error("struct/union has no such member: %s", Text);

    if ((PSTRUCT == p || STCPTR == p) && symbols[y].value)
    {

        genlit(symbols[y].value);
        genadd(PINT, PINT, 1);

    }

    Token = scan();
    p = symbols[y].prims;

    if (TARRAY == symbols[y].type)
    {

        p = pointerto(p);
        a = 0;

    }

    *pprim = p;

    return a;

}

/*
 * postfix :=
 *      primary
 *    | postfix [ expr ]
 *    | postfix ( )
 *    | postfix ( fnargs )
 *    | postfix ++
 *    | postfix --
 *    | postfix . identifier
 *    | postfix -> identifier
 */

static int postfix(int *lv)
{

    int lv2[LV], p, na;
    int a = primary(lv);

    for (;;)
    {

        switch (Token)
        {

        case LBRACK:
            while (LBRACK == Token)
            {

                p = indirection(a, lv);
                Token = scan();

                if (expr(lv2))
                    rvalue(lv2);

                if (PINT != lv2[LVPRIM])
                    error("non-integer subscript", NULL);

                if (PINT == p || INTPTR == p || CHARPTR == p || VOIDPTR == p || STCPTR == (p & STCMASK) || UNIPTR == (p & STCMASK))
                    genscale();

                genadd(PINT, PINT, 1);
                rbrack();
                lv[LVSYM] = 0;
                a = 1;

            }

            break;

        case LPAREN:
            Token = scan();
            na = fnargs(lv[LVSYM]);

            if (lv[LVSYM] && TFUNCTION == symbols[lv[LVSYM]].type)
            {

                if (!argsok(na, symbols[lv[LVSYM]].size))
                    error("wrong number of arguments: %s", symbols[lv[LVSYM]].name);

                gencall(lv[LVSYM]);

            }

            else
            {

                if (lv[LVPRIM] != FUNPTR)
                    badcall(lv);

                clear(0);
                rvalue(lv);
                gencalr();

                lv[LVPRIM] = PINT;

            }

            genstack((na + 1) * arch_intsize());

            a = 0;

            break;

        case INCR:
        case DECR:
            if (a)
            {

                if (INCR == Token)
                    geninc(lv, 1, 0);
                else
                    geninc(lv, 0, 0);

            }

            else
                error("lvalue required before '%s'", Text);

            Token = scan();
            a = 0;

            break;

        case DOT:
            Token = scan();

            if (comptype(lv[LVPRIM]))
                a = stc_access(&lv[LVPRIM], 0);
            else
                error("struct/union expected before '.'", NULL);

            break;

        case ARROW:
            Token = scan();

            if (a)
                rvalue(lv);

            p = lv[LVPRIM] & STCMASK;

            if (p == STCPTR || p == UNIPTR)
                a = stc_access(&lv[LVPRIM], 1);
            else
                error("struct/union pointer expected before '->'", NULL);

            lv[LVSYM] = 0;

            break;

        default:
            return a;

        }

    }

}

void comp_size(void)
{

    int k, y, lv[LV];

    if (CHAR == Token || INT == Token || VOID == Token || STRUCT == Token || UNION == Token)
    {

        switch (Token)
        {

        case CHAR:
            k = CHARSIZE;

            break;

        case INT:
            k = arch_intsize();

            break;

        case STRUCT:
        case UNION:
            k = primtype(Token, NULL);
            k = objsize(k, TVARIABLE, 1);

            break;

        }

        Token = scan();

        if (STAR == Token)
        {

            k = arch_pointersize();
            Token = scan();

            if (STAR == Token)
                Token = scan();

        }

        else if (0 == k)
        {

            error("sizeof(void) is unknown", NULL);

        }

    }

    else
    {

        y = prefix(lv) ? lv[LVSYM] : 0;
        k = y ? objsize(symbols[y].prims, symbols[y].type, symbols[y].size) : objsize(lv[LVPRIM], TVARIABLE, 1);

        if (0 == k)
            error("cannot compute sizeof: %s", Text);

        clear(1);

    }

    genlit(k);

}

/*
 * prefix :=
 *      postfix
 *    | ++ prefix
 *    | -- prefix
 *    | & cast
 *    | * cast
 *    | + cast
 *    | - cast
 *    | ~ cast
 *    | ! cast
 *    | SIZEOF ( type )
 *    | SIZEOF ( type * )
 *    | SIZEOF ( type * * )
 *    | SIZEOF ( IDENT )
 *
 * type :=
 *      INT
 *    | CHAR
 *    | VOID
 *    | STRUCT IDENT
 *    | UNION IDENT
 */

static int prefix(int *lv)
{

    int t, a;

    switch (Token)
    {

    case INCR:
    case DECR:
        t = Token;
        Token = scan();

        if (prefix(lv))
        {

            if (INCR == t)
                geninc(lv, 1, 1);
            else
                geninc(lv, 0, 1);

        }

        else
        {

            error("lvalue expected after '%s'", t == INCR ? "++" : "--");

        }

        return 0;

    case STAR:
        Token = scan();
        a = cast(lv);

        indirection(a, lv);

        return 1;

    case PLUS:
        Token = scan();

         if (cast(lv))
            rvalue(lv);

        if (!inttype(lv[LVPRIM]))
            error("bad operand to unary '+'", NULL);

        return 0;

    case MINUS:
        Token = scan();

        if (cast(lv))
            rvalue(lv);

        if (!inttype(lv[LVPRIM]))
            error("bad operand to unary '-'", NULL);

        genneg();

        return 0;

    case TILDE:
        Token = scan();

        if (cast(lv))
            rvalue(lv);

        if (!inttype(lv[LVPRIM]))
            error("bad operand to '~'", NULL);

        gennot();

        return 0;

    case XMARK:
        Token = scan();

        if (cast(lv))
            rvalue(lv);

        genlognot();

        lv[LVPRIM] = PINT;

        return 0;

    case AMPER:
        Token = scan();

        if (cast(lv))
        {

            if (lv[LVSYM])
                genaddr(lv[LVSYM]);

        }

        else if ((0 == lv[LVSYM] || symbols[lv[LVSYM]].type != TARRAY) && !comptype(lv[LVPRIM]))
        {

            error("location expected after unary '&'", NULL);

        }

        lv[LVPRIM] = pointerto(lv[LVPRIM]);

        return 0;

    case SIZEOF:
        Token = scan();

        lparen();
        comp_size();
        rparen();

        lv[LVPRIM] = PINT;

        return 0;

    default:
        return postfix(lv);

    }

}

/*
 * cast :=
 *      prefix
 *    | ( type ) prefix
 *    | ( type * ) prefix
 *    | ( type * * ) prefix
 *    | ( INT ( * ) ( ) ) prefix
 */

static int cast(int *lv)
{

    int t, a;

    if (LPAREN == Token)
    {

        Token = scan();

        if (INT == Token || CHAR == Token || VOID == Token || STRUCT == Token || UNION == Token)
        {

            t = primtype(Token, NULL);
            Token = scan();

        }

        else
        {

            reject();

            Token = LPAREN;

            strcpy(Text, "(");

            return prefix(lv);

        }

        if (PINT == t && LPAREN == Token)
        {

            Token = scan();

            match(STAR, "int(*)()");
            rparen();
            lparen();
            rparen();

            t = FUNPTR;

        }

        else if (STAR == Token)
        {

            t = pointerto(t);
            Token = scan();

            if (STAR == Token)
            {

                t = pointerto(t);
                Token = scan();

            }

        }

        rparen();

        a = prefix(lv);
        lv[LVPRIM] = t;

        return a;

    }

    else
    {

        return prefix(lv);

    }

}

/*
 * term :=
 *      cast
 *    | term * cast
 *    | term / cast
 *    | term % cast
 *
 * sum :=
 *      term
 *    | sum + term
 *    | sum - term
 *
 * shift :=
 *      sum
 *    | shift << sum
 *    | shift >> sum
 *
 * relation :=
 *      shift
 *    | relation < shift
 *    | relation > shift
 *    | relation <= shift
 *    | relation >= shift
 *
 * equation :=
 *      relation
 *    | equation == relation
 *    | equation != relation
 *
 * binand :=
 *      equation
 *    | binand & equation
 *
 * binxor :=
 *      binand
 *    | binxor ^ binand
 *
 * binor :=
 *      binxor
 *    | binor '|' binxor
 *
 * binexpr :=
 *      binor
 */

static int binexpr(int *lv)
{

    int ops[9], prs[10], sp = 0;
    int a2 = 0, lv2[LV];
    int a = cast(lv);

    prs[0] = lv[LVPRIM];

    while (SLASH == Token || STAR == Token || MOD == Token || PLUS == Token || MINUS == Token || LSHIFT == Token || RSHIFT == Token || GREATER == Token || GTEQ == Token || LESS == Token || LTEQ == Token || EQUAL == Token || NOTEQ == Token || AMPER == Token || CARET == Token || PIPE == Token)
    {

        if (a)
            rvalue(lv);

        if (a2)
            rvalue(lv2);

        while (sp > 0 && precedence[Token] <= precedence[ops[sp - 1]])
        {

            prs[sp - 1] = genbinop(ops[sp - 1], prs[sp - 1], prs[sp]);
            sp--;

        }

        ops[sp++] = Token;
        Token = scan();
        a2 = cast(lv2);
        prs[sp] = lv2[LVPRIM];
        a = 0;

    }

    if (a2)
        rvalue(lv2);

    while (sp > 0)
    {

        prs[sp - 1] = genbinop(ops[sp - 1], prs[sp - 1], prs[sp]);
        sp--;

    }

    lv[LVPRIM] = prs[0];

    return a;

}

/*
 * logand :=
 *      binexpr
 *    | logand && binexpr
 *
 * logor :=
 *      logand
 *    | logor '||' logand
 */

static int cond2(int *lv, int op)
{

    int a, a2 = 0, lv2[LV];
    int lab = 0;

    a = op == LOGOR ? cond2(lv, LOGAND) : binexpr(lv);

    while (Token == op)
    {

        if (!lab)
            lab = label();

        if (a)
            rvalue(lv);

        if (a2)
            rvalue(lv2);

        commit();

        if (op == LOGOR)
            genbrtrue(lab);
        else
            genbrfalse(lab);

        clear(0);

        Token = scan();
        a2 = op == LOGOR ? cond2(lv2, LOGAND): binexpr(lv2);
        a = 0;

    }

    if (lab)
    {

        if (a2)
            rvalue(lv2);

        commit();
        genlab(lab);
        genbool();
        load();

    }

    return a;

}

/*
 * condexpr :=
 *      logor
 *    | logor ? expr : condexpr
 */

static int cond3(int *lv)
{

    int a, lv2[LV], p;
    int l1 = 0, l2 = 0;

    a = cond2(lv, LOGOR);
    p = 0;

    while (QMARK == Token)
    {

        l1 = label();

        if (!l2)
            l2 = label();

        if (a)
            rvalue(lv);

        a = 0;

        genbrfalse(l1);
        clear(0);

        Token = scan();

        if (expr(lv))
            rvalue(lv);

        if (!p)
            p = lv[LVPRIM];

        if (!typematch(p, lv[LVPRIM]))
            error("incompatible types in '?:'", NULL);

        genjump(l2);
        genlab(l1);
        clear(0);
        colon();

        if (cond2(lv2, LOGOR))
            rvalue(lv2);

        if (QMARK != Token)
            if (!typematch(p, lv2[LVPRIM]))
                error("incompatible types in '?:'", NULL);

    }

    if (l2)
    {

        commit();
        genlab(l2);
        load();

    }

    return a;

}

/*
 * asgmnt :=
 *      condexpr
 *    | condexpr = asgmnt
 *    | condexpr *= asgmnt
 *    | condexpr /= asgmnt
 *    | condexpr %= asgmnt
 *    | condexpr += asgmnt
 *    | condexpr -= asgmnt
 *    | condexpr <<= asgmnt
 *    | condexpr >>= asgmnt
 *    | condexpr &= asgmnt
 *    | condexpr ^= asgmnt
 *    | condexpr |= asgmnt
 */

static int asgmnt(int *lv)
{

    int lv2[LV], op;
    int a = cond3(lv);

    if (ASSIGN == Token || ASDIV == Token || ASMUL == Token || ASMOD == Token || ASPLUS == Token || ASMINUS == Token || ASLSHIFT == Token || ASRSHIFT == Token || ASAND == Token || ASXOR == Token || ASOR == Token)
    {

        op = Token;
        Token = scan();

        if (ASSIGN != op && !lv[LVSYM])
        {

            genpush();
            genind(lv[LVPRIM]);

        }

        if (asgmnt(lv2))
            rvalue(lv2);

        if (ASSIGN == op)
            if (!typematch(lv[LVPRIM], lv2[LVPRIM]))
                error("assignment from incompatible type", NULL);

        if (a)
            genstore(op, lv, lv2);
        else
            error("lvalue expected in assignment", Text);

        a = 0;

    }

    return a;

}

/*
 * expr :=
 *      asgmnt
 *    | asgmnt , expr
 */

int expr(int *lv)
{

    int a2 = 0, lv2[LV];
    int a = asgmnt(lv);

    while (COMMA == Token)
    {

        Token = scan();

        clear(0);

        a2 = asgmnt(lv2);
        a = 0;

    }

    if (a2)
        rvalue(lv2);

    return a;

}

int rexpr(int com)
{

    int lv[LV];

    if (expr(lv))
        rvalue(lv);

    if (com)
        commit();

    return lv[LVPRIM];

}

