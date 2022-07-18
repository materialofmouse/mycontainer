OBJS = mycontainer/container.o mycontainer/controller.o mycontainer/main.o 
LIBS = -lcap
LDFLAGS = -L/lib/x86_64-linux-gnu

build: $(OBJS)
	@gcc $(LDFLAGS) $(OBJS) $(LIBS) -o container

test : test.c
	gcc test.c -o test

run : build
	sudo ./container
