container: mycontainer/main.c
	@gcc -L/lib/x86_64-linux-gnu mycontainer/main.c -lcap -o container
test : test.c
	gcc test.c -o test

run :
	sudo ./container
