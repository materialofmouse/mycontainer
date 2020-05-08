#define _GNU_SOURCE
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>
#include<errno.h>
#include</usr/include/linux/capability.h>

const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 


int main(){
	pid_t pid = getpid();
	errno = 0;
	
	struct __user_cap_header_struct hdr = { 0 };
	hdr.pid = 0;
	hdr.version = _LINUX_CAPABILITY_VERSION_3;

	struct __user_cap_data_struct data = { 0 };
	data.permitted =  (1 << CAP_NET_RAW) | (1 << CAP_SYS_CHROOT) | (1 << CAP_SYS_ADMIN);
	data.inheritable = data.permitted;
	data.effective = data.permitted;
	if(capset(&hdr, &data) < 0){
		perror("capset");
		printf("errno:%d\n",errno);
	}
	printf("effective:%lld\n",data.effective);
	printf("inheritable:%lld\n",data.inheritable);
	printf("permitted:%lld\n",data.permitted);
	hdr.pid = (int)getpid();
	if(capget(&hdr, &data) < 0){
		perror("capget");
		printf("errno:%d\n",errno);
	}
	printf("effective:%lld\n",data.effective);
	printf("inheritable:%lld\n",data.inheritable);
	printf("permitted:%lld\n",data.permitted);

	int status;
	if( unshare(UNSHARE_FLAGS) == -1){
		perror("unshare");
		return 1;
	}
  if( (pid = fork()) < 0) {
    perror("fork");
    return 1;
  }
	//make cgroup
	if( access("/sys/fs/cgroup/container", F_OK) != 0){
		if( mkdir("/sys/fs/cgroup/container", 0644) != 0){
			perror("mkdir");
			return 1;
		}
	}

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd == -1){ perror("open"); return 1; }
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	
	//サブシステムの登録
	fd = open("/sys/fs/cgroup/cgroup.subtree_control", O_WRONLY);
	if (fd == -1){ perror("subtree error"); return 1;}
	write(fd, "+cpu", 5);
	close(fd);
	
	//CPU制限
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd == -1){ perror("cpu open"); return 1; }
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);

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
		if (mount("proc", "/proc", "proc", 0, NULL) != 0){
				perror("mount");
				return 1;
		}

		execl("/bin/bash","a",NULL);
    perror("bash");
  }

  printf("parrent process:%d\n",(int)pid);
  if ((pid = waitpid(pid,&status,0)) < 0) {
    perror("wait");
    return 1;
  }
  if (WIFEXITED(status)) {
    printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
  }
  return 0;
}
