package main

import (
	"fmt"
	"syscall"
)

func main() {
	var current_pid int = syscall.Getpid()
	var UNSHARE_FLAGS int = syscall.CLONE_FILES | syscall.CLONE_NEWIPC | syscall.CLONE_NEWNS | syscall.CLONE_NEWUTS | syscall.CLONE_NEWPID
	fmt.Printf("pid: %d\n", current_pid)

	err := syscall.Unshare(UNSHARE_FLAGS)
	if err != nil {
		fmt.Println("unshare:", err)
	}

	pid, err := syscall.ForkExec("/bin/bash", []string{""}, nil)
	if err != nil {
		fmt.Println("ForkExec:", err)
	}

	if pid == 0 {
		chdirErr := syscall.Chdir("./debian")
		if chdirErr != nil {
			fmt.Println("chdir")
		}

		chrootErr := syscall.Chroot("./debian")
		if chrootErr != nil{
			fmt.Println("chroot")
		}
	}

	//parent process
	var waitStatus syscall.WaitStatus
	wpid, err := syscall.Wait4(pid, &waitStatus, 0, nil);
	fmt.Println(wpid)
	if err != nil {
		fmt.Println("wait:", err)
	}

	//if waitStatus.Exited() {
		//fmt.Println("exit!")
//d	}
}
