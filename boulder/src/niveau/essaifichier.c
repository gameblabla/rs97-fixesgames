#include <stdio.h>
#include <stdlib.h>


int main ()
{
  FILE* f = fopen("./testfichier.txt","r");
  char c = 'a'; 
  while (!feof(f))
  {
     fread(&c, sizeof(char), 1, f);
     switch (c) {
	case '\0': break;fprintf(stderr,"FINCHAINE"); break;
	case '\n': fprintf(stderr,"RETOUR"); break;
	case 'a': fprintf(stderr, "a");   break;
        default: break;
     }
  }
 printf("\n");
 fclose(f);
 return 0;


}
