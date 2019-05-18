//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//              This class processes an FTP transactions.
// 
//****************************************************************************



#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>

#include "common.h"

#include "ClientConnection.h"





ClientConnection::ClientConnection(int s) {
    int sock = (int)(s);
  
    char buffer[MAX_BUFF];

    control_socket = s;
    // Check the Linux man pages to know what fdopen does.
    fd = fdopen(s, "a+");
    if (fd == NULL){
	std::cout << "Connection closed" << std::endl;

	fclose(fd);
	close(control_socket);
	ok = false;
	return ;
    }
    
    ok = true;
    data_socket = -1;
   
  
  
};


ClientConnection::~ClientConnection() {
 	fclose(fd);
	close(control_socket); 
  
}


int connect_TCP( uint32_t address,  uint16_t  port) {
     // Implement your code to define a socket here
	struct sockaddr_in sin;
	struct hostent *hent;
	int s;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET; 
	sin.sin_port = htons(port);

	sin.sin_addr.s_addr = address;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		errexit("No se puede crear el socket: %s\n", strerror(errno));

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errexit("No se puede conectar con %d: %s\n", address, strerror(errno));


	return s;
/////////////////////////////////////////////////////////////////////////////77
    return -1; // You must return the socket descriptor.

}






void ClientConnection::stop() {
    close(data_socket);
    close(control_socket);
    parar = true;
  
}





    
#define COMMAND(cmd) strcmp(command, cmd)==0

// This method processes the requests.
// Here you should implement the actions related to the FTP commands.
// See the example for the USER command.
// If you think that you have to add other commands feel free to do so. You 
// are allowed to add auxiliary methods if necessary.

void ClientConnection::WaitForRequests() {
    if (!ok) {
	 return;
    }
    
    fprintf(fd, "220 Service ready\n");
  
    while(!parar) {


 
      fscanf(fd, "%s", command);
      if (COMMAND("USER")) {
	    fscanf(fd, "%s", arg);

	    // fprintf(fd, "HOLA USUARIO: %s\n", arg);
	    if (strcmp(arg, "jonas") == 0)
		    fprintf(fd, "331 User name ok, need password.\n");

		else
		 	fprintf(fd, "530 Not logged in.\n");

      }
      //Comando SYST
      else if (COMMAND("PASS")) {
		fscanf(fd, "%s", arg);

	    if (strcmp(arg, "XXXX") == 0)
		    fprintf(fd, "230 User logged in, proceed.\n");

		else
		 	fprintf(fd, "530 Not logged in.\n");
      }
      //Comando SYST 
      else if (COMMAND("SYST")) {
      	/**
      	 *
      	 * Con el comando #ifdef podemos detectar el sistema operativo. En el caso de __linux__ detectamos si es un linux, en caso de serlo se ejecutaría esa parte del codigo, en caso de no se pasaría el siguiente if.
      	 *
      	 */
      	
		#ifdef __linux__ 
		    fprintf(fd, "215 UNIX system type.\n");
		#elif _WIN32
		    fprintf(fd, "215 MSDOS system type.\n");
		#else
		    fprintf(fd, "450 Error, system busy.\n");
		#endif
      }
      //Comando TYPE
      /**
       *
       * Se define el tipo de codificación del enlace de datos. Se reconocen los siguientes Tipos:
		*TYPE A N TYPE ASCII NON PRINT. Se utiliza para transferir ficheros de texto. 'N'es opcional y puede no existir.
		*TYPE A T TYPE ASCII TELNET. Se utiliza para transferir ficheros de texto. (Nosoportado).
		*TYPE A C TYPE ASCII CARRIAGE. Se utiliza para transferir ficheros de texto. (No soportado).
		*TYPE I TYPE IMAGE. Se utiliza para transferir ficheros binarios.
		*TYPE E TYPE EBCDIC. (No soportado).
		*TYPE L 8 TYPE LOCAL. Solo se soporta LOCAL 8 = IMAGE.
       *
       */
      
    else if (COMMAND("TYPE")) {
    	fscanf(fd, "%s", arg);
	  	if (strcmp(arg, "A") == 0)
	  		fprintf(fd, "200 TYPE is set to ASCII NON PRINT.\n");
	  	
	  	else if (strcmp(arg, "A N") == 0)
	  		fprintf(fd, "200 TYPE is set to ASCII NON PRINT.\n");
	  	
	  	else if (strcmp(arg, "A T") == 0)
	  		fprintf(fd, "200 TYPE is set to ASCII TELNET.\n");

	  	else if (strcmp(arg, "A C") == 0)
	  		fprintf(fd, "504 TYPE ASCII CARRIAGE not supported.\n");

	  	else if (strcmp(arg, "I") == 0)
	  		fprintf(fd, "200 TYPE is set to IMAGE.\n");

	  	else if (strcmp(arg, "E") == 0)
	  		fprintf(fd, "504 TYPE EBCDIC not supported.\n");

	  	else if (strcmp(arg, "L 8") == 0)
	  		fprintf(fd, "200 TYPE is set to LOCAL 8.\n");

	  	else if (strcmp(arg, "") == 0)
	  		fprintf(fd, "450 Error, system busy.\n");

	  	else
	  		fprintf(fd, "501 TYPE argument error.\n");

    }

      //Formas de abrir el socket
      //Comando PORT para abrir el socket en modo activo
      else if (COMMAND("PORT")) {
      	//PORT h0, h1, h2, h3, p0, p1
      	int h0, h1, h2, h3;
      	int p0, p1;
	  	fscanf(fd, "%d,%d,%d,%d,%d,%d", &h0, &h1, &h2, &h3, &p0, &p1);
	  	uint32_t address = h0 << 24 | h1 << 16 | h2 << 8 | h3;
	  	uint32_t port = p0 << 8 | p1;

      }
      else if (COMMAND("PASV")) { //modo pasivo
	  
      }
      /////////////////////////////////////////
      //Cambia el directorio de trabajo al directorio que indica el parámetro. Ahora solo sirve para poder tratar los sistemas de ficheros como directorios, también admite '..' o '/' para subir al directorio raíz.
    else if (COMMAND("CWD")) {
    	fscanf(fd, "%s", arg);
		if (strcmp(arg, "/") == 0 || strcmp(arg, "..") == 0)
		{
			//system("cd /");
			fprintf(fd, "200 CWD root dir successful.\n");  
		}

		else if (strcmp(arg, "") == 0 )
		{
			//system("cd /");
			fprintf(fd, "501 No pathname defined.\n");  
		}

		else if (strcmp(arg, "/") != 0 && strcmp(arg, "..") != 0 && strcmp(arg, "") != 0)
		{
			//system("cd %s", arg);
			fprintf(fd, "200 CWD current dir successful.\n");  
		}

		else
		{
			fprintf(fd, "450 Error, system busy.\n");  
		}
    }
    
    //Envía el nombre del directorio de trabajo. El servidor FTP solo se encuentra implementado para funcionar en el directorio raíz de los sistemas de ficheros. Además, indica el sistema de ficheros utilizado.
    else if (COMMAND("PWD")) {
	   char arg2 = system("pwd");
	   if (arg2 == '/' == 0 )
		{
			//system("cd /");
			fprintf(fd, "257 %d is current directory\n", arg2);  
		}
		else if(COMMAND("PASSWORD"))
			fprintf(fd, "530 Access denied, not logged in.\n"); 
			
		else 
			fprintf(fd, "450 Error, system busy.\n"); 

    }        
    
    //Comando Quit, sale del sistema y cierra la conexión con el servidor
    else if (COMMAND("QUIT")) {
    	fprintf(fd, "221 Goodbye.\n"); 
    }

      //Para transmitir ficheros
      else if (COMMAND("RETR")) {
	   
      }
      else if (COMMAND("STOR") ) {
	    
      }
      else if (COMMAND("LIST")) {
	
      }
      else  {
	    fprintf(fd, "502 Command not implemented.\n"); fflush(fd);
	    printf("Comando : %s %s\n", command, arg);
	    printf("Error interno del servidor\n");
	
      }
      
    }
    
    fclose(fd);

    
    return;
  
};
