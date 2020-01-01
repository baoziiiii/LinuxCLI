
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

/* output the last N(default 10) lines (ignore the tailing blank lines)
 * supports following: 
 * 1.	tail README
 * 2.	tail -N README
 */

#define BUF_SIZE 1024
char buf[BUF_SIZE]={0}; // use streaming read so only require 1K buffer memory

char tail(char* filename, int lines){
	int fd = 0;
	int tmp_file;  // need tmp_file because stdin don't allow second pass
	int read_sz=0 ; //  count for read
	int line_offset = 0; // 
	int line_offset_start = -lines ; //  start line number of output
	int line_offset_end = 0; // end line number of output (ignore tailing blank lines)
	int blank_cnt = 0; // blank line count
	int i = 0;

	if( filename != 0 ){
		fd = open(filename,0);
		if(fd<0)
			return -1;
	}
	tmp_file = open ("tmp_file", O_CREATE | O_RDWR);
	while((read_sz = read(fd,buf,BUF_SIZE))>0){ // first pass to get start and end
		write(tmp_file,buf,read_sz);
		for( i = 0; i<read_sz; i++){
			if(buf[i] == '\n'){
				line_offset_start++;
				blank_cnt++;
				buf[i]=0; 
			}else if(buf[i] != '\0')
				blank_cnt=0;
		}
	}

	close(tmp_file);
	if( fd!=0 )
		close(fd);
	line_offset_end = line_offset_start + lines - blank_cnt + 1; // calculate end line

	tmp_file = open("tmp_file",0);
	while((read_sz=read(tmp_file,buf,BUF_SIZE))>0){ // second pass for output
		for(i=0;i<read_sz;i++){
			if(line_offset_start <= line_offset && line_offset < line_offset_end)
				printf(1,"%c",buf[i]);
			if(buf[i] == '\n')
				line_offset ++ ;
		}
	}
	close(tmp_file);
	unlink("tmp_file"); 
	
	return 0;
}


int main(int argc,char* argv[]){
	int argi;
	int lns = 10; // number of lines, default = 10
	char* name = 0; // filename
	for(argi=1;argi<argc;argi++){  
		if(argv[argi][0]=='-'){
			int tmp = 0;   
			int i=1;
			while(argv[argi][i]<='9' && argv[argi][i]>='0'){ // extract line argument
				tmp *= 10;
				tmp = argv[argi][i] - '0' + tmp;
				i++;
			}
			if( tmp == 0 ){
				printf(1, "tail: wrong argument\n");
				exit();	
			}
			lns = tmp; 
		}else
			name = argv[argi]; // extract filename
	}
	if(tail(name,lns) < 0 )
      		printf(1, "tail: cannot open %s\n", name);
	exit();
}
