#include <stdio.h>
//Librerias para Serial
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#define mask1 0x000000FF
#define mask2 0x0000FF00
#define mask3 0x00FF0000
#define mask4 0xFF000000

#define DEBUG

int changeEndianess(unsigned int original){
  unsigned int new;
  new = ((original&mask1)<<24)|((original&mask2)<<8)|((original&mask3)>>8)|((original&mask4)>>24);
  return new;
}

void sendILV(int fd, char* data, int dataSize){
    unsigned int headerData;
    unsigned char singleChar;
    int reportedSize=dataSize;
    int complementSize=-(dataSize+1);

  /* Enviar SYNC escrito en LittleEndian para linux */
    //headerData=0x53594E43;
    headerData=0x434E5953;
    write(fd,&headerData,4);

  /* Enviar tamaño de datos y complemento a 2 */
    write(fd,&reportedSize,4);
    write(fd,&complementSize,4);

  /* Enviar comando */
    write(fd,data,dataSize);

  /* Enviar terminación */
    singleChar=0x45;
    write(fd,&singleChar,1);
    singleChar=0x4E;
    write(fd,&singleChar,1);
#ifdef DEBUG
    printf("\n-------%08x%08x%08x",changeEndianess(headerData),changeEndianess(reportedSize),changeEndianess(complementSize));
    int i;
    for(i=0;i<dataSize;i++)printf("%02x",data[i]);
    printf("454E-------\n");
#endif
}

int openPort(struct termios * oldtio){
    char* puerto="/dev/ttyACM0";
    unsigned int baudrate = 0000017;
    struct termios newtio;
    int fd = open(puerto, O_RDWR | O_NOCTTY );
    if (fd <0) {
        perror(puerto);
        printf("No se pudo abrir el puerto");
        return fd;
    }

    tcgetattr(fd,oldtio); //Salvar la configuración actual del puerto
    bzero(&newtio, sizeof(newtio));
    newtio.c_iflag = IGNBRK; //| IGNPAR;
    newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    newtio.c_oflag = 0;
  /* Establecer modo de entrada (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1; //Esperar .1s para recibir respuesta
    newtio.c_cc[VMIN]     = 0;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    return fd;
}

void getInfo(){
    int fd;
    struct termios oldtio;
    unsigned char datos[4];

    fd = openPort(&oldtio);
    datos[0]=0x05;
    datos[1]=0x01;
    datos[2]=0x00;
    datos[3]=0x2F;
    sendILV(fd, datos, 4);

    unsigned char resp;
    int bytes;
    int tries,maxtries=1000;
    for(tries=0;tries<maxtries;tries++){
        ioctl(fd, FIONREAD, &bytes);

//        printf("\nIntento %d bytes: %d\n",tries,bytes);

        if(bytes<=0){//return;
        }else{
	    int i;
	    for(i=0;i<bytes;i++){
		read(fd,&resp,1);
	//	printf("%x",resp);
	    }
	    break;
        }
    }

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
}

void getImage(){
    int fd;
    struct termios oldtio;
    unsigned char datos[11];
    unsigned short longitud=8,espera;
    unsigned char base,calidad,pasadas,dedos,guardar,tamano;

    fd = openPort(&oldtio);

    base	= 0x00;
    espera	= 10;
    calidad	= 0x00;
    pasadas	= 0x01;
    dedos	= 0x01;
    guardar	= 0x00;
    tamano	= 0x00;

    datos[0]	= 0x21;
    datos[1]	= longitud & mask1;
    datos[2]	= (longitud & mask2)>>8;
    datos[3]	= base;
    datos[4]	= espera&mask1;
    datos[5]	= (espera&mask2)>>8;
    datos[6]	= calidad;
    datos[7]	= pasadas;
    datos[8]	= dedos;
    datos[9]	= guardar;
    datos[10]	= tamano;

 /*   datos[11]=0x00;
    datos[12]=0x00;
    datos[13]=0x00;
    datos[14]=0x00;
    datos[15]=0x00;
    datos[16]=0x00;
    datos[17]=0x00;
    datos[18]=0x00;
    datos[19]=0x00;
    datos[20]=0x00;
*/
    sendILV(fd, datos, 11);

    unsigned char resp;
    int bytes;
    int tries,maxtries=10000;

    for(tries=0;tries<maxtries;tries++){
        ioctl(fd, FIONREAD, &bytes);

//        printf("\nIntento %d bytes: %d\n",tries,bytes);

        if(bytes<=0){//return;
        }else{
	    int i;
	    for(i=0;i<bytes;i++){
		read(fd,&resp,1);
//		printf("%x",resp);
	    }
	    break;
        }
    }


    tcsetattr(fd,TCSANOW,&oldtio);
   close(fd);

}

void funcTest(){
    int fd;
    struct termios oldtio;
    unsigned char datos[4];

    fd = openPort(&oldtio);
    datos[0]=0x05;
    datos[1]=0x01;
    datos[2]=0x00;
    datos[3]=0x2F;
    sendILV(fd, datos, 4);

    unsigned char resp;
    int bytes;
    int tries,maxtries=1000;
    for(tries=0;tries<maxtries;tries++){
        ioctl(fd, FIONREAD, &bytes);

//        printf("\nIntento %d bytes: %d\n",tries,bytes);

        if(bytes<=0){//return;
        }else{
	    int i;
	    for(i=0;i<bytes;i++){
		read(fd,&resp,1);
	//	printf("%x",resp);
	    }
	    break;
        }
    }

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
}


int main()
{
//    getImage();
    funcTest();
    return 0;
}
