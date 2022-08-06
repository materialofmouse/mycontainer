#!/bin/python3
import time
import subprocess

if __name__=="__main__":
    container_state = False
    while True:
        controller = open("./config/controller_pid", 'r')
        pid = int(controller.readline())
        if container_state and pid == 0:
            print("container stop")
            subprocess.run(["./network/network_delete"], stdout=subprocess.DEVNULL)
            container_state = False
        elif not container_state and pid > 0:
            print("container launch")
            subprocess.run(["./network/network_setup"], stdout=subprocess.DEVNULL)
            container_state = True
        controller.close()
        time.sleep(1)


