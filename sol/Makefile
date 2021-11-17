# APD - Tema 1
# Octombrie 2021

build:
	@echo "Building..."
	@gcc -o test test.c -lm -Wall -Werror
	@echo "Done"

build_debug:
	@echo "Building debug..."
	@gcc -o main main.c -lm -Wall -Werror -O0 -g3 -DDEBUG
	@echo "Done"

clean:
	@echo "Cleaning..."
	@rm -rf test
	@echo "Done"
