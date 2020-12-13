MODULES=common.c compiler.c memory.c vm.c debug.c scanner.c table.c object.c
OMODULES=obj/common.o obj/compiler.o obj/memory.o obj/vm.o obj/debug.o obj/scanner.o \
	obj/table.o obj/object.o

zao: $(OMODULES) main.c
	g++ -o zao $(OMODULES) main.c

obj/common.o: common.c
	g++ -o obj/common.o -c common.c

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

obj/object.o: object.c
	g++ -o obj/object.o -c object.c

clean:
	rm $(OMODULES)