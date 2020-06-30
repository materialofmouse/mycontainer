container: container.c
	@gcc -lcap container.c -o container

test : test.c
	gcc test.c -o test

run :
	sudo ./container
