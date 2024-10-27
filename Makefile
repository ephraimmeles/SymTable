# Dependency rules for non-file targets
all: testsymtablelist testsymtablehash

# Clobber target to remove additional files such as backups
clobber: clean
	rm -f *~ \#*\#

# Clean target to remove compiled files
clean:
	rm -f testsymtablelist testsymtablehash *.o

# Dependency rules for file targets

# Rule to build testsymtablelist executable
testsymtablelist: testsymtable.o symtablelist.o
	gcc217 testsymtable.o symtablelist.o -o testsymtablelist

# Rule to build testsymtablehash executable
testsymtablehash: testsymtable.o symtablehash.o
	gcc217 testsymtable.o symtablehash.o -o testsymtablehash

# Compile testsymtable.c to an object file
testsymtable.o: testsymtable.c symtable.h
	gcc217 -c testsymtable.c

# Compile symtablelist.c to an object file
symtablelist.o: symtablelist.c symtable.h
	gcc217 -c symtablelist.c

# Compile symtablehash.c to an object file
symtablehash.o: symtablehash.c symtable.h
	gcc217 -c symtablehash.c
