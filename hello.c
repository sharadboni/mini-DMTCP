#include<stdio.h>
#include<unistd.h>

int main() {
	for(int i = 0; ; i++) {
		printf("%d ", i);
		fflush(stdout);
		sleep(1);	
	}
	return 0;
}
