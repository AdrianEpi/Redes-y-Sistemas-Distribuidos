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
      
      else if (COMMAND("PASS")) {
		fscanf(fd, "%s", arg);

	      if (strcmp(arg, "XXXX") == 0)
		     fprintf(fd, "230 User logged in, proceed.\n");

		 else
		 	fprintf(fd, "530 Not logged in.\n");
      }

      else if (COMMAND("SYST")) {
	   
      }
      else if (COMMAND("TYPE")) {
	  
      }

      //Formas de abrir el socket
      else if (COMMAND("PORT")) {//modo activo
      	//PORT h0, h1, h2, h3, p0, p1
      	int h0, h1, h2, h3;
      	int p0, p1;
	  	//fscanf(fd, "d", "d", "d", "d", "d", "d", &h0, &h1, &h2, &h3, &p0, &p1);
	  	//uint32_t address = h0 << 24 | h1 << 16 | h2 << 8 | h3;
	  	//uint32_t port = p0 << 8 | p1;

      }
      else if (COMMAND("PASV")) { //modo pasivo
	  
      }
      /////////////////////////////////////////

      else if (COMMAND("CWD")) {
	   
      }
      else if (COMMAND("PWD")) {
	   
      }        
      else if (COMMAND("QUIT")) {
	 
      }

      //PAra transmitir ficheros
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
