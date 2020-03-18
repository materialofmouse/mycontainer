#include <stdio.h>

typedef struct TEST_DATA{
	int test;
	int hoge;
} test_data;

void main(){
	test_data mydata = {10,20};
	printf("%d\n", mydata.test);
}
