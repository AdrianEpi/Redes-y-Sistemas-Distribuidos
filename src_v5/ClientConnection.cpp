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
#include <fstream>
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
	//return -1; // You must return the socket descriptor.

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
	bool logged_in;

	if (!ok)
		return;
	
	fprintf(fd, "220 Service ready\n");
  
	while(!parar) 
	{
		fscanf(fd, "%s", command);

		//Comando de logueo (solo usuario)
		if (COMMAND("USER")) 
		{
			fscanf(fd, "%s", arg);
				
			if (strcmp(arg, "jonas") == 0)
			fprintf(fd, "331 User name ok, need password.\n");

			else
				fprintf(fd, "530 Not logged in.\n");
		}

		//Comando de logueo (contraseña)
		else if (COMMAND("PASS")) 
		{
			fscanf(fd, "%s", arg);

			if (strcmp(arg, "XXXX") == 0)
			{
				fprintf(fd, "230 User logged in, proceed.\n");
				logged_in = true;
			}

			else
				fprintf(fd, "530 Not logged in.\n");
		}

		//Comando de reconocimiento de sistema 
		else if (COMMAND("SYST")) 
		{
			/**
			 *
			 * Con el comando #ifdef podemos detectar el sistema operativo. En el caso de __linux__ detectamos si es un linux, en caso de serlo se ejecutaría esa parte del codigo, en caso de no se pasaría el siguiente if.
			 *
			 */
		
			#ifdef __linux__ 
				fprintf(fd, "215 UNIX Type: L8.\n");
			#else
				fprintf(fd, "450 Error, system busy.\n");
			#endif
		}

	  //Comando TYPE
	  /**
	   * Se define el tipo de codificación del enlace de datos. Se reconocen los siguientes Tipos:
		*TYPE A N TYPE ASCII NON PRINT. Se utiliza para transferir ficheros de texto. 'N'es opcional y puede no existir.
		*TYPE A T TYPE ASCII TELNET. Se utiliza para transferir ficheros de texto. (Nosoportado).
		*TYPE A C TYPE ASCII CARRIAGE. Se utiliza para transferir ficheros de texto. (No soportado).
		*TYPE I TYPE IMAGE. Se utiliza para transferir ficheros binarios.
		*TYPE E TYPE EBCDIC. (No soportado).
		*TYPE L 8 TYPE LOCAL. Solo se soporta LOCAL 8 = IMAGE.
	   */
	  
		else if (COMMAND("TYPE")) 
		{
			fscanf(fd, "%s", arg);
			fprintf(fd, "200 OK.\n");

		}

		//Formas de abrir el socket
		//Comando PORT para abrir el socket en modo activo
		else if (COMMAND("PORT")) 
		{
			int h0, h1, h2, h3;
			int p0, p1;
			fscanf(fd, "%d,%d,%d,%d,%d,%d", &h0, &h1, &h2, &h3, &p0, &p1);
			uint32_t address = h3 << 24 | h2 << 16 | h1 << 8 | h0;
			uint32_t port = p0 << 8 | p1;
			data_socket = connect_TCP(address, port);
			fprintf(fd, "200 OK.\n");
		}

		//Comando para abrir el modo pasivo
		else if (COMMAND("PASV")) 
		{
		
		}

		//Cambia el directorio de trabajo al directorio que indica el parámetro. Ahora solo sirve para poder tratar los sistemas de ficheros como directorios, también admite '..' o '/' para subir al directorio raíz.
		else if (COMMAND("CWD")) 
		{
			fscanf(fd, "%s", arg);
			if (strcmp(arg, "/") == 0 || strcmp(arg, "..") == 0)
				fprintf(fd, "200 CWD root dir successful.\n");  

			else if (strcmp(arg, "") == 0 )
				fprintf(fd, "501 No pathname defined.\n");  


			else if (strcmp(arg, "/") != 0 && strcmp(arg, "..") != 0 && strcmp(arg, "") != 0)
				fprintf(fd, "200 CWD current dir successful.\n");  


			else
				fprintf(fd, "450 Error, system busy.\n");  
		}
		
		//Envía el nombre del directorio de trabajo. El servidor FTP solo se encuentra implementado para funcionar en el directorio raíz de los sistemas de ficheros. Además, indica el sistema de ficheros utilizado.
		else if (COMMAND("PWD")) 
		{
			char arg2[MAX_BUFF];
			
			if (logged_in == true )
			{	
				strcat(arg2, getcwd(arg, sizeof(arg)));
				//printf("%s", arg2);
				fprintf(fd, "257 %s is current directory\n", arg2);
			}

			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n"); 

		}        
		
		//Comando Quit, sale del sistema y cierra la conexión con el servidor
		else if (COMMAND("QUIT")) 
			fprintf(fd, "221 Goodbye.\n"); 
			

		//Para transmitir ficheros
		else if (COMMAND("RETR")) 
		{
			char arg2[MAX_BUFF];
			char cadena[128];
			int i;
			fscanf(fd, "%s", arg2);
			if (logged_in == true )
			{	

				//Faltaría enviar el archivo
				fprintf(fd, "150 File status okay; about to open data connection\n");
				 // Abre un fichero de entrada
				
				FILE * f;
				f = fopen(arg2, "r");
				if (f==NULL) {fputs ("File error",stderr); exit (1);}

				//do
				//{
					fread(cadena,1, ftell(f), f);
					printf("%s \n", cadena);
					i += 128;
				//}while(!f.eof());
				fclose(f);
				printf("El tamaño total EEEEESSSS: %d\n", i);
				fprintf(fd, "226 Closing data connection.\n");
				printf("Pedro pica piedras en la cantera con beelma\n");

			}
			/*if (logged_in == true )
			{	
				//Faltaría enviar el archivo
				fprintf(fd, "150 File status okay; about to open data connection\n");
				 // Abre un fichero de entrada
				
				std::ifstream fe(arg2); 

				// Leeremos mediante getline, si lo hiciéramos 
				// mediante el operador << sólo leeríamos 
				// parte de la cadena:

				do
				{
					fe.getline(cadena, 128);
					printf("%s \n", cadena);
					i += 128;
				}while(!fe.eof());
				fe.close();
				printf("El tamaño total EEEEESSSS: %d\n", i);
				fprintf(fd, "226 Closing data connection.\n");
				printf("Pedro pica piedras en la cantera con beelma\n");

			}*/

			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n"); 

		}
		else if (COMMAND("STOR") ) 
		{
			char arg2[MAX_BUFF];
			fscanf(fd, "%s", arg2);
			if (logged_in == true )
			{	
				if(1)//Existe el fichero
				{

				}
				else
				{

				}
				//Faltaría enviar el archivo
				fprintf(fd, "150 File status okay; about to open data connection.\n");
			}

			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n");
			
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
