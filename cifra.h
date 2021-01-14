#ifndef CIFRA_H

#include <openssl/rsa.h>

#define CIFRA_H

const EVP_MD *EVP_get_digestbyname(const char *name);
int numCaracteresArchivo(char * nombre);
void llenarArreglo(char * nombre, char * str);
void crearHash(char * nombre);
RSA * cargarLlavePublica( char * archivo, const char * pass );
RSA * cargarLlavePrivada( char * archivo, const char * pass );
unsigned char * obtenerCadenaArchivo(char * archivo);
void guardarCadenaArchivo(unsigned char * cadCifra, char * nomArch);
int firmarArchivo(char * archivo, char * nomArchFirma, char * nomArchLlavePrivada, const char * pass);
int comprobarArchivo(char * archivo,  char * nomArchLlavePublica);
int numCaracteresArchivoHex(char * nombre);
void agregarFirmaArchivo(unsigned char * cadFirma, char *nomArch);
unsigned char * obtenerFirmaArchivo(char * nomArch);


#endif