all:
	clang src/main.c src/server.c -Iinclude -o http

clean:
	rm http
