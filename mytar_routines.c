#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	int nByte;		//Nº de bytes copiados
	int c, ret;		//Nº del byte leido (int a char)
	

	while(nByte < nBytes && (c = getc(origin)) != EOF)){
		ret = putc( (unsigned char) c, destination);
		//Aunque no va a pasar, comprobamos si se queda sin espacio en el destination
		if(ret == EOF){
			err(3, "putc() ha fallado y no debería");
			return -1;	
		}
		nByte++;
	}
	
	return nByte;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
// https://www.tutorialspoint.com/c_standard_library/c_function_fread.htm
char*
loadstr(FILE * file)
{
	char* readFrom;	//String que lees de file (BUFFER)

	int tam = 0, i;

	i = getc(file);
	if (i == EOF) return NULL;
	else tam++;

	//Recorremos el string primero para saber cuanto ocupa
	//Tiene que revisar que ha llegado al final de la palabra y/o del propio archivo
	while (i != (int) '\0' && i != EOF){
		i = getc(file);
		tam++;
	}
	
	//Asignamos al buffer la dirección de inicio y comprobamos si ha habido fallo
	readFrom = (char*) malloc(tam);
	if(readFrom == NULL) return NULL;

	//Retrocedemos al principio de la palabra
	fseek(file, -tam, SEEK_CUR);

	//Escribimos en el buffer
	fread(readFrom, 1, tam, file);
	
	
	return readFrom;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	int i,j;
	stHeaderEntry* header;
	//... Leemos el número de ficheros (N) del tarFile y lo copiamos en nFiles
	//Primer argumento = direccion de memoria donde se empieza a leer
	//Segundo argumento = quieres leer sizeof int porque nfiles es un int, y lo leemos 1 vez
	//Lees sobre tarFile;
	fread(nFiles, sizeof(int), 1, tarFile);

	/* Reservamos memoria para el array */
	//El tamaño del header multiplicado por lo que valga el valor de nFiles, de su puntero.
	header = (stHeaderEntry *) malloc(sizeof (stHeaderEntry) * (*nFiles));

	if(header==NULL){
		fclose(tarFile);	
	 	return NULL;  
	}

	

	for (i = 0; i < *nFiles; i++) {
	        //.. usamos loadstr para cargar el nombre en header[i].name
		//... comprobación y tratamiento de errores
		//... leemos el tamaño del fichero y lo almacenamos en header[i].size
			
		//Cargamos el nombre del header y se lo asignamos
		if ((header[i].name=loadstr(tarFile))==NULL) {
			//*nFiles = valor del puntero
			for (j = 0; j < *nFiles; j++){
				free(header[j].name);
			}	

			free(header);
			fclose(tarFile);
			return NULL;
			
		}
		//&header.size porque queremos la direccion de memoria
		//Y queremos leer el size del header[i] una sola vez
		fread(&header[i].size, sizeof(header[i].size), 1, tarFile);
	}

	return header;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	FILE *tar, *input;	//tarfile al que vamos a añadir el header.
				//los inputfile se meten con un for.

	stHeaderEntry *header;	//Cabezera estandar, con la que reservamos memoria.
	int headerSize;		//Para saber el tamaño de la cabezera.
	
	//Si no llegan archivos, da error.
	if(nFiles <= 0)  return EXIT_FAILURE; 
	
	//Intentamos abrir el tarfile
	//Tar tendrá la direccion inicial asignada por fopen
	//fopen abre el archivo y si no existe lo crea
	if((tar = fopen(tarName), "wx") == NULL) {
		return EXIT_FAILURE;
	}

	//Intentamos alojar todos los headers en memoria. Si no cabe dará fallo
	//Tamaño de un header multiplicado el número de estructuras
	//En header se guarda la dirección de memeoria en la que se puede empezar a escribir
	if((header = malloc(sizeof(stHeaderEntry) * nFiles)) == NULL){
	
		fclose(tar);
		remove(tarName);
		return EXIT_FAILURE;
	}
	
	//Como mínimo, el tamaño del header tiene el tamaño de un entero
	//Porque nuestro header tiene un array y un int. El array puede ser vacio, pero el int no.
	//(No va a pasar, pero es una medida de seguridad)
	HeaderSize = sizeof(int);
	
	
	for(int i = 0; i < nFiles; i++){
		//Sumamos 1 porque el array tiene "/0" y hay que considerarlo
		int tamNombre = strlen(fileNames[i]+1); 
		header[i].name = (char*) malloc(tamNombre); //Se entiende que siempre tendrá nombre

		strcpy(header[i].name, fileNames[i]);
		
		//añadimos un elemento que va a ocultar el tamaño del nombre + el size del header
		//Que será un int, pero así sirve para otros tipos de datos.
		headerSize += tamNombre + sizeof(header->size);
		   
		
	}
	
	//Metemos los datos primero, y luego la cabezera.
	//SEEK_SET -> offset desde el inicio de los datos
	fseek(tar, headerSize, SEEK_SET);

	//For para copiar todos los datos y pegarlos en el archivo destino
	//Necesitaremos también un mecanismo de control para ver si los datos no son null
	for(int i = 0; i<nFiles; i++){
		
		//Abrimos cada uno de los archivos que tenemos en fileNames y los vamos guardando
		//en el input
		//Solo lo hacemos de lectura
		if((input = fopen(fileNames[i], "r")) == NULL){
			
			fclose(tar);
			remove(tarName);
			
			//Eliminamos la memoria usada anteriormente si se ha usado
			for (int j = 0; j < nFiles; j++)
				free(header[j].name);

			free(header);
			return (EXIT_FAILURE);		
		}
		
		//Queremos saber cuanto ocupan los datos de nuestro archivo
		//Luego meteremos el archivo en el tar
		header[i].size = copynFile(input, tar, INT_MAX);
		fclose(input);
	}

	//Volvemos al inicio del archivo
	rewind(tar);
	
	//Escribimos la cabecera del archivo en el tar
	//Necesitamos la direccion de memoria de nFiles para escribirlo una sola vez con
	//el tamaño de un int lo que contenga nFiles en tar
	fwrite(&nFiles, sizeof(int), 1, tar);

	//Vamos a ir añadiendo elementos en el array
	for (int i = 0; i < nFiles; i++){
	
		fwrite(header[i].name, 1, strlen(header[i].name + 1, tar);
		fwrite(&header[i].size, sizeof(header[i].size), 1, tar);
	}

	//Debemos liberar la memoria que ya no vamos a utilizar
	for (int i = 0; i < nFiles; i++)
		free(header[i].name;
	free(header);
	fclose(tar);

	return (EXIT_SUCCESS);
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	StHeaderEntry * header = NULL;
	int numArchivos;		//Para los archivos
	FILE * tar = NULL;			//Tarfile

	//Intentamos abrir el tarfile
	//Tar tendrá la direccion inicial asignada por fopen
	//fopen abre el archivo y si no existe lo crea
	if((tar = fopen(tarName), "r") == NULL) {
		return EXIT_FAILURE;
	}

	//Lectura de la cabecera
	header = readHeader(tar, &numArchivos);
			
	for(int i= 0; i < numArchivos; i++){
	
		//Creamos y escribimos sobre los ficheros
		FILE * tmpFile = fopen(header[i].name, "w");
		//Caso de error
		if(tmpFile == NULL){
			for(int j = 0; j < numArchivos; j++)
				free(header[i].name);
			free(header);
			fclose(tar);
			return (EXIT_FAILURE);
		}
		//Caso de acierto
		//Con esto sabemos cuantos datos se han copiado
		int copiaFiles = copynFile(tar, tmpFile, header[i].size);

		if(copiaFiles = -1){
			for(int j = 0; j < numArchivos; j++)
				free(header[i].name);
			free(header);
			fclose(tar);
			return (EXIT_FAILURE);
		}

		fclose(tmpFile);	//Si se copia bien, lo cerramos.
		
		
	}

	//Vamos a liberar la cabecera y cerrar todo
	for(int i = 0; i < numArchivos, i++) 
		free(header[i].name);
	free(header)
	close(tar);
	
	return EXIT_SUCCESS;
	
	// Complete the function
	return EXIT_FAILURE;
}
