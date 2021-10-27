#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdbool.h>
#define BUFFER_SIZE 4096
#define RESPONSE_HEADER "HTTP/1.1 %s\r\nContent-Length: %d\r\n\r\n"
#define RESPONSE_TAIL "\n"
 char *header_field;
 char *body;
 char *method;
 char *resource;
 char *http_version;
 char *descriptor;
  int cl;
  char *host;
  bool erro=false;
/**
   Converts a string to an 16 bits unsigned integer.
   Returns 0 if the string is malformed or out of the range.
 */
uint16_t strtouint16(char number[]) {
  char *last;
  long num = strtol(number, &last, 10);
  if (num <= 0 || num > UINT16_MAX || *last != '\0') {
    return 0;
  }
  return num;
}

/**
   Creates a socket for listening for connections.
   Closes the program and prints an error message on error.
 */
int create_listen_socket(uint16_t port) {
  struct sockaddr_in addr;
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    err(EXIT_FAILURE, "socket error");
  }

  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr*)&addr, sizeof addr) < 0) {
    err(EXIT_FAILURE, "bind error");
  }

  if (listen(listenfd, 500) < 0) {
    err(EXIT_FAILURE, "listen error");
  }

  return listenfd;
}

size_t open_file2(int connfd,char *filename){
    size_t ret=0;
    size_t file_len;
    char buffer[BUFFER_SIZE];
    int fo=open(filename,O_RDWR,0666);
    lseek(fo,0,SEEK_SET);
    while(1){
      file_len= read(fo, buffer, BUFFER_SIZE);
      //printf("file_len=%zu\n",file_len);
     if(send(connfd,buffer,file_len,0)){
        bzero(&buffer,BUFFER_SIZE);
     }
       ret=ret+file_len;
     if(file_len<BUFFER_SIZE){
       break;
     }
    }
    close(fo);
    return ret;
}
size_t get_file_length(char* filename){

    size_t ret=0;
    size_t file_len;
    char buffer[BUFFER_SIZE];
    int fo=open(filename,O_RDWR,0666);
    lseek(fo,0,SEEK_SET);
    while(1){
      file_len= read(fo, buffer, BUFFER_SIZE);
      //printf("file_len=%zu\n",file_len);
       bzero(&buffer,BUFFER_SIZE);
       ret=ret+file_len;
      if(file_len<BUFFER_SIZE){
       break;
     }
    }
    close(fo);
    return ret;
}
int create_file(int connfd,char* filename){
     size_t ret=0;
     size_t file_len;
     char buffer[BUFFER_SIZE];
    int fo=open(filename,O_RDWR+O_CREAT,0666);
    lseek(fo,0,SEEK_SET);
    //printf("cl=%d\n",cl);
    if(cl==0){
      return 0;
      close(fo);
    }
    while(1){
      file_len= recv(connfd, buffer, BUFFER_SIZE, 0);
      if(file_len==-1){
          break;
      }
      //printf("file_len=%zu\n",file_len);
     ret=ret+file_len;
     if(write(fo,buffer,file_len)){
       bzero(&buffer,BUFFER_SIZE);
     }

     if(file_len<BUFFER_SIZE){
       break;
     }
    }

    close(fo);
    return ret;

}
void response_400(char** msg,char**descriptor){
    *msg="400 Bad Request";
    *descriptor="Bad Request\n";
    erro=true;
}
void response_403(char** msg,char**descriptor){
    *msg="403 Forbidden";
    *descriptor="Forbidden\n";
    erro=true;
}
void response_404(char** msg,char**descriptor){
    *msg="404 File Not Found";
    *descriptor="File Not Found\n";
    erro=true;

}
void response_200(char** msg,char**descriptor){
    *msg="200 OK";
    *descriptor="OK\n";

}
void response_201(char** msg,char**descriptor){
    *msg="201 Created";
    *descriptor="Created\n";
    erro=true;
}
void response_500(char** msg,char**descriptor){
    *msg="501 Internal Server Error";
    *descriptor="Internal Server Error\n";
    erro=true;
}
void response_501(char** msg,char**descriptor){
    *msg="501 Not Implemented";
    *descriptor="Not Implemented\n";
    erro=true;
}
bool is_valid(char* resource_name){
  //resource_name=resource_name+1;
    int l=strlen(resource_name);
    if(l>19||l<1){
      return false;
    }
    for(int i=0;i<l;i++){
      if(((resource_name[i] >=65)&&(resource_name[i]<=90))||((resource_name[i] >=97)&&(resource_name[i]<=122))|| ((resource_name[i] >=48)&&(resource_name[i]<=57))||(resource_name[i]==46)||(resource_name[i]==95)){
        //printf("resource_name[i]=%c\n",resource_name[i]);
            continue;
      }
        else{
          //printf("resource_name[i]=%c\n",resource_name[i]);
          //printf("none ascii\n");
          return false;
        }
    }
    return true;
}
bool have_space(char* host_name){
  for(int i=0;i<strlen(host_name);i++){
    if(host_name[i]==' '){
      return true;
    }
  }
  return false;

}
int make_response(char **response,char *msg,int file_length,int connfd){

 char* filename=resource+1;
 char response_buffer[BUFFER_SIZE];
 char *file_buffer;
 if(!is_valid(filename)){
   response_400(&msg,&descriptor);
   file_length=strlen(descriptor);

 }
 else if(strcmp(http_version,"HTTP/1.1\r")!=0){
   response_400(&msg,&descriptor);
    file_length=strlen(descriptor);
 }
else if(have_space(host)||host==NULL){
   response_400(&msg,&descriptor);
    file_length=strlen(descriptor);
 }

else if(strcmp(method,"PUT")==0){

      int fl=create_file(connfd,filename);
      printf("fl=%d cl=%d",fl,cl);
      if((fl==-1)||(fl!=cl)){
        response_500(&msg,&descriptor);
        file_length=strlen(descriptor);
      }
      else{
        response_201(&msg,&descriptor);
        file_length=strlen(descriptor);
      }

}
else if(strcmp(method,"GET")==0||strcmp(method,"HEAD")==0){
  if(access(filename,F_OK)==-1){
     response_404(&msg,&descriptor);
     file_length=strlen(descriptor);

  }
  else if(access(filename,(R_OK | W_OK))==-1){
      response_403(&msg,&descriptor);
      file_length=strlen(descriptor);

 }
  else{
    response_200(&msg,&descriptor);
    file_length=get_file_length(filename);
 }
}
else{
  response_501(&msg,&descriptor);
  file_length=strlen(descriptor);
}
 memset(response_buffer, 0, sizeof(response_buffer));
	sprintf(response_buffer, RESPONSE_HEADER, msg, file_length);
int headlen = strlen(response_buffer);
	int taillen = strlen(RESPONSE_TAIL);
	int totallen = headlen;
	*response = (char *) malloc(totallen);
  char *tmp = *response;
	memcpy(tmp, response_buffer, headlen);
  send(connfd, tmp, totallen, 0);
  if(erro==true){
    send(connfd,descriptor,strlen(descriptor),0);
    return totallen;
  }
  if((strcmp(method,"GET")==0)&&(erro==false)){
    file_length=open_file2(connfd,filename);
  }
 return totallen;
}

void get_response(char* method,int connfd){
  char *response;
  char *msg;
  int file_length=0;
  int response_len=make_response(&response,msg,file_length,connfd);
  response_len=strlen(response);
	free(response);
}

void handle_request(char *buffer,int connfd){
  int len=strlen(buffer);
  char *buffer_string=(char*)malloc((len+1)*sizeof(char));
  memcpy(buffer_string,buffer,len+1);
  char* request_string;
  for(int i=0;i<strlen(buffer_string)-1;i++){
    if(buffer_string[i]=='\n'&&buffer_string[i+1]=='\n'){
      buffer_string[i+1]='|';
    }
  }
 char *request_line=strtok(buffer_string,"\n");
header_field=strtok(NULL,"|");
 body=strtok(NULL,"|");
 method=strtok(request_line," ");
 resource=strtok(NULL," HTTP");
 http_version=strtok(NULL," ");
 char *content_length_field="Content-Length";
 char *host_field="Host";
 // printf("1.http_version=%s\n",http_version);
 // printf("2.method=%s\n",method);
 // printf("3.resource=%s\n",resource);
//  printf("4.header_field=%s\n",header_field);
char *tmp=(char*)malloc(strlen(header_field)*sizeof(char));
char *p=NULL;
p=tmp;
strcpy(p,header_field);
char *hp=strstr(p,host_field);
char *hs = hp;
while ((*hp!='\r')&&(*hp!=0)){
        hp++;
}
if (*hp== '\r'){
        *hp= 0;
}
host=hs;

 char *pt=strstr(header_field,content_length_field);
if(pt!=NULL){
  char *content_length=strstr(pt," ");
  cl=atoi(content_length);
}
if(tmp!=NULL){
  free(tmp);
  tmp=NULL;
}
if(p!=NULL){
  p=NULL;
}
get_response(method,connfd);


}

void handle_connection(int connfd) {
  // do something
  int len = 0;
  char buffer[BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);
  len = recv(connfd, buffer, BUFFER_SIZE, 0);
  if (len <= 0 ) {
    return;
  } else {
    handle_request(buffer,connfd);
    //printf("Debug request:\n--------------\n%s\n\n",buffer);
  }
  memset(buffer,0x00,sizeof(buffer[0]*BUFFER_SIZE));

  // when done, close socket
  close(connfd);
}
int main(int argc, char *argv[]) {

  int listenfd;
  uint16_t port;

  if (argc != 2) {
    errx(EXIT_FAILURE, "wrong arguments: %s port_num", argv[0]);
  }
  port = strtouint16(argv[1]);
  if (port == 0) {
    errx(EXIT_FAILURE, "invalid port number: %s", argv[1]);
  }
  listenfd = create_listen_socket(port);

  while(1) {
    erro=false;
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
      warn("accept error");
      continue;
    }
    //printf("my http server begin\n");
    handle_connection(connfd);
  }
  return EXIT_SUCCESS;
}

