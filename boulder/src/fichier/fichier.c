#include <stdio.h>
#include <stdlib.h>


#include "fichier.h"

/* plus de commentaire dans le .h*/

int fichierlLireLigne (Fichierl* f, char* c)
{
 char* temp = c;
 char lu;  
 if (f == NULL)
 {
   fprintf(stderr, "\n** Module Fichier: fichierlLireLigne: pointeur de fichier nul\n"); 
   return EXIT_FAILURE;   
  }
 do {
    if( fscanf(f, "%c", &lu) == EOF)
      return FIN_DE_FICHIER ;
 }
 while( lu == ' ' || lu == '\n' || lu == '\0' ||  lu == '\r');  
 
 *temp = lu; 
 if (*temp != '=')
   while(*temp != '\n' && *temp != EOF && *temp != '\0' && *temp != '\r') {
     temp++;
    if( fscanf(f, "%c", temp) == EOF)
      return FIN_DE_FICHIER;
   }

 *temp = '\0';        
 return EXIT_SUCCESS;     
}


int fichierlLireEgale (Fichierl* f, char* c)
{
 char* temp = c;
 char lu;
 if (f == NULL)
 {
   fprintf(stderr, "\n** Module Fichier: fichierlLireEgale: pointeur de fichier nul\n"); 
   return EXIT_FAILURE;   
 }
   
 do {
    if( fscanf(f, "%c", &lu) == EOF)
      return FIN_DE_FICHIER ;
 }
 while( lu == ' ' || lu == '\n' || lu == '\0' || lu == '\r'); 
   
 *temp = lu; 
 if (*temp != '=')
   while(*temp != '=' && *temp != EOF) {
     temp++;
    if( fscanf(f, "%c", temp) == EOF)
      return FIN_DE_FICHIER;
   }

 *temp = '\0';        
 return EXIT_SUCCESS;     
    
}


int fichierlLireEntier (Fichierl* f, int* i)
{
  if (fscanf(f, "%d", i) == EOF)
   return FIN_DE_FICHIER;
  else
   return EXIT_SUCCESS;   
}

int fichierlLireCaractere (Fichierl*f, char* lu)
{
  do  {
    if( fscanf(f, "%c", lu) == EOF)
      return FIN_DE_FICHIER;  
  }
  while( *lu == '\n' || *lu == '\0' ||  *lu == '\r'); 
  return EXIT_SUCCESS;
}

Fichierl* fichierlOuvrir(char* c)
{
  Fichierl* fichier;
  fichier = fopen(c, "r");
  if (fichier==NULL)
   fprintf(stderr, "**probleme d'ouverture du fichier %s\n", c);
  return fichier;           
}

int fichierlFermer(Fichierl *f)
{
    if (f!=NULL)
     return fclose(f);
    else 
     return EXIT_FAILURE;
}
