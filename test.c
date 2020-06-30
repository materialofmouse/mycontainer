#include <stdio.h>
#include <sys/capability.h>

void main(){
	cap_t t;
	const cap_value_t a;
	t = cap_get_proc();
	if(t == NULL) printf("error\n");
	printf("%d\n",t);
}
