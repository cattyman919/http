all:
	clang src/main.c src/server.c src/routes.c -Iinclude -o http

clean:
	rm http
