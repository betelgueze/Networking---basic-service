#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<sys/types.h>
#include <signal.h>
#include <string.h>

#define BUFFER (1024)
typedef struct
{
	int cislo;
	char mesto[50];
	struct zoznam * next;
}zoznam;
char * get_city(int cislo,zoznam * prvy)
{
	zoznam * tmp = prvy;
	while(tmp != NULL){
		if(tmp->cislo == cislo)return tmp->mesto;
		else tmp = tmp->next;
	}
	return NULL;
}
Xfree(zoznam * prvy)
{
	zoznam * predosly;
	zoznam * tmp = prvy;
	while(tmp != NULL){predosly = tmp; tmp = tmp->next;free(predosly);}
}
char * usage = "Synopsis: server -p port\n";

int main(int argc,char * argv[]){
	int portno;
	FILE * data;
	int c;
	while ((c = getopt (argc, argv, "p:d:")) != -1){
		switch(c){
		case 'p':{
			if(sscanf(optarg,"%d",&portno) == 0){
				perror("ERROR portno no invalid\n");
				exit(1);
			}
		}break;
		case 'd':{
			if((data = fopen(optarg,"r+"))== NULL ){
							perror("ERROR filename invalid\n");
							exit(1);
			}
		}break;
		case '?':{
			perror("ERROR command arguments invalid\n");
			exit(1);
		}break;
        default:{
        	perror("ERROR command arguments invalid\n");
        	exit(1);
        }break;
		}
	}
	//nacitanie dat
	int i;
	zoznam * prvy;
	zoznam * predosly = NULL;

	for (i=0;!feof(data);i++){
		zoznam * temp = malloc(sizeof(zoznam));
		if(temp == NULL){
			perror("ERROR malloc\n");
			exit(1);
		}
		fscanf(data,"%d;",&(temp->cislo));
		int c,index=0;
		while((c = getc(data))!= EOF && c != '\n'){temp->mesto[index]=c;index++;}
		++index;
		temp->mesto[index]='\n';
		++index;
		temp->mesto[++index]='\0';
		if(predosly == NULL){predosly = temp;prvy = temp;}
		else{predosly->next = temp;temp->next=NULL;predosly = temp;}
	}
	int listenfd,connectfd;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
	struct sockaddr_in server;
	struct servent * servent;
	pid_t pid;
	long p;
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGCHLD,&sa,NULL)==-1){
    	perror("ERROR signals\n");
    	exit(1);
    }
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(portno);
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1){
    	perror("ERROR socket\n");
    	exit(1);
    }
	if(bind(listenfd,(struct sockaddr*)&server,sizeof(server))==-1){
    	perror("ERROR bind\n");
    	exit(1);
    }
	if(listen(listenfd,5)==-1){
    	perror("ERROR listen\n");
    	exit(1);
    }
	clilen = sizeof(cli_addr);
	while(1){
		if((connectfd=accept(listenfd,(struct sockaddr *) &cli_addr, &clilen))==-1){
		    	perror("ERROR connect\n");
		    	exit(1);
		}
		if((pid=fork())>0){/*parent*/
			close(connectfd);
		}
		else if(pid==0){/*child*/
			close(listenfd);
			char * buf = malloc(sizeof(char)*BUFFER);
			if(buf == NULL){perror("ERROR alloc\n");	exit(1);}

			memset(buf,0,BUFFER);
			char * new_buf = malloc(sizeof(char)*BUFFER);
			if(new_buf == NULL){perror("ERROR alloc\n");	exit(1);}
			memset(new_buf,0,BUFFER);
			char TMP[100];
			bzero(TMP,100);
			char * new_tmp;
			new_tmp = buf;
			int k,N;
			int velkost = BUFFER;
			while((N = read(connectfd,new_tmp,BUFFER))==BUFFER){
				new_tmp += BUFFER;
				if((k = new_tmp - buf) == velkost){
					buf = realloc(buf,velkost * 2);
					if(buf == NULL){perror("ERROR realloc1\n");	exit(1);}
					new_tmp = buf + k;
					velkost *= 2;
				}
			}
			new_tmp = new_buf;
			char * text;
			char * ukazatel=buf;
			int n,aktualny;
			velkost = BUFFER;
			while(*ukazatel != 0){
				n = sscanf(ukazatel,"%d",&aktualny);
				if(n <= 0)break;
				bzero(TMP,100);
				sprintf(TMP,"%d",aktualny);
				ukazatel += strlen(TMP);
				ukazatel += 1;
				text = get_city(aktualny,prvy);

				if(text == NULL){

					n = sprintf(new_tmp,"#%d\n",aktualny);
					new_tmp +=n;
					if((k=new_tmp - new_buf)==velkost-1){
						new_buf = realloc(new_buf,velkost*2);
						if(new_buf == NULL){perror("ERROR realloc2\n");	exit(1);}
						new_tmp = k + new_buf;
						velkost *=2;
					}
				}
				else
					n = sprintf(new_tmp,"%s\n",text);
					new_tmp += n;
					if((k=new_tmp - new_buf)==velkost-1){
						new_buf = realloc(new_buf,velkost*2);
						if(new_buf == NULL){perror("ERROR realloc2\n");	exit(1);}
						new_tmp = k + new_buf;
						velkost *=2;
					}
			}

			if(write(connectfd,new_buf,strlen(new_buf))<0){
		    	perror("ERROR write\n");
		    	exit(1);
		    }
			close(connectfd);
			exit(0);
		}
		else{
	    	perror("ERROR fork\n");
	    	exit(1);
	    }
	}
	close(listenfd);
	Xfree(prvy);
	return 0;
}
