# APD - Tema 1
# Octombrie 2021

build:
	@echo "Building..."
	@gcc -o tema1_par par_tema1.c par_genetic_algorithm.c -lm -Wall -Werror -lpthread
	@echo "Done"

build-merge:
	@echo "Building..."
	@gcc -o merges merge.c -lm -Wall -Werror -O0 -g -g3 -DDEBUG -lpthread
	@echo "Done"

build_debug:
	@echo "Building debug..."
	@gcc -o tema1_par par_tema1.c par_genetic_algorithm.c -lm -Wall -Werror -O0 -g -g3 -DDEBUG -lpthread
	@echo "Done"

clean:
	@echo "Cleaning..."
	@rm -rf par_tema1
	@echo "Done"
