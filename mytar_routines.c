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
	//Segundo argumento = sizeof int porque nfiles es un int
	fread(nFiles, sizeof(int), 1, tarFile);

	/* Reservamos memoria para el array */
	header = (stHeaderEntry *) malloc(sizeof (stHeaderEntry) * (*nFiles));
	for (i = 0; i < *nFiles; i++) {
	        //.. usamos loadstr para cargar el nombre en header[i].name
		//... comprobación y tratamiento de errores
		//... leemos el tamaño del fichero y lo almacenamos en header[i].size
			
		//ORDENADOR EXPLICAMELO
		if ((header[i].name=loadstr(tarFile))==NULL) {
			for (j = 0; j < *nFiles; j++){
				free(header[j].name);
				free(header);
				fclose(tarFile);
				return NULL;
			}
		}

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
	// Complete the function
	return EXIT_FAILURE;
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
	// Complete the function
	return EXIT_FAILURE;
}
