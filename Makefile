OBJS = source/container.o source/controller.o source/main.o 
LIBS = -lcap
LDFLAGS = -L/lib/x86_64-linux-gnu

setup:
	sudo apt update
	sudo apt install debootstrap libcap2-dev make gcc bridge-utils
	
	@if [ ! -e debian ]; then \
		@sudo debootstrap --arch amd64 jessie ./debian http://ftp.jp.debian.org/debian; \
	fi
	@if [ ! -d layer ]; then \
		@mkdir -p ./layer/root ./layer/work; ./layer/diff; \
	fi

build: $(OBJS:.o=.c)
	@gcc -w $(LDFLAGS) $(OBJS:.o=.c) $(LIBS) -o container

run: build
	@sudo cp ./config/start.sh ./layer/diff/root/start.sh
	@sudo ./container 

observer:
	@sudo python3 tools/container_observer.py &
