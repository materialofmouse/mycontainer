#define _GNU_SOURCE
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<sched.h>
#include<sys/mount.h>
#include<fcntl.h>
#include</usr/include/linux/capability.h>

const unsigned int UNSHARE_FLAGS = ( CLONE_FILES | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID); 


int main(){
	pid_t pid;
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
    //capability header
		cap_user_header_t hdrp;
		cap_user_header_t h_obj;
		hdrp = &h_obj;
		hdrp->version = _LINUX_CAPABILITY_VERSION_3;
		hdrp->pid = 1;

		//capability data 
		cap_user_data_t datap;
		cap_user_data_t d_obj;
		datap = &d_obj;
		int err;
		datap->permitted = 0;
		datap->permitted = datap->permitted | (1 << CAP_NET_RAW) | (1 << CAP_SYS_CHROOT) | (1 << CAP_SETFCAP);
		datap->inheritable = datap->permitted;
		datap->effective = 1;
		err = capset(hdrp, datap);
		perror("capset");
		//err = capget(hdrp, datap);
		//perror("capget");
		printf("effective-> %d\n",datap->effective);
		printf("permitted-> %lld\n",datap->permitted);
		printf("inheritable-> %lld\n",datap->inheritable);
			

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

  printf("parrent process:%d\n",(int)getpid());
  if ((pid = waitpid(pid,&status,0)) < 0) {
    perror("wait");
    return 1;
  }
  if (WIFEXITED(status)) {
    printf("pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
  }
  return 0;
}
