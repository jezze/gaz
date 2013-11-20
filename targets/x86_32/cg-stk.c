#include "../../defs.h"
#include "../../cgen.h"

int arch_intsize()
{

    return 4;

}

int arch_pointersize()
{

    return 4;

}

void cgdata(void)
{

    gen(".data");

}

void cgtext(void)
{

    gen(".text");

}

void cgprelude(void)
{

}

void cgpostlude(void)
{

}

void cgpublic(char *s)
{

    ngen(".globl\t%s", s, 0);

}

void cglit(int v)
{

    ngen("%s\t$%d,%%eax", "movl", v);

}

void cgclear(void)
{

    gen("xorl\t%eax,%eax");

}

void cgldgb(char *s)
{

    sgen("%s\t%s,%%al", "movb", s);

}

void cgldgw(char *s)
{

    sgen("%s\t%s,%%eax", "movl", s);

}

void cgldlb(int n)
{

    ngen("%s\t%d(%%ebp),%%al", "movb", n);

}

void cgldlw(int n)
{

    ngen("%s\t%d(%%ebp),%%eax", "movl", n);

}

void cgldsb(int n)
{

    lgen("%s\t%c%d,%%al", "movb", n);

}

void cgldsw(int n)
{

    lgen("%s\t%c%d,%%eax", "movl", n);

}

void cgldla(int n)
{

    ngen("%s\t%d(%%ebp),%%eax", "leal", n);

}

void cgldsa(int n)
{

    lgen("%s\t$%c%d,%%eax", "movl", n);

}

void cgldga(char *s)
{

    sgen("%s\t$%s,%%eax", "movl", s);

}

void cgindb(void)
{

    gen("movl\t%eax,%edx");
    cgclear();
    gen("movb\t(%edx),%al");

}

void cgindw(void)
{

    gen("movl\t(%eax),%eax");

}

void cgldlab(int id)
{

    lgen("%s\t$%c%d,%%eax", "movl", id);

}

void cgpush(void)
{

    gen("pushl\t%eax");

}

void cgpushlit(int n)
{

    ngen("%s\t$%d", "pushl", n);

}

void cgpop2(void)
{

    gen("popl\t%ecx");

}

void cgswap(void)
{

    gen("xchgl\t%eax,%ecx");

}

void cgand(void)
{

    gen("andl\t%ecx,%eax");

}

void cgxor(void)
{

    gen("xorl\t%ecx,%eax");

}

void cgior(void)
{

    gen("orl\t%ecx,%eax");

}

void cgadd(void)
{

    gen("addl\t%ecx,%eax");

}

void cgmul(void)
{

    gen("imull\t%ecx,%eax");

}

void cgsub(void)
{

    gen("subl\t%ecx,%eax");

}

void cgdiv(void)
{

    gen("cdq");
    gen("idivl\t%ecx");

}

void cgmod(void)
{

    cgdiv();
    gen("movl\t%edx,%eax");

}

void cgshl(void)
{

    gen("shll\t%cl,%eax");

}

void cgshr(void)
{

    gen("sarl\t%cl,%eax");

}

void cgcmp(char *inst)
{

    int lab = label();

    gen("xorl\t%edx,%edx");
    cgpop2();
    gen("cmpl\t%eax,%ecx");
    lgen("%s\t%c%d", inst, lab);
    gen("incl\t%edx");
    genlab(lab);
    gen("movl\t%edx,%eax");

}

void cgeq()
{

    cgcmp("jne");

}

void cgne()
{

    cgcmp("je");

}

void cglt()
{

    cgcmp("jge");

}

void cggt()
{

    cgcmp("jle");

}

void cgle()
{

    cgcmp("jg");

}

void cgge()
{

    cgcmp("jl");

}

void cgneg(void)
{

    gen("negl\t%eax");

}

void cgnot(void)
{

    gen("notl\t%eax");

}

void cglognot(void)
{

    gen("negl\t%eax");
    gen("sbbl\t%eax,%eax");
    gen("incl\t%eax");

}

void cgscale(void)
{

    gen("shll\t$2,%eax");

}

void cgscale2(void)
{

    gen("shll\t$2,%ecx");

}

void cgunscale(void)
{

    gen("shrl\t$2,%eax");

}

void cgscaleby(int v)
{

    ngen("%s\t$%d,%%ecx", "movl", v);
    gen("mull\t%ecx");

}

void cgscale2by(int v)
{

    gen("pushl\t%eax");
    ngen("%s\t$%d,%%eax", "movl", v);
    gen("mull\t%ecx");
    gen("movl\t%eax,%ecx");
    gen("popl\t%eax");

}

void cgunscaleby(int v)
{

    ngen("%s\t$%d,%%ecx", "movl", v);
    gen("xorl\t%edx,%edx");
    gen("divl\t%ecx");

}

void cgbool(void)
{

    gen("negl\t%eax");
    gen("sbbl\t%eax,%eax");
    gen("negl\t%eax");

}

void cgldinc(void)
{

    gen("movl\t%eax,%edx");

}

void cginc1pi(int v)
{

    ngen("%s\t$%d,(%%eax)", "addl", v);

}

void cgdec1pi(int v)
{

    ngen("%s\t$%d,(%%eax)", "subl", v);

}

void cginc2pi(int v)
{

    ngen("%s\t$%d,(%%edx)", "addl", v);

}

void cgdec2pi(int v)
{

    ngen("%s\t$%d,(%%edx)", "subl", v);

}

void cgincpl(int a, int v)
{

    ngen2("%s\t$%d,%d(%%ebp)", "addl", v, a);

}

void cgdecpl(int a, int v)
{

    ngen2("%s\t$%d,%d(%%ebp)", "subl", v, a);

}

void cgincps(int a, int v)
{

    lgen2("addl\t$%d,%c%d", v, a);

}

void cgdecps(int a, int v)
{

    lgen2("subl\t$%d,%c%d", v, a);

}

void cgincpg(char *s, int v)
{

    sgen2("%s\t$%d,%s", "addl", v, s);

}

void cgdecpg(char *s, int v)
{

    sgen2("%s\t$%d,%s", "subl", v, s);

}

void cginc1iw(void)
{

    ngen("%s\t(%%eax)", "incl", 0);

}

void cgdec1iw(void)
{

    ngen("%s\t(%%eax)", "decl", 0);

}

void cginc2iw(void)
{

    ngen("%s\t(%%edx)", "incl", 0);

}

void cgdec2iw(void)
{

    ngen("%s\t(%%edx)", "decl", 0);

}

void cginclw(int a)
{

    ngen("%s\t%d(%%ebp)", "incl", a);

}

void cgdeclw(int a)
{

    ngen("%s\t%d(%%ebp)", "decl", a);

}

void cgincsw(int a)
{

    lgen("%s\t%c%d", "incl", a);

}

void cgdecsw(int a)
{

    lgen("%s\t%c%d", "decl", a);

}

void cgincgw(char *s)
{

    sgen("%s\t%s", "incl", s);

}

void cgdecgw(char *s)
{

    sgen("%s\t%s", "decl", s);

}

void cginc1ib(void)
{

    ngen("%s\t(%%eax)", "incb", 0);

}

void cgdec1ib(void)
{

    ngen("%s\t(%%eax)", "decb", 0);

}

void cginc2ib(void)
{

    ngen("%s\t(%%edx)", "incb", 0);

}

void cgdec2ib(void)
{

    ngen("%s\t(%%edx)", "decb", 0);

}

void cginclb(int a)
{

    ngen("%s\t%d(%%ebp)", "incb", a);

}

void cgdeclb(int a)
{

    ngen("%s\t%d(%%ebp)", "decb", a);

}

void cgincsb(int a)
{

    lgen("%s\t%c%d", "incb", a);

}

void cgdecsb(int a)
{

    lgen("%s\t%c%d", "decb", a);

}

void cgincgb(char *s)
{

    sgen("%s\t%s", "incb", s);

}

void cgdecgb(char *s)
{

    sgen("%s\t%s", "decb", s);

}

void cgbr(char *how, int n)
{

    int lab = label();

    gen("orl\t%eax,%eax");
    lgen("%s\t%c%d", how, lab);
    lgen("%s\t%c%d", "jmp", n);
    genlab(lab);

}

void cgbrtrue(int n)
{

    cgbr("jz", n);

}

void cgbrfalse(int n)
{

    cgbr("jnz", n);

}

void cgjump(int n)
{

    lgen("%s\t%c%d", "jmp", n);

}

void cgldswtch(int n)
{

    lgen("%s\t$%c%d,%%edx", "movl", n);

}

void cgcalswtch(void)
{

    gen("jmp\tswitch");

}

void cgcase(int v, int l)
{

    lgen2(".long\t%d,%c%d", v, l);

}

void cgpopptr(void)
{

    gen("popl\t%edx");

}

void cgstorib(void)
{

    ngen("%s\t%%al,(%%edx)", "movb", 0);

}

void cgstoriw(void)
{

    ngen("%s\t%%eax,(%%edx)", "movl", 0);

}

void cgstorlb(int n)
{

    ngen("%s\t%%al,%d(%%ebp)", "movb", n);

}

void cgstorlw(int n)
{

    ngen("%s\t%%eax,%d(%%ebp)", "movl", n);

}

void cgstorsb(int n)
{

    lgen("%s\t%%al,%c%d", "movb", n);

}

void cgstorsw(int n)
{

    lgen("%s\t%%eax,%c%d", "movl", n);

}

void cgstorgb(char *s)
{

    sgen("%s\t%%al,%s", "movb", s);

}

void cgstorgw(char *s)
{

    sgen("%s\t%%eax,%s", "movl", s);

}

void cginitlw(int v, int a)
{

    ngen2("%s\t$%d,%d(%%ebp)", "movl", v, a);

}

void cgcall(char *s)
{

    sgen("%s\t%s", "call", s);

}

void cgcalr(void)
{

    gen("call\t*%eax");

}

void cgstack(int n)
{

    ngen("%s\t$%d,%%esp", "addl", n);

}

void cgentry(void)
{

    gen("pushl\t%ebp");
    gen("movl\t%esp,%ebp");

}

void cgexit(void)
{

    gen("popl\t%ebp");
    gen("ret");

}

void cgdefb(int v)
{

    ngen("%s\t%d", ".byte", v);

}

void cgdefw(int v)
{

    ngen("%s\t%d", ".long", v);

}

void cgdefp(int v)
{

    ngen("%s\t%d", ".long", v);

}

void cgdefl(int v)
{

    lgen("%s\t%c%d", ".long", v);

}

void cgdefc(int c)
{

    ngen("%s\t'%c'", ".byte", c);

}

void cgbss(char *s, int z)
{

    ngen(".lcomm\t%s,%d", s, z);

}

