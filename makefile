a.out: y.tab.cpp lex.yy.c
	g++ -w lex.yy.c y.tab.cpp index.cpp registers.cpp
	@echo "Each Test File has a naming format"
	@echo "PLease Look TEST folder to check the test files names"
	@echo "Run the Test Codes as : ./a.out < [TEST/testx]"
	@echo "And then, to run asm code, do : make runnasm"
	@echo "To run all the test cases at once , do : make autorun"

y.tab.cpp: parser.y
	yacc -Wno -v -d -g parser.y -o y.tab.cpp

lex.yy.c: lexer.l y.tab.hpp
	lex lexer.l

clean:
	@rm -rf lex.yy.c y.tab.hpp y.tab.cpp a.out gen.o gen.lst gen y.dot y.output gen.asm
	@echo "rm -rf lex.yy.c y.tab.hpp y.tab.cpp a.out gen.o gen.lst gen y.dot y.output gen.asm"

runnasm:
	nasm -f elf64 -l gen.lst gen.asm
	gcc -m64 -o gen gen.o -no-pie
	./gen

autorun:
	python3 test_script.py 
