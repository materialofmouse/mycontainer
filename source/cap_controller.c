#include <linux/capability.h>
#include <sys/capability.h>  
#include <sys/prctl.h> 
#include <sys/types.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cap_controller.h"

#define CAP_MAX 40
#define CAP_COUNT 16

struct capabilities {
	cap_value_t inh[CAP_MAX];
	int inh_count;
	cap_value_t eff[CAP_MAX];
	int eff_count;
	cap_value_t prm[CAP_MAX];
	int prm_count;
	cap_value_t bnd[CAP_MAX];
	int bnd_count;
};
static struct capabilities cap_conf;

cap_value_t read_cap_from_file() {
	FILE* file;
	file = fopen("./config/capabilities.conf", "r");
	if (file == NULL){
		perror("\x1b[31m[ERROR]\x1b[0m capabilities.conf is not found");
		return -1;
	}
	
	char line[256];
	int index = 0;
	char header;
	while(fgets(line, 256, file) != NULL) {
		char* end;
		int cap;
		//capability section header
		if(line[0] == '[') {
			//printf("header: %s", line);
			header = line[1];
			index = 0;
		}
		else if (line[0] == '\n') {
			continue;
		}
		else {
			//str -> int
			cap = strtol(line, &end, 10);
			if (errno == ERANGE){
				printf("\x1b[31m[ERROR]\x1b[0m CAP format is valid\n");
				return -1;
			}
			else if (end == line){
				printf("\x1b[31m[ERROR]\x1b[0m CAP is string\n");
				return -1;
			}
			else {
				//printf("line: %d\n", cap);
				if (header == 'I'){
					cap_conf.inh[index] = cap;
					cap_conf.inh_count = index;
				}
				else if (header == 'E'){
					cap_conf.eff[index] = cap;
					cap_conf.eff_count = index;
				}
				else if (header == 'P'){
					cap_conf.prm[index] = cap;
					cap_conf.prm_count = index;
				}
				else if (header == 'B'){
					cap_conf.bnd[index] = cap;
					cap_conf.bnd_count = index;
				}
				index++;
			}
		}
	}
	fclose(file);
	return 0;
}

int set_capability() {
	//cap_t caps = cap_init();
	const cap_value_t cap_list[CAP_COUNT] = {
		CAP_CHOWN,
		CAP_NET_RAW,
		CAP_FOWNER,
		CAP_FSETID,
		CAP_KILL,
		CAP_SETGID,
		CAP_SETUID,
		CAP_SETPCAP,
		CAP_NET_RAW,
		CAP_SYS_CHROOT,
		CAP_SYS_ADMIN,
		CAP_MKNOD,
		CAP_AUDIT_WRITE,
		CAP_SETFCAP,
		CAP_DAC_OVERRIDE,
		CAP_DAC_READ_SEARCH,
	};

	cap_t caps = cap_get_proc();
		// -- debug --
	printf("\x1b[36m[DEBUG]\x1b[0m current capability:%s\n",cap_to_text(caps, NULL));
	//printf("\n");

	if(cap_clear_flag(caps, CAP_INHERITABLE)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");
	}
	if(cap_clear_flag(caps, CAP_PERMITTED)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");	
	}
	if(cap_clear_flag(caps, CAP_EFFECTIVE)){
		perror("\x1b[31m[ERROR]\x1b[0m cap_clear_flag");
	}
	int i;
	for (i = 0; i < cap_conf.prm_count;i++){
		cap_value_t c[1];
		c[0] = cap_conf.prm[i];
		if(cap_set_flag(caps, CAP_PERMITTED, 1, c, CAP_SET) == -1){ 
			perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag prm");
			return -1;
		}
	}
	for (i = 0; i < cap_conf.inh_count; i++){
		cap_value_t c[1];
		c[0] = cap_conf.prm[i];
		if(cap_set_flag(caps, CAP_INHERITABLE, 1, c, CAP_SET) == -1) {
			perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag inh");
			return -1;
		}
	}
	for (i = 0; i < cap_conf.eff_count;i++){
		cap_value_t c[1];
		c[0] = cap_conf.prm[i];
		if(cap_set_flag(caps, CAP_EFFECTIVE, 1, c, CAP_SET) == -1) {
			perror("\x1b[31m[ERROR]\x1b[0m cap_set_flag eff");
		}
	}
	printf("\x1b[36m[DEBUG]\x1b[0m flag_set capability:%s\n",cap_to_text(caps, NULL));
	if(cap_set_proc(caps) == -1) {
		perror("\x1b[31m[ERROR]\x1b[0m cap_set_proc");
		return -1;
	}
	for (i = 0; i < cap_conf.bnd_count; i++){
		prctl(PR_CAPBSET_DROP, cap_conf.bnd[i], 0, 0, 0);
	}
	// -- debug --
	caps = cap_get_proc();
	printf("\x1b[36m[DEBUG]\x1b[0m capability:%s\n", cap_to_text(caps, NULL));
	return 0;
}
