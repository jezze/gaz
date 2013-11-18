#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//CHANGE BPW FOR 32 and 16 BIT
#define BPW                             8

#define PREFIX                          'C'
#define LPREFIX                         'L'
#define INTSIZE                         BPW
#define PTRSIZE                         BPW
#define CHARSIZE                        1
#define TEXTLEN                         512
#define NAMELEN                         16
#define MAXCASE                         256
#define MAXBREAK                        16
#define MAXLOCINIT                      32
#define MAXFNARGS                       32
#define NSYMBOLS                        1024
#define POOLSIZE                        16384

enum
{

    TVARIABLE = 1,
    TARRAY,
    TFUNCTION,
    TCONSTANT,
    TMACRO,
    TSTRUCT

};

enum
{

    PCHAR = 1,
    PINT,
    CHARPTR,
    INTPTR,
    CHARPP,
    INTPP,
    PVOID,
    VOIDPTR,
    VOIDPP,
    FUNPTR,
    PSTRUCT = 0x2000,
    PUNION  = 0x4000,
    STCPTR  = 0x6000,
    STCPP   = 0x8000,
    UNIPTR  = 0xA000,
    UNIPP   = 0xC000,
    STCMASK = 0xE000

};

enum
{

    CPUBLIC = 1,
    CEXTERN,
    CSTATIC,
    CLSTATC,
    CAUTO,
    CSPROTO,
    CMEMBER,
    CSTCDEF

};

enum
{

    LVSYM,
    LVPRIM,
    LV

};

enum
{

    empty,
    addr_auto,
    addr_static,
    addr_globl,
    addr_label,
    literal,
    arg_count,
    auto_byte,
    auto_word,
    static_byte,
    static_word,
    globl_byte,
    globl_word

};

enum
{

    none,
    equal,
    not_equal,
    less,
    greater,
    less_equal,
    greater_equal

};

enum
{

    SLASH, STAR, MOD, PLUS, MINUS, LSHIFT, RSHIFT,
    GREATER, GTEQ, LESS, LTEQ, EQUAL, NOTEQ, AMPER,
    CARET, PIPE, LOGAND, LOGOR,
    __ARGC, ARROW, ASAND, ASXOR, ASLSHIFT, ASMINUS, ASMOD, ASOR,
    ASPLUS, ASRSHIFT, ASDIV, ASMUL, ASSIGN, AUTO, BREAK, CASE,
    CHAR, COLON, COMMA, CONTINUE, DECR, DEFAULT, DO, DOT,
    ELLIPSIS, ELSE, ENUM, EXTERN, FOR, IDENT, IF, INCR, INT,
    INTLIT, LBRACE, LBRACK, LPAREN, NOT, QMARK, RBRACE, RBRACK,
    REGISTER, RETURN, RPAREN, SEMI, SIZEOF, STATIC, STRLIT,
    STRUCT, SWITCH, TILDE, UNION, VOID, WHILE, XEOF, XMARK

};

int Token;
char Text[TEXTLEN + 1];
int Value;
int Line;
int Errors;
int Syntoken;
int Putback;
int Rejected;
int Rejval;
char Rejtext[TEXTLEN + 1];
char *File;
int Isp;
int Textseg;
char *Names[NSYMBOLS];
int Prims[NSYMBOLS];
char Types[NSYMBOLS];
char Stcls[NSYMBOLS];
int Sizes[NSYMBOLS];
int Vals[NSYMBOLS];
char *Mtext[NSYMBOLS];
int Globs;
int Locs;
int Thisfn;
char Nlist[POOLSIZE];
int Nbot;
int Ntop;
int Breakstk[MAXBREAK], Bsp;
int Contstk[MAXBREAK], Csp;
int Retlab;
int LIaddr[MAXLOCINIT];
int LIval[MAXLOCINIT];
int Nli;
int Q_type;
int Q_val;
char Q_name[NAMELEN + 1];
int Q_cmp;

int addglob(char *name, int prim, int type, int scls, int size, int val, char *mval, int init);
int addloc(char *name, int prim, int type, int scls, int size, int val, int init);
void cerror(char *s, int c);
int chrpos(char *s, int c);
void clear(int q);
void clrlocs(void);
void colon(void);
void commit(void);
void compound(int lbr);
int comptype(int p);
int constexpr(void);
void copyname(char *name, char *s);
int deref(int p);
int eofcheck(void);
void error(char *s, char *a);
int expr(int *lv);
void fatal(char *s);
int findmem(int y, char *s);
int findstruct(char *s);
int findsym(char *s);
char *galloc(int k);
void gen(char *s);
int genadd(int p1, int p2, int swap);
void genaddr(int y);
void genalign(int k);
void genand(void);
void genargc(void);
void genasop(int op, int p1, int p2, int swap);
int genbinop(int op, int p1, int p2);
void genbool(void);
void genbrfalse(int dest);
void genbrtrue(int dest);
void genbss(char *name, int len);
void gencall(int y);
void gencalr(void);
void gencmp(char *inst);
void gendata(void);
void gendefb(int v);
void gendefl(int id);
void gendefp(int v);
void gendefs(char *s, int len);
void gendefw(int v);
void gendiv(int swap);
void genentry(void);
void genexit(void);
void geninc(int *lv, int inc, int pre);
void genind(int p);
void genior(void);
void genjump(int dest);
void genlab(int id);
void genldlab(int id);
void genlit(int v);
void genln(char *s);
void genlocinit(void);
void genlognot(void);
void genmod(int swap);
void genmul(void);
void genname(char *name);
void genneg(void);
void gennot(void);
void genpostlude(void);
void genprelude(void);
void genpublic(char *name);
void genpush(void);
void genpushlit(int n);
void genraw(char *s);
void genscale(void);
void genscale2(void);
void genshl(int swap);
void genshr(int swap);
void genstack(int n);
void genstore(int op, int *lv, int *lv2);
int gensub(int p1, int p2, int swap);
void genswitch(int *vals, int *labs, int nc, int dflt);
void gentext(void);
void genxor(void);
char *globname(char *s);
char *gsym(char *s);
void ident(void);
int inttype(int p);
int label(void);
char *labname(int id);
void lbrace(void);
void lgen(char *s, char *inst, int n);
void lgen2(char *s, int v1, int v2);
void load(void);
void lparen(void);
void match(int t, char *what);
int next(void);
void ngen(char *s, char *inst, int n);
void ngen2(char *s, char *inst, int n, int a);
int objsize(int prim, int type, int size);
int pointerto(int prim);
int primtype(int t, char *s);
void putback(int t);
void rbrace(void);
void rbrack(void);
void reject(void);
void resume(void);
int rexpr(int com);
void rparen(void);
void rvalue(int *lv);
int scan(void);
void semi(void);
void sgen(char *s, char *inst, char *s2);
void sgen2(char *s, char *inst, int v, char *s2);
int skip(void);
void suspend(void);
int synch(int syn);
void top(void);
int typematch(int p1, int p2);
