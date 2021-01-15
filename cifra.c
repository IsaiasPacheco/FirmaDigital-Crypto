#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <string.h>

#include "cifra.h"

//firmarArchivo("prueba2.txt", "firma.sha", "private.pem", "pacheco01");
//comprobarArchivo("prueba2.txt", "public.pem");


/**
 * Metodo para obtener la firma digital de un archivo
 * @param archivo Archivo del cual se quiere obtener la firma
 * @param nomArchFirma Nombre del archivo que contiene la firma
 * @param nomArchLlavePrivada Nombre de la llave privada
 * @param pass Contraseña para obtener la llave
 * */
int firmarArchivo(char * archivo, char * nomArchFirma, char * nomArchLlavePrivada, const char * pass){
    RSA *llaveprivada = cargarLlavePrivada(nomArchLlavePrivada, pass);

    //Se crea el hash del mensaje usando SHA1 y se guarda en el archivo "hash"
    crearHash(archivo);
    int tam = numCaracteresArchivo("hash");
    unsigned char * cipher = (unsigned char *)malloc(sizeof(char)*128);
    
    
    //Se cifra el hash obtenido usando la llave privada y obteniendo la firma digitals
    int ret = RSA_private_encrypt(tam, obtenerCadenaArchivo("hash"), cipher, llaveprivada,RSA_PKCS1_PADDING);
    if (ret < 0) {
        printf("Error al cifrar el archivo\n");
        return 0;
    }

    //Se agrega la firma al archivo
    agregarFirmaArchivo(cipher, archivo);
    return 1;
}

/**
 * Metodo para obtener comprobar la firma de un archivo
 * @param archivo Archivo firmado
 * @param nomArchLlavePublica Nombre de la llave publica
 * */
int comprobarArchivo(char * archivo,  char * nomArchLlavePublica){
    RSA *llavepublica = cargarLlavePublica(nomArchLlavePublica, NULL);
    int tam = numCaracteresArchivo(archivo);
    int ret,i = 0;

    //Arreglo que almacena el resultado de descifrar el digesto obtenido del archivo firmado
    unsigned char digestocad[tam];
    ret = RSA_public_decrypt(128, obtenerFirmaArchivo(archivo),digestocad, llavepublica, RSA_PKCS1_PADDING);    
    if(ret < 0){
        printf("Error al comprobar el archivo\n");
        return 0;
    }else{
        //Se crea el Hash del msj del archivo firmado usando SHA1 y se guarda en el archivo "msj"
        crearHash("msj");
        //Se abre el archivo para poder comparar los hash
        FILE * f;   
        f = fopen("hash", "r");
        
        int valido=1;
        unsigned int caracter;
        
        //Se compara el hash obtenido del msj y el obtenido de descifrar la firma
        for(i=0; i<20; i++){
            fscanf(f, "%02x", &caracter);
            //printf("[%02x]", caracter);
            if( !(caracter == digestocad[i]) ){
                valido = 0;
            }
        }

        fclose(f);
        /*printf("\n");
        for(i=0; i<20; i++){
            printf("<%02x>",digestocad[i]);
        }*/
        
        if(valido){
            printf("No hubo alteracion en el archivo\n");
            return 1;
        }
        else{
            printf("El archivo fue alterado\n");
            return 2;
        }
    }

}


/*
* Metodo para crear una llave provada desde el archivo especificado
* @param archivo El nombre del archivo que contiene la llave privada
* @return llavepublica Apuntador a la llave
*/
RSA * cargarLlavePrivada( char * archivo, const char * pass ){
    FILE * f;
    RSA *llaveprivada = NULL;
    f = fopen(archivo, "rb");

    if( f == NULL ){
        perror("Error al cargar la llave privada\n");
    }else{
        llaveprivada = PEM_read_RSAPrivateKey(f,NULL, NULL,(void *)pass);
    }

    fclose(f);
    return llaveprivada;   
}

/*
* Metodo para crear una llave publica desde el archivo especificado
* @param archivo El nombre del archivo que contiene la llave publica
* @return llavepublica Apuntador a la llave
*/
RSA * cargarLlavePublica( char * archivo, const char * pass ){
    FILE * f;
    RSA *llavepublica = NULL;
    f = fopen(archivo, "rb");

    if( f == NULL ){
        perror("Error al cargar la llave pública\n");
    }else{
        llavepublica = PEM_read_RSA_PUBKEY(f, &llavepublica,NULL,(void *)pass);
    }

    fclose(f);
    return llavepublica;
}

/**
 * Crea el archivo hash  del digesto del archivo de entrada
 * 
 * */
void crearHash(char * nombre){
    //Arreglo para almacenar la cadena del archivo de texto
    char * str = (char *) malloc(sizeof(char) * numCaracteresArchivo(nombre));
    llenarArreglo(nombre, str);

    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int i=0;

    md = EVP_get_digestbyname("sha1");

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, str, strlen(str));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);

    FILE * archivo;
    archivo = fopen("hash", "w");

    printf("Digesto:[");
    for(i = 0; i < md_len; i++){
            printf("%02x", md_value[i]);
            fprintf(archivo, "%02x", md_value[i]);
    }
    printf("]\n");

    fclose(archivo);
    EVP_cleanup();

}

//Llana el arreglo con el contenido del archivo
void llenarArreglo(char * nombre, char * str){
    FILE * archivo;
    int cont=0;
    archivo = fopen(nombre, "r");

    if(archivo==NULL){
        printf("Error al abrir el archivo");
        exit(0);
    }else{
        char cad;

        while( (cad = fgetc(archivo)) != EOF ){
            str[cont] = cad;
            cont++;
        }
    }
    fclose(archivo);
}

//Cuenta el num de caracteres de un archivo
int numCaracteresArchivo(char * nombre){
    FILE * archivo;
    int cont=0;
    archivo = fopen(nombre, "r");

    if(archivo==NULL){
        printf("Error al abrir el archivo");
        exit(0);
    }else{
        char cad;

        while( (cad = fgetc(archivo)) != EOF ){
            cont++;
        }
    }
    fclose(archivo);
    return cont;
 }

//Se encarga de leer la firma en formato hexadecimal
unsigned char * obtenerCadenaArchivo(char * archivo){
    FILE * f;
    int tam = numCaracteresArchivo(archivo);
    tam = tam/2;
    unsigned int caracter;
    int count =0;
    unsigned char * cad = (unsigned char *)malloc(sizeof(char) * tam );
    
    f = fopen(archivo, "r");
    
    while( fscanf(f,"%02x", &caracter) != EOF ){
        cad[count] = caracter;
        count++;
    }

    /*int i=0;
    for(i=0; i<tam;i++)
        printf("%02x",cad[i]);*/
    fclose(f);
    return cad;
}

//Se encarga de escribir un archivo con la cadena especificada
void guardarCadenaArchivo(unsigned char * cadCifra, char * nomArch){
     FILE * f;
     f = fopen(nomArch, "w");

     if(f == NULL){
         printf("Error con el archivo\n");
     }else{
         int tam = 128;
         int i = 0;
         for(i=0; i<tam; i++){
             fprintf(f, "%02x", cadCifra[i]);
         }
     }
     fclose(f);
}

//Se encarga de agregar la firma al final del archivo
void agregarFirmaArchivo(unsigned char * cadFirma, char *nomArch){
    FILE * f;
    f = fopen(nomArch, "a");

    //Se agrega la cadena con la firma
    int i=0;
    int tam=128;
    fprintf(f, "\n<");
    for(i=0; i<tam; i++){
        fprintf(f,"%02x", cadFirma[i]);
    }
    fprintf(f, ">");

    fclose(f);
}

//Se encarga de obtener la firma del archivo
unsigned char * obtenerFirmaArchivo(char * nomArch){
    FILE * f;
    FILE * faux;
    f = fopen(nomArch, "r");
    faux = fopen("msj", "w");
    
    int tam = 128;
    int cont=0;
    int i=0;
    
    unsigned int caracter;
    char car;
    unsigned char * firma = (unsigned char *)malloc(sizeof(char)*tam);

    //Contar elementos hasta '<'
    while( (car = fgetc(f)) != EOF ){
        cont++;
        if( car == '<'){
           i = 1;
           break;
        }
    }

    //Si se encuentra '<' significa que el archivo esta firmado
    if(i){
        
        //Encontrar '<';
        rewind(f);
        while( (car = fgetc(f)) != '<');

        //Rebobinar para quitar ultimo salto de linea
        rewind(f);
        while( i < (cont-1) ){
            car = fgetc(f);
            putc(car, faux);
            i++;
        }

        rewind(f);
        //Encontra '<'
        while( (car = fgetc(f)) != '<');

        cont = 0;
        while( cont < tam ){
            fscanf(f,"%02x", &caracter);
            firma[cont] = caracter;
            cont++;
        }
        
        /*
        for(cont=0; cont<tam; cont++){
            printf("%02x", firma[cont]);
        }*/

    }else{
        printf("El archivo no esta firmado\n");
        firma = NULL;
        exit(0);
    }

    fclose(faux);
    fclose(f);
    
    return firma;
}