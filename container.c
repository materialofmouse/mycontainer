#define _GNU_SOURCE
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>

int main(){
  pid_t pid;
  int status;
	int unshare_flags = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 
	if( unshare(unshare_flags) == -1){
		perror("unshare");
		return 1;
	}
  if ((pid = fork()) < 0) {
    perror("fork");
    return 1;
  }
	//cgroup
	/*if( mkdir("/sys/fs/cgroup/container", 0644) != 0){
		perror("mkdir");
		return 1;
	}*/
	int fd;
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY | O_CREAT);
	if( fd == -1){
		perror("open");
		return 1;
	}
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd == -1){
		perror("cpu open");
		return 1;
	}
	write(fd, "10000", 6);
	close(fd);
	//cgroup v1 is mounted already on debian 10
	//child process
  if (pid == 0) {
    printf("child process:%d\n",(int)getpid());
    sethostname("container",9);
		if (chdir("/home/mouse/container/debian") != 0){
			perror("chdir");
			return 1;
		}
		if(chroot("/home/mouse/container/debian") != 0){
			perror("chroot");
			return 1;
		}
		//if (mount("proc", "/proc", "proc", 0, NULL) != 0){
		//		perror("mount");
		//		return 1;
		//}

		execl("/bin/bash","a",NULL);
    perror("bash");
    //_exit(1);
  }

  printf("parrent proces:%d\n",(int)getpid());
  if ((pid = waitpid(pid,&status,0)) < 0) {
    perror("wait");
    return 1;
  }
  if (WIFEXITED(status)) {
    printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
  }
  return 0;
}
