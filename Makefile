MODULES=compiler.c memory.c vm.c debug.c
OMODULES=obj/compiler.o obj/memory.o obj/vm.o obj/debug.o

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

clean:
	rm $(OMODULES)