build:
	gcc -Wall -o search_index *.c

run: build
	./search_index

clean:
	rm -f search_index