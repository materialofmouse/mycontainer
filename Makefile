container: container.c
	gcc container.c -o container

test : test.c
	gcc test.c -o test

run :
	sudo ./container
