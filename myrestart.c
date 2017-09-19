#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<ucontext.h>
#include<sys/mman.h>

#define ADDR 0x5300000
#define SIZE 0x1000

struct header { 
  char *start;
  char *end;
  char rwxp[4];
};

ucontext_t ucp_ref;
struct header memory_header;
char chkpt_img[1000];
int file_descriptor;
char input_c;

void print_error(){
	printf("Encountered Error....Try again later");
	exit(1);
}

void restore_memory() {
	file_descriptor = open(chkpt_img, O_RDONLY);
	if (file_descriptor < 0) {
		print_error();
	}

	read(file_descriptor, &ucp_ref, sizeof(ucontext_t));
	while (read(file_descriptor, &input_c, 1) > 0) {
		lseek(file_descriptor, -1, SEEK_CUR);
		read(file_descriptor, &memory_header, sizeof(struct header));
		unsigned long int length = memory_header.end - memory_header.start;
		printf("%lu\n",length);
		fflush(stdout);
		munmap((void *) memory_header.start, length);
		char *p=mmap((void*) memory_header.start, length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_FIXED | MAP_PRIVATE, -1, 0);
		printf("%p-%p\t%p\n",memory_header.start,memory_header.end,p);
		int rwxp = 0;
		if (memory_header.rwxp[0] != '-') rwxp |= PROT_READ;
		if (memory_header.rwxp[1] != '-') rwxp |= PROT_WRITE;
		if (memory_header.rwxp[2] != '-') rwxp |= PROT_EXEC;
		read(file_descriptor, memory_header.start, length);
		mprotect(memory_header.start, length, rwxp);
		
	}

	setcontext(&ucp_ref);

	close(file_descriptor);
}


int main(int argc, char * argv[]) {

	sprintf(chkpt_img, argv[1]);
	char *ptr = mmap((void *)ADDR, SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
	asm volatile ("mov %0, %%sp;" : : "g" (ptr) : "memory");
	restore_memory();
	return 0;
}

