
			/*VERSAO DE 13-OUT*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


typedef struct dadosap{
	char campodecontrolo;
	char n_de_sequecia;
	char l2;
	char l1;
	char * dados;

} dados_aplicacao;


typedef struct tlv{
	char type;
	char lenght;
	char * value;
} tlv;


typedef struct controlo{
	char campodecontrolo;
	tlv * dado;
	int n_de_dados;
} controlo;



typedef struct trama_inf{
	char flag;
	char campo_end;
	char controlo;
	char bcc1;
	dados_aplicacao * dados;
	char bcc2;

} trama_informacao;

typedef struct trama_controlo{
	char flag;
	char campo_end;
	char controlo;
	char bcc1;
	controlo * campo_de_controlo;
	char bcc2;

} trama_controlo;

typedef struct trama_sup{
	char flag;
	char campo_end;
	char controlo;
	char bcc1;

} trama_sup;

int flag=0, conta=2;



void retry(){
	printf("Tentativa %d\n", conta);
	flag=1;
	conta++;
}

int readfile(int fd_com,char * filename, int lenght){
	int fd_file;
	char * buffer=malloc(lenght+1);
	fd_file=open(filename,O_RDONLY);
	if(fd_file<0){
		return fd_file;
	}
	char dados[]="parte: ";
	int lido;
	while(lido=read(fd_file,buffer,lenght)){
	buffer[lido]='\n';
	write(1,dados,7);	
	write(1,buffer,lido+1);
	printf("read: %d bytes\n",lido);
	//printf("\n");
}
	return 0;

}




int llwrite(int fd, char * buffer, int lenght){
}

int llread(int fd, char * buffer){

}

int llclose(int fd){
}

int llopen(int porta, int transmitter){
	int fd,c, res;
	char str[15]="";
   	char buf[5];
	int STOP=FALSE;
	char num1[2]={porta+'0','\0'};
	strcat(str,"/dev/ttyS");
	strcat(str,num1);
	//printf("%s\n",str)
	signal(SIGALRM, retry);
	struct termios oldtio,newtio;
	
	if(transmitter==1){
 		fd = open(str, O_RDWR | O_NOCTTY |O_NONBLOCK);
	}else{
		fd = open(str, O_RDWR | O_NOCTTY);
	}
	
	//int fd2=open(argv[1], O_RDWR | O_NOCTTY);
	if (fd <0) {
		perror(str); 
		exit(-1); 
	}

    	if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      		perror("tcgetattr");
 		exit(-1);
    	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

	tcflush(fd, TCIOFLUSH);

    	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      		perror("tcsetattr");
      		exit(-1);
	}

	if(transmitter==1){	
		/*
		*Parte do emissor.
		*/
		buf[0]=0x7E;
		buf[1]=0x03;
		buf[2]=0x03;
		buf[3]=buf[1]^buf[2];
		buf[4]=0x7E;
		alarm(3);

		do{
			res = write(fd,buf,5);
			alarm(3); 
			int state=0;
			char received[5];
			while (STOP==FALSE && flag==0) {       /* loop for input */   
		   		
				char a;
				res = read(fd,&a,1);   /* returns after 5 chars have been input */   
				
				if(res==1){			
					if(state==0 && a==0x7E){
						printf("Entrou0\n");
						received[state]=a;			
						state=1;
					}else if(state==1 && a==0x03){
						printf("Entrou1\n");	
						received[state]=a;			
						state=2;
					}else if(state==2 && a==0x07){
						printf("Entrou2\n");
						received[state]=a;			
						state=3;
					}else if(state==3 && a==received[1]^received[2]){
						printf("Entrou3\n");
						received[state]=a;			
						state=4;
					}else if(state==4 && a==0x7E){
						printf("Entrou4\n");
						received[state]=a;
					}else{
						printf("Sinal errado estado:%d\n recebeu: %x",state,a);
						state=0;
					}
		
					if(a==0x7E && state==4)	
						STOP=TRUE;
		    			}
				}
			
				printf("Saiu\n");
				flag=0;

		} while(STOP==FALSE);

	} else {
		/*
		*Parte do receptor.
		*/
	
	    	int i = 0;
	    	int estado = 0;
	    
		while (STOP == FALSE){       /* loop for input */

	   		char a;
			res = read(fd,&a,1);   /* returns after 5 chars have been input */   
	
			if(estado == 0 && a == 0x7E){
				printf("ENTROU_0\n");
				estado = 1;
				buf[0] = a;
			} else if(estado == 1 && a == 0x03){
				printf("ENTROU_1\n");
				estado = 2;
				buf[1] = a;
			} else if(estado == 2 && a == 0x03){
				printf("ENTROU_2\n");
				estado = 3;
				buf[2] = a;
			} else if(estado == 3 && a == buf[1]^buf[2]){
				printf("ENTROU_3\n");
				estado = 4;
				buf[3] = a;
			} else if(estado == 4 && a == 0x7E){
				printf("ENTROU_4\n");
				buf[4] = a;
			} else {
				printf("ESTADO INVALIDO\n");
				estado = 0;
		}
	 
		if(a == 0x7E && estado == 4)	
			STOP = TRUE;
	    	}

		char ua[5];

		ua[0] = 0x7E;
		ua[1] = 0x03;
		ua[2] = 0x07;
		ua[3] = ua[1]^ua[2];
		ua[4] = 0x7E;
		res = write(fd,&ua,5);
	}

	if(STOP==TRUE){
		return fd;
	}else{
		return -1;
	}

}

int main(int argc, char *argv[]){
	/*int a,b;
	
	printf("trasmitter: ");
	scanf(" %d",&a);
	printf("porta: ");
	scanf(" %d",&b);
	llopen(b,a);*/
	readfile(0,argv[1],12);
	
	return 0;
}
