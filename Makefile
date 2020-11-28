MODULES=compiler.c memory.c vm.c debug.c scanner.c table.c
OMODULES=obj/compiler.o obj/memory.o obj/vm.o obj/debug.o obj/scanner.o obj/table.o

zao: $(OMODULES) main.c
	g++ -o zao $(OMODULES) main.c

obj/compiler.o: compiler.c
	g++ -o obj/compiler.o -c compiler.c

obj/memory.o: memory.c
	g++ -o obj/memory.o -c memory.c

obj/vm.o: vm.c
	g++ -o obj/vm.o -c vm.c

obj/debug.o: debug.c
	g++ -o obj/debug.o -c debug.c

obj/scanner.o: scanner.c
	g++ -o obj/scanner.o -c scanner.c

obj/table.o: table.c
	g++ -o obj/table.o -c table.c

clean:
	rm $(OMODULES)