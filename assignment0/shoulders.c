#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <err.h>
extern int errno;
#define BUFFER_SIZE 8192
static bool
is_digit(char *p)
{
    size_t i, len = strlen(p);

    if(len == 0)
        return false;

    for(i = 0; i < len; ++i)
    {
        
        if(!isdigit(p[i]))
            return false;
    }

    return true;
}

static bool
have_dash(char *p)
{
    size_t i,len = strlen(p);
     for(i=0;i<len;i++){
        if(p[i]=='-'){
           
            return true;
        }
    }
    return false;
}
static void
illegal_line_error(char*e){

    fprintf(stderr,"shoulders: invalid number of lines: '%s'",e);
    exit(1);
}
static void
no_argument_error(){
    fprintf(stderr,"shoulders: option requires an argument -- 'n'\n");
    exit(1);
}
static void
echo(int line){
     size_t c=0;
               
        while(c<line){
            char buffer[BUFFER_SIZE];
            read(STDIN_FILENO,buffer,BUFFER_SIZE);
            write(1, buffer, strlen(buffer));
            
            c++;
            memset(buffer,0,BUFFER_SIZE);
            }
          
            
            

}

static char* 
tokenize(char** input,int line){

            static int count=0;
            char *p=NULL,*token=NULL,*buffer=*input,*linebreak="\n";
            size_t tokenLen=0;
            

            if((!buffer)||(line==0)) {//the buffer is trimed empty
                
                count=0;//reset count
            	return NULL;
            }
            p=strstr(buffer,linebreak);
            if(p)//found line break
                tokenLen=p-buffer; //get the length of token

            else//line break not found
                tokenLen=strlen(buffer);//token length is the buffer
            token=malloc(tokenLen+1);
            memcpy(token,buffer,tokenLen);
            token[tokenLen]='\0';
            *input=p?p+1:NULL; 
            if(count!=0&&buffer){//output a line break when reach eof
            	write(1,"\n",1);
            }
                
            count++;
            return token;

}

int main(int argc, char *argv[]){
    size_t line_to_read,line,i,filecount,count=0;
 
    char *filelist[100];
    
   
         if(argc<2){
            
               no_argument_error();
         }
    
        if(is_digit(argv[1])){
           
           if(atoi(argv[1])<0){ //line number<0 
               illegal_line_error(argv[1]);
           }
           line_to_read=atoi(argv[1]);
        
           
            filecount=argc-2;
            if(filecount==0){//no filename given
             
               echo(line_to_read);
               exit(0);
               
            }
            if(argc>=3){//store the file name
            for(i=0;i<argc-2;i++){
                filelist[i]=argv[i+2];
            }
        }
       }
        else{
            illegal_line_error(argv[1]);
        }
  

    while(count<filecount){
         if(count!=0)
         	write(1,"\n",1);
        line=line_to_read;
        char buffer[BUFFER_SIZE];
        
        
        int fo=open(filelist[count],O_RDONLY);
        if(fo==-1){
           
                   if(have_dash(filelist[count])){
                    echo(line_to_read);
                    close(fo);
                    count++;
                    continue;
                   }
                
            
            warn("%s",filelist[count]);
            close(fo);
            count++;
            continue;
        }
    
    
         while(read(fo,buffer,BUFFER_SIZE)!=0){
            
            int len=strlen(buffer);
          
            char *buffer_string=(char*)malloc((len+1)*sizeof(char));
            char **bufferP=&buffer_string;
            char *token;
            
            memcpy(buffer_string,buffer,len+1);
           
            while((token=tokenize(bufferP,line)) ){
              
                write(1,token,strlen(token));
                
                line--;
                
            }
        

            memset(buffer,0,BUFFER_SIZE);//reset buffer
         }
    
         
            
          
        count++;
        close(fo);
    }
        
    write(1,"\n",1);
}

