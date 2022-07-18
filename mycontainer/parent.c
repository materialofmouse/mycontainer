int init_cgroup() {
	char* CGROUP_PATH = "/sys/fs/cgroup/container";
	//cgroupの有効化
	if( access( CGROUP_PATH, F_OK) < 0){
		if( mkdir(CGROUP_PATH, 0644) < 0){
			perror("[ERROR]: cannot mkdir at cgroup path");
			return -1;
		}
	}
	else return 0;

	int fd;
	//プロセスの登録
	fd = open("/sys/fs/cgroup/container/cgroup.procs", O_WRONLY);
	if( fd < 0 ){ 
		perror("[ERROR]: fd cannot open"); 
		return -1; 
	}
	int _pid = getpid();
	char buff[6];
	snprintf(buff, 6 , "%d", _pid);
	write(fd, buff, 6);
	close(fd);
	return 0;
}

int set_subsystem(char *subsytem) {
//サブシステムの登録
	int fd;
	fd = open("/sys/fs/cgroup/cgroup.subtree_control", O_WRONLY);
	if (fd < 0) { 
		perror("[ERROR]: subtree error"); 
		return -1;
	}
	write(fd, "+cpu", 5);
	close(fd);
	return 0;
}

int restrict_cpu(int percent) {
	int fd;
	fd = open("/sys/fs/cgroup/container/cpu.max", O_WRONLY);
	if (fd < 0){ 
		perror("[ERROR]: cpu open"); 
		return -1; 
	}
	write(fd, "10000", 6);//このサーバーの場合
	close(fd);
	return 0;
}

//OverlayFSの使用にmountを行い、引数で設定する箇所があるためそれを行う関数
int init_overlay(){ 
	char* ROOT_PATH = "/home/mouse/work/mycontainer/condir/overlay";
	errno = 0;
	if(mount("overlay", "/home/mouse/work/mycontainer/condir/root", "overlay", 0,
				"lowerdir=/home/mouse/work/mycontainer/debian,upperdir=/home/mouse/work/mycontainer/condir/root,workdir=/home/mouse/work/mycontainer/condir/work") != 0){ 
		perror("[ERROR]: mount overlay");
		return -1;
	}
	return 0;
}

int set_capability() {
	cap_t caps = cap_init();
	const cap_value_t cap_list[15] = {
		CAP_SETPCAP,
		CAP_MKNOD,
		CAP_AUDIT_WRITE,
		CAP_CHOWN,
		CAP_NET_RAW,
		CAP_DAC_OVERRIDE,
		CAP_FOWNER,
		CAP_FSETID,
		CAP_KILL,
		CAP_SETGID,
		CAP_SETUID,
		CAP_NET_BIND_SERVICE,
		CAP_SYS_CHROOT,
		CAP_SETFCAP,
		CAP_SYS_ADMIN
	};

	caps = cap_get_proc();
	//cap_clear(caps);
	if(cap_set_proc(caps) == -1) {
		perror("[ERROR]: cap_set_proc");
		return -1;
	}
	// -- debug --
	//printf("capability:%s\n",cap_to_text(caps, NULL));
	//printf("\n");

	if(cap_set_flag(caps, CAP_PERMITTED, 14, cap_list, CAP_SET) == -1){ 
		perror("[ERROR]: cap_set_flag");
		return -1;
	}
	if(cap_set_flag(caps, CAP_INHERITABLE, 14, cap_list, CAP_SET) == -1) {
		return -1;
	}
	if(cap_set_flag(caps, CAP_EFFECTIVE, 14, cap_list, CAP_SET) == -1) {
		perror("[ERROR]: cap_set_flag");
		return -1;
	}
	if(cap_set_proc(caps) == -1) {
		perror("[ERROR]: cap_set_proc");
		return -1;
	}
	// -- debug --
	//caps = cap_get_proc();
	//printf("capability:%s\n", cap_to_text(caps, NULL));
	return 0;
}


int parrent_process(pid_t * pid) {
	int status;
	printf("[DEBUG]: parrent process:%d\n",(int)getpid());
	if ((*pid = waitpid(*pid,&status,0)) < 0) {
		perror("[ERROR]: wait");
		return -1;
	}
	if (WIFEXITED(status)) {
		printf("[DEBUG]: pid:%d status:%d\n",(int)getpid(),WEXITSTATUS(status));
		close_container();
	}
}

void close_container() {
	if(umount("/home/mouse/work/mycontainer/condir/root/proc") < 0 ) perror("[ERROR]: umount");
	if(umount("/home/mouse/work/mycontainer/condir/root") < 0 ) perror("[ERROR]: umount");
}


