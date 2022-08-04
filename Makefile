OBJS = mycontainer/container.o mycontainer/controller.o mycontainer/main.o 
LIBS = -lcap
LDFLAGS = -L/lib/x86_64-linux-gnu
CON_DIR = ./condir

setup:
	sudo apt update
	sudo apt install debootstrap libcap2-dev make gcc bridge-utils
	
	@if [ ! -e debian ]; then \
		sudo debootstrap --arch amd64 jessie ./debian http://ftp.jp.debian.org/debian; \
	fi
	@if [ ! -d $(CON_DIR) ]; then \
		mkdir $(CON_DIR); mkdir ./condir/root ./condir/work; \
	fi

build: $(OBJS:.o=.c)
	@gcc $(LDFLAGS) $(OBJS:.o=.c) $(LIBS) -o container

test : test.c
	gcc test.c -o test

run : build
	sudo ./container
