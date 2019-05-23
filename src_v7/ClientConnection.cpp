    
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
#include "FTPServer.h"




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


int connect_TCP( uint32_t address,  uint16_t  port) 
{
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
			//Con el comando #ifdef podemos detectar el sistema operativo. En el caso de __linux__ detectamos si es un linux, en caso de serlo se ejecutaría esa parte del codigo, en caso de no se pasaría el siguiente if.
			#ifdef __linux__ 
				fprintf(fd, "215 UNIX Type: L8.\n");
			#else
				fprintf(fd, "450 Error, system busy.\n");
			#endif
		}

	  //Comando TYPE
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
			uint16_t port;
			int p0, p1, s;
			struct sockaddr_in sin;
			socklen_t slen;

			s = define_socket_TCP(0);
			slen = sizeof(sin);
			getsockname(s, (struct sockaddr *)&sin, &slen);
			port = sin.sin_port;
			port = p0 << 8 | p1;
			fprintf(fd, "227 Entering Passive Mode. (127,0,0,1,%d,%d)", p0, p1);
			fflush(fd);			
			data_socket = accept(s, (struct sockaddr*)&sin, &slen);
			
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
			char cadena[MAX_BUFF];
			int i;
			fscanf(fd, "%s", arg2);
			if (logged_in == true )
			{	
				fprintf(fd, "150 File status okay; about to open data connection\n");				
				FILE* f;
				f = fopen(arg2, "r");
				i = 200;
				do
				{
					i = fread(cadena,sizeof(char), ftell(f), f);
					send(data_socket, cadena, ftell(f), 0);
				}while(i == MAX_BUFF);

				fclose(f);
				fprintf(fd, "226 Closing data connection.\n");
				close(data_socket);

			}

			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n"); 

		}
		else if (COMMAND("STOR") ) 
		{
			char arg2[MAX_BUFF];
			fscanf(fd, "%s", arg2);
			int cadena[MAX_BUFF];
			int i;
			if (logged_in == true )
			{	
				fprintf(fd, "150 File status okay; about to open data connection\n");
				fflush(fd);
				
				FILE* f;
				f = fopen(arg2, "r");
				i = 200;
				do
				{
					i = recv(data_socket,cadena, ftell(f), 0);
					i = write(data_socket, f, ftell(f));
				}while(i == MAX_BUFF);

				fclose(f);
				fprintf(fd, "226 Closing data connection.\n");
				close(data_socket);
			}

			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n");
			
		}


		else if (COMMAND("LIST")) 
		{
			if (logged_in == true )
			{	
				char arg2[MAX_BUFF];
				fscanf(fd, "%s", arg2);

				if ( strlen(arg2)==0)
				{
					struct dirent* h;
					char cadena[MAX_BUFF];
					std::string arch_dir_;
					fprintf(fd, "125 Data connection already open; transfer starting\n");		
					DIR *d = opendir(".");
					FILE* f = popen("ls", "r");

					while(readdir(d) != NULL)
					{
						if(fgets(cadena, MAX_BUFF, f) != NULL)
							arch_dir_.append(cadena);
					}

					send(data_socket, arch_dir_.c_str(), arch_dir_.size(), 0);
					closedir(d);
					fclose(f);
					fprintf(fd, "226 List transfer completed, data connection is closed.\n");
					close(data_socket);				
				}

				else
				{
					printf("Es necesario un argumento para el ls.\n");
					fprintf(fd, "550 Error ending search.\n");
					return;
				}	
			}
			else if(logged_in == false)
				fprintf(fd, "530 Access denied, not logged in.\n"); 
				
			else 
				fprintf(fd, "450 Error, system busy.\n");
		}

		else  
		{
			fprintf(fd, "502 Command not implemented.\n"); 
			fflush(fd);
			printf("Comando : %s %s\n", command, arg);
			printf("Error interno del servidor\n");
		
		}
		  
	}
	
	fclose(fd);

	
	return;
  
};