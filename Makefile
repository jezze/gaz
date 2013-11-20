AS_OBJ=as.o
CC_OBJ=cc.o decl.o expr.o misc.o scan.o stmt.o sym.o
CC_OBJ_STK=targets/gen-stk.o
CC_OBJ_SYN=targets/gen-syn.o
CC_OBJ_X86_16_SYN=$(CC_OBJ_SYN) targets/x86_16/cg-syn.o
CC_OBJ_X86_32_STK=$(CC_OBJ_STK) targets/x86_32/cg-stk.o
CC_OBJ_X86_32_SYN=$(CC_OBJ_SYN) targets/x86_32/cg-syn.o
CC_OBJ_X86_64_STK=$(CC_OBJ_STK) targets/x86_64/cg-stk.o
CC_OBJ_X86_64_SYN=$(CC_OBJ_SYN) targets/x86_64/cg-syn.o

all: cc16-syn cc32-stk cc32-syn cc64-stk cc64-syn

.c.o:
	$(CC) -c -o $@ $<

as: $(AS_OBJ)
	$(CC) -o $@ $^

cc16-syn: $(CC_OBJ) $(CC_OBJ_X86_16_SYN)
	$(CC) -o $@ $^

cc32-stk: $(CC_OBJ) $(CC_OBJ_X86_32_STK)
	$(CC) -o $@ $^

cc32-syn: $(CC_OBJ) $(CC_OBJ_X86_32_SYN)
	$(CC) -o $@ $^

cc64-stk: $(CC_OBJ) $(CC_OBJ_X86_64_STK)
	$(CC) -o $@ $^

cc64-syn: $(CC_OBJ) $(CC_OBJ_X86_64_SYN)
	$(CC) -o $@ $^

example/test.s: example/test.c
	./cc32-syn < $^ > $@

example/test.o: example/test.s
	$(AS) -o $@ $^

example/test: example/test.o
	$(CC) -o example/test example/test.o example/crt0.s

clean:
	rm -f as
	rm -f $(AS_OBJ)
	rm -f cc16-syn
	rm -f cc32-stk
	rm -f cc32-syn
	rm -f cc64-stk
	rm -f cc64-syn
	rm -f $(CC_OBJ)
	rm -f $(CC_OBJ_STK)
	rm -f $(CC_OBJ_SYN)
	rm -f $(CC_OBJ_X86_16_SYN)
	rm -f $(CC_OBJ_X86_32_STK)
	rm -f $(CC_OBJ_X86_32_SYN)
	rm -f $(CC_OBJ_X86_64_STK)
	rm -f $(CC_OBJ_X86_64_SYN)
	rm -f example/test.s
	rm -f example/test.o
	rm -f example/test
