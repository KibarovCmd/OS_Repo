All: compile run

compile:
	# Create directories if they don't exist
	mkdir -p bin lib

	# Compile source files into object files
	gcc -I ./include/ -o ./lib/parser.o -c ./src/parser.c
	gcc -I ./include/ -o ./lib/shell.o -c ./src/shell.c
	gcc -I ./include/ -o ./lib/main.o -c ./src/main.c
	
	# Link object files into final executable
	gcc -I ./include/ -o ./bin/myshell ./lib/parser.o ./lib/shell.o ./lib/main.o

run:
	./bin/myshell

