TARGETS_STK=targets/gen-stk.c
TARGETS_SYN=targets/gen-syn.c
TARGETS_X86_16_SYN=$(TARGETS_SYN) targets/x86_16/cg-syn.c
TARGETS_X86_32_STK=$(TARGETS_STK) targets/x86_32/cg-stk.c
TARGETS_X86_32_SYN=$(TARGETS_SYN) targets/x86_32/cg-syn.c
TARGETS_X86_64_STK=$(TARGETS_STK) targets/x86_64/cg-stk.c
TARGETS_X86_64_SYN=$(TARGETS_SYN) targets/x86_64/cg-syn.c

SRC=decl.c expr.c main.c misc.c scan.c stmt.c sym.c
OBJ=decl.o expr.o main.o misc.o scan.o stmt.o sym.o

all: scc16-syn scc32-stk scc32-syn scc64-stk scc64-syn

.c.o:
	$(CC) -c $<

scc16-syn: $(OBJ) $(TARGETS_X86_16_SYN)
	$(CC) -o $@ $^

scc32-stk: $(OBJ) $(TARGETS_X86_32_STK)
	$(CC) -o $@ $^

scc32-syn: $(OBJ) $(TARGETS_X86_32_SYN)
	$(CC) -o $@ $^

scc64-stk: $(OBJ) $(TARGETS_X86_64_STK)
	$(CC) -o $@ $^

scc64-syn: $(OBJ) $(TARGETS_X86_64_SYN)
	$(CC) -o $@ $^

example/test.s: example/test.c
	./scc64 < $^ > $@

example/test.o: example/test.s
	$(AS) -o $@ $^

example/test: example/test.o
	$(CC) -o example/test example/test.o example/crt0.s

clean:
	rm -f scc16*
	rm -f scc32*
	rm -f scc64*
	rm -f $(OBJ)
	rm -f example/test.s
	rm -f example/test.o
	rm -f example/test
