#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>


#define BUFFER (1024)

int main(int argc,char * argv[])
{
	int portno;
	if (argc < 3){
		perror("ERROR command line arguments\n");
		exit(1);
	}
	int index=0;
	char * str = argv[1];
	while(argv[1][index]!= ':')index++;
	index++;
	if(sscanf(&argv[1][index],"%d",&portno)== 0 ){
		perror("ERROR portnumber scan\n");
		exit(1);
	}
	char * buf = malloc(sizeof(char)*BUFFER);
	if(buf == NULL){
		perror("ERROR allocation\n");
		exit(1);
	}

	bzero(buf,BUFFER);
	char servername[100];
	bzero(servername,100);
	strncpy(servername,argv[1],--index);
	int connectfd;
	struct sockaddr_in server;
	struct hostent * hostent;
	struct servent* servent;
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	if((hostent=gethostbyname(servername))==NULL){
		perror("ERROR get hos by name\n");
		exit(1);
	}
	memcpy(&server.sin_addr,hostent->h_addr,hostent->h_length);
	if((servent=getservbyname(servername,"tcp"))!=NULL)
		server.sin_port=servent->s_port;
	else
		server.sin_port=htons(portno);	
	if((connectfd=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("ERROR on socket creation\n");
		exit(1);
	}
	int i;
	if(connect(connectfd,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){
		perror("ERROR on socket connecting\n");
		exit(1);
	}
	int n;
	i=2;
	char * tempik = buf;
	int velkost = BUFFER;
	for(;i<argc;i++){
		n=sprintf(tempik,"%d\n",atoi(argv[i]));
		if(i == velkost -1){
			buf = realloc(buf,2*velkost);
			if(buf == NULL){perror("ERROR realloc1\n");	exit(1);}
			tempik = buf + velkost;
			velkost *=2;
		}
		tempik+=n;
	}
	if(write(connectfd,buf,strlen(buf))< 0 ){
		perror("ERROR on writing\n");
		exit(1);
	}
	bzero(buf,velkost);
	char * tempovec = buf;
	int k,N=0;
	while((N = read(connectfd,tempovec,BUFFER))==BUFFER){
		tempovec += BUFFER;
		if((k = tempovec - buf) == velkost){
			buf = realloc(buf,velkost * 2);
			if(buf == NULL){perror("ERROR realloc2\n");	exit(1);}
			tempovec = buf + k;
			velkost *= 2;
		}
	}
	char * new_buf = malloc (sizeof(char)* velkost);
	if(new_buf == NULL){perror("ERROR realloc\n");	exit(1);}
	bzero(new_buf,velkost);
	int error;
	i=0;
	index = 0;
	char tmp[100];

	while(buf[i] !=0){
		if(buf[i]=='#'){
			i++;
			sscanf(&buf[i],"%d",&error);
			bzero(tmp,100);
			sprintf(tmp,"%d",error);
			i+=strlen(tmp);
			fprintf(stderr,"Chyba: nezname PSC %d\n",error);
		}	
		else{
			new_buf[index]=buf[i];
			i++;
			index++;	
		}
	}
	new_buf[++index]='\0';
	printf("%s",new_buf);
	close(connectfd);
	return 0;
}
