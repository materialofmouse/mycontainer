int init_proc() {
	char* PROC_PATH = "/proc";
	if( access( PROC_PATH, F_OK) < 0){
		if( mkdir(PROC_PATH, 0555) < 0){
			perror("[ERROR]: cannot mkdir at proc path");
			return -1;
		}
	}
	if (mount("proc", "/proc", "proc", 0, NULL) < 0) {
				perror("[ERROR]: mount proc");
			return -1;
	}
	return 0;
}

int child_process() { 
		if(init_overlay() < 0) {
		perror("[ERROR]: init overlay");
		return -1;
	}

	printf("[DEBUG]: child process:%d\n",(int)getpid());
	sethostname("container",9);
	if (chdir("/home/mouse/work/mycontainer/condir/root") < 0) {
		perror("[ERROR]: chdir");
		return -1;
	}
	if (chroot("/home/mouse/work/mycontainer/condir/root") < 0) {
		perror("[ERROR]: chroot");
		return -1;
	}
	if (init_proc() < 0){
		perror("[ERROR]: init_proc");
		return -1;
	}
	
	if (execl("/bin/bash","a",NULL) < 0){
		perror("[ERROR]: bash");
		return -1;
	}
}


