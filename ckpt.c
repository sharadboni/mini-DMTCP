#include<stdio.h>
#include<ucontext.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>

struct header {
	char rwxp[4];
	char *start;
	char *end;
};

void print_error() {
	printf("Error occured...Try again later\n");
	exit(1);
}

void copy_address(int file_descriptor,char ** value) {
	char input_c;
	unsigned long int val;
	val = 0;
	while(1) {
		read(file_descriptor,&input_c,1);
		if((input_c >= '0') && (input_c <= '9')) input_c -= '0';
		else if ((input_c >= 'a') && (input_c <= 'f')) input_c -= 'a' - 10;
		else if ((input_c >= 'A') && (input_c <= 'F')) input_c -= 'A' - 10;
		else break;
		val = val * 16 + input_c;
	}
	*value = (char *)val;
}

void read_to_end(int fd, char* name)
{
	char c[1] = " ";
	while(c[0] != '\n')
	{
		if(read(fd,c,1)<0)
		{
			printf("Error on reading to the end of line\n");
			exit(1);
		}
		else if(c[0] == '[')
		{
			while(c[0] != ']')
			{
				read(fd,c,1);
				*name = c[0];
				name++;
			}
			// printf("%c",c[0]);
		}
	}
}
void sig_handler() {
	char name[100];
	char rwxp[4];
	char input_char;
	struct header memory_header;
	// get the context(stack,register etc.)
	ucontext_t ucp_ref;
	getcontext(&ucp_ref);
	// read / bring the file descriptor for the memory map of the process
	int file_descriptor = open("/proc/self/maps",O_RDONLY);
	//open/create the output checkpoint file
	int output_file = open("./myckpt",O_CREAT | O_WRONLY | O_APPEND, 0666);
	
	if(file_descriptor<0 || output_file<0){
		print_error();	
	}
	write(output_file,&ucp_ref,sizeof(ucp_ref));
	while (read(file_descriptor,&input_char,1)>0) {
		lseek(file_descriptor,-1,SEEK_CUR);
			copy_address(file_descriptor,&memory_header.start);
			copy_address(file_descriptor,&memory_header.end);
    			read(file_descriptor, &rwxp, 4);
    			strcpy(memory_header.rwxp, rwxp);
			read_to_end(file_descriptor,name);
			if(memory_header.rwxp[0]=='r' && (strstr(name,"vvar") != NULL || strstr(name,"vdso") != NULL || strstr(name,"vsyscall") != NULL)) {
				unsigned long int length = memory_header.end - memory_header.start;
				printf("%lu\n",length);
				printf("%p-%p\n",memory_header.start,memory_header.end);
				write(output_file,&memory_header,sizeof(struct header));
				write(output_file,&memory_header.start,memory_header.end-memory_header.start);	
			}
			name[0]='\0';
	}
	close(output_file);
	close(file_descriptor);
}

__attribute__((constructor))
void myconstructor() {
	signal(SIGUSR2,sig_handler);
}
