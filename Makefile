OBJ_DIR=obj

MODULES=common.c compiler.c memory.c vm.c debug.c scanner.c table.c object.c
OMODULES=$(OBJ_DIR)/common.o $(OBJ_DIR)/compiler.o $(OBJ_DIR)/memory.o \
	$(OBJ_DIR)/vm.o $(OBJ_DIR)/debug.o $(OBJ_DIR)/scanner.o \
	$(OBJ_DIR)/table.o $(OBJ_DIR)/object.o

zao: obj_dir $(OMODULES) main.c
	g++ -o zao $(OMODULES) main.c

$(OBJ_DIR)/common.o: common.c
	g++ -o $(OBJ_DIR)/common.o -c common.c

$(OBJ_DIR)/compiler.o: compiler.c
	g++ -o $(OBJ_DIR)/compiler.o -c compiler.c

$(OBJ_DIR)/memory.o: memory.c
	g++ -o $(OBJ_DIR)/memory.o -c memory.c

$(OBJ_DIR)/vm.o: vm.c
	g++ -o $(OBJ_DIR)/vm.o -c vm.c

$(OBJ_DIR)/debug.o: debug.c
	g++ -o $(OBJ_DIR)/debug.o -c debug.c

$(OBJ_DIR)/scanner.o: scanner.c
	g++ -o $(OBJ_DIR)/scanner.o -c scanner.c

$(OBJ_DIR)/table.o: table.c
	g++ -o $(OBJ_DIR)/table.o -c table.c

$(OBJ_DIR)/object.o: object.c
	g++ -o $(OBJ_DIR)/object.o -c object.c

obj_dir:
ifeq (,$(wildcard ./zao))
	$(shell mkdir $(OBJ_DIR))
endif

.PHONY=obj_dir

clean:
	rm $(OMODULES)