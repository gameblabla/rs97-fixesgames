#include <stdio.h>
#include <assert.h>

#include "fichier.h"

//#define DEBUG_FICHIER

#ifdef DEBUG_FICHIER

int strcmp(const char *c, const char *d )
{
  int b = 0;
  while(*c != '\0' && *d != '0')
  {
     if ( (*c == '0' && *d != '0') ||
           (*d == '0' && *c != '0')) { b=1; break;}
       if (*c != *d)
       {
         b = 1;
         fprintf(stderr, "Comparaison entre: %c et %c\n", *c, *d);
       }
       c++; d++;
         
   }
  return b;    
}


#else
int strcmp(const char * cs,const char * ct)
{
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

#endif


void oki () { fprintf(stderr, " = OK\n"); }

void test_fichierlLireEgale()
{
   fprintf(stderr, "* Test test_niveauLireFichierEgale");
   Fichierl* f = fichierlOuvrir("./test/testfichieregale.txt");
   char test[52];
   int i;
   
   if (f==NULL)
    fprintf(stderr, "probleme fichier");
   fichierlLireEgale(f, test);
   i = strcmp("NIVEAU", test);
   assert(i==0);
   fichierlFermer(f);   
   oki ();  
}

void test_fichierlLireLigne ()
{
   fprintf(stderr, "* Test test_niveauLireLigne format dos");
   Fichierl* f = fichierlOuvrir("./test/testligne.txt");
   char test[52];
   
   if (f==NULL)
    fprintf(stderr, "probleme fichier");
    
   fichierlLireLigne(f, test);
   // fprintf(stderr, "\n%s\n", test);
   assert( strcmp("COUCOU", test)==0);
   fichierlLireLigne(f, test);
   // fprintf(stderr, "\n%s\n", test);
 
   assert( strcmp("SALUT", test)==0);
   assert(fichierlLireLigne(f, test) == FIN_DE_FICHIER);
   fichierlFermer(f);   
   oki ();     
     
}

void test_fichierlLireLigne_linux ()
{
   fprintf(stderr, "* Test test_niveauLireLigne format linux");
   Fichierl* f = fichierlOuvrir("./test/testligne_linux.txt");
   char test[52];
   
   if (f==NULL)
    fprintf(stderr, "probleme fichier");
    
   fichierlLireLigne(f, test);
   //fprintf(stderr, "\n%s\n", test);
   assert( strcmp("COUCOU", test)==0);
   fichierlLireLigne(f, test);
   assert( strcmp("SALUT", test)==0);
   assert(fichierlLireLigne(f, test) == FIN_DE_FICHIER);
   fichierlFermer(f);   
   oki ();     
     
}

int main ()
{
    fprintf(stderr, "===TEST MODULE FICHIER====\n");
    test_fichierlLireEgale();
    test_fichierlLireLigne ();
    test_fichierlLireLigne_linux ();
 
 fprintf(stderr, "\nMODULE OK\n");

 return 0;
 
}
