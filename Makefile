OBJS = source/container.o source/controller.o source/main.o 
LIBS = -lcap
LDFLAGS = -L/lib/x86_64-linux-gnu

setup:
	#sudo apt update
	#sudo apt -y install debootstrap libcap2-dev make gcc bridge-utils
	
	@if [ ! -e debian ]; then \
		echo "\e[36m[INFO]: \e[0mrootfs file: ./debian not found! create..."; \
		sudo debootstrap --arch amd64 jessie ./debian http://ftp.jp.debian.org/debian; \
	else \
	echo "\e[36m[INFO]: \e[0mrootfs file: ./debian found!"; \
	fi
	@if [ ! -e layer ]; then \
		echo "\e[36m[INFO]: \e[0mlayer directory create!"; \
		mkdir -p ./layer/root ./layer/work ./layer/diff; \
	else \
		echo "\e[36m[INFO]: \e[0mlayer directory found!"; \
	fi

build: $(OBJS:.o=.c)
	@gcc -w $(LDFLAGS) $(OBJS:.o=.c) $(LIBS) -o container

run: build
	@sudo cp ./config/start.sh ./layer/diff/root/start.sh
	@sudo ./container 

observer:
	@sudo python3 tools/container_observer.py &
