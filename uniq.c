
#include "types.h"
#include "stat.h"
#include "user.h"
/* supports following but not only these: 
 * 1.	uniq example.txt 
 * 2.	cat example.txt | uniq
 * 3.	uniq -i example.txt [or] uniq example.txt -i
 * 4.   uniq -c example.txt [or] uniq example.txt -c 
 * 5.   uniq -d example.txt [or] uniq example.txt -d
 * 6.   uniq [any two or three of -i/-c/-d] example.txt 
 * 7.   uniq [-i/-c/-d] example1.txt example2.txt ...
 */

#define MAX_OUTPUT_LINE 1024
#define BUF_SIZE 2048

int occurs[MAX_OUTPUT_LINE]={0};
//storing count of occurences of each distinct line after being grouped

char ignore_case=0;
char count_lines=0;
char only_dup=0;

//uniq_f processes the input string by collapsing the adjacent identical line into one and also recording the occurences in occurs[]

// Size of collapsed string must be smaller than orginal, so I figure out a method of two pointers, the faster one is used to scan the whole string, the slower one is used to rewrite the string at the exactly same memory space of original string. Other pointers like line_st,line_ed,line_pt are to help locate the history line for comparison use.
char uniq_f(char* str){
	char* str_pt=str; 
	char* line_st=str;
	char* line_pt=str;
	char* line_ed=str;
	int occi = 0;
	int same = 1;
	while(*str_pt!='\n'){
		if(*str_pt==0)break;
		str_pt++;
	}
	occurs[occi] = 1;
	line_ed=str_pt;
	str=++str_pt;
	while(*str_pt!=0&&occi<MAX_OUTPUT_LINE){
		if(same==1&&*str_pt==*line_pt){
			line_pt++;
		}else{
			same=0;
		}
		if(*str_pt=='\n'){
			if(same==1&&line_pt-1==line_ed){
				line_pt=line_st;
				str=line_ed;
				occurs[occi]++;
			}else{
				occurs[++occi]++;
				line_st=line_ed+1;
				line_pt=line_st;
				line_ed=str;
				same=1;
			}			
		}
		*str=*str_pt;	
		str++;
		str_pt++;
	}
	*str=0;
	return 1;
}

//uniq_read function is a bridge between main and uniq_f. After opening the file or pipe in the main, the read happens here. The string in buffer is preprocessed before going to uniq_f and the result from uniq_f is print out here. For example, if -i, then the string will be uppercased before going to uniq_f; if -c, then will print occurence before printing each line; if -d, then the line will be print out only if occurences > 1.
char uniq_read(int fd,char* buf,int size){
	int i;
	char* buf_ptr=buf;
	int readsize;
	if((readsize=read(fd,buf,size))<size){
		buf[readsize]=0;
		if(ignore_case){			
			for(i=0;i<readsize;i++){
				if(buf[i]>='a'&&buf[i]<='z')
					buf[i]+='A'-'a';										
			}
		}
		uniq_f(buf);
		if(!(count_lines||only_dup))
			printf(1,"%s",buf);
		else{
			for(i=0;i<MAX_OUTPUT_LINE&&occurs[i]!=0;i++){
				if(count_lines&&!(only_dup&&occurs[i]==1)){
					printf(1,"%d ",occurs[i]);
				}
				while(*buf_ptr!=0){
					if(!(only_dup&&occurs[i]==1))
						printf(1,"%c",*buf_ptr);
					if(*buf_ptr++=='\n')
						break;
				}
			}
		}
		return 1;
	}else{
     		printf(1, "uniq:exceeding max input bytes %s",BUF_SIZE);
		return 0;
	};
}


int main(int argc,char* argv[]){
	int fd;
	int argi;
	int argfc=argc;
	char buf[BUF_SIZE]={0};
	for(argi=1;argi<argc;argi++){     
		if(argv[argi][0]=='-'){
			argfc--;
			switch(argv[argi][1]){
			case 'c':count_lines=1;break;
			case 'i':ignore_case=1;break;
			case 'd':only_dup=1;break;
			default:break;
			}
		}
	}
	if( argfc <= 1 ){     // from stdin/pipe
		uniq_read(0,buf,sizeof(buf));
	}
	else {
		for(argi=1;argi<argc;argi++){
			if(argv[argi][0]!='-'){
				if((fd = open(argv[argi],0))<0){
      					printf(1, "uniq: cannot open %s\n", argv[argi]);
					exit();	
				}
				uniq_read(fd,buf,sizeof(buf));
				close(fd);
			}
		}
	}
	exit();
}
