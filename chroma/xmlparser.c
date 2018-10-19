/*  
    xmlparser.c

    Copyright (C) 2010 Amf

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>

#include "chroma.h"
#include "xmlparser.h"
#include "util.h"

// #define PARSER_DEBUG

/* A simple state machine based XML parser - not foolproof, but enough. */

struct parser* parser_new(char * filename)
{
    struct parser* pparser;

    pparser = malloc(sizeof(struct parser));
    if(pparser == NULL)
	fatal(gettext("Out of memory in parser_new()"));

    pparser->file = fopen(filename, "r");
    if(pparser->file == NULL)
    {
	free(pparser);
	return NULL;
    }

    pparser->state = 0;
    pparser->depth = 0;
    pparser->line = 1;
    pparser->result = 0;
    pparser->size = 256;

    pparser->buffer = malloc(pparser->size);
    if(pparser->buffer == NULL)
	fatal(gettext("Out of memory in parser_new()"));

    return pparser;
}

void parser_delete(struct parser* pparser)
{
    int i;

    if(pparser == NULL)
        return;

    for(i = 0; i < pparser->depth; i ++)
    {
	free(pparser->stack[i]);
    }
    fclose(pparser->file);
    free(pparser->buffer);
    free(pparser);
}

void parser_push(struct parser* pparser, char *buffer)
{
    int i;
    int j;
    int k;

    pparser->stack[pparser->depth] = malloc(strlen(buffer) + 1);
    if(pparser->stack[pparser->depth] == NULL)
        fatal(gettext("Out of memory in parser_push()"));
    if(pparser->depth >= PARSER_MAXDEPTH)
	fatal(gettext("Stack overflow in parser_push()"));

    /* Entity translation */
    j = 0;
    for(i = 0; i < strlen(buffer); i ++)
    {
	if(buffer[i] == '&')
	{
	    k = 0;
	    while(buffer[i + k] != 0 && buffer[i + k] != ';')
		k ++;

	    /* Other than these five, all other entities are stripped */
	    if(k == 5 && strncmp(buffer + i + 1, "quot", 4) == 0)
		pparser->stack[pparser->depth][j++] = '"';
	    if(k == 5 && strncmp(buffer + i + 1, "apos", 4) == 0)
		pparser->stack[pparser->depth][j++] = '\'';
	    if(k == 4 && strncmp(buffer + i + 1, "amp", 3) == 0)
		pparser->stack[pparser->depth][j++] = '&';
	    if(k == 3 && strncmp(buffer + i + 1, "gt", 2) == 0)
		pparser->stack[pparser->depth][j++] = '>';
	    if(k == 3 && strncmp(buffer + i + 1, "lt", 2) == 0)
		pparser->stack[pparser->depth][j++] = '<';

	    i += k;
	}
	else
	{
	    pparser->stack[pparser->depth][j++] = buffer[i];
	}
	    
    }
    pparser->stack[pparser->depth][j++] = 0;

    pparser->depth ++;
}

void parser_pop(struct parser* pparser)
{
    if(pparser->depth > 0)
    {
        free(pparser->stack[pparser->depth - 1]);
        pparser->depth --;
    }
}

int parser_parse(struct parser* pparser)
{
    int result;
    int state;
    int i, j;
    char c;
    char *tmp;

    if(pparser == NULL)
        return PARSER_END;
 
    switch(pparser->result)
    {
        case PARSER_ATTRIBUTE:
 	   parser_pop(pparser);
 	   parser_pop(pparser);
 	   break;
 
        case PARSER_CONTENT:
        case PARSER_ELEMENT_END:
        case PARSER_ERROR:
 	   parser_pop(pparser);
 	   break;
 
        default:
 	   break;
    }

    result = PARSER_CONTINUE;
    state = pparser->state;
    i = 0;
 
    /* XML parsing state machine */
    while(result == PARSER_CONTINUE)
    {
        c = fgetc(pparser->file);
        if(c == '\r')
 	   continue;
        if(c == '\n')
 	   pparser->line ++;
        if(c == '\t')
 	   c = ' ';
        if(feof(pparser->file))
 	   state = -1;
 
        switch(state)
        {
            /* State -1 : end of file detected */
 	   case -1:
 	       result = PARSER_END;
 	       break;
 
            /* State 0 : outside of <> */
 	   case 0:
 	       if(c == '<')
 	           state = 1;
 	       if(c == '\n')
 	           continue;
 	       break;
 
            /* State 1 : reading element name */
 	   case 1:
 	       if(c == '>')
 	           state = 0;
 	       if(c == ' ' || c == '\n')
 	           state = 2;
	       if(c == '<')
		   state = 2; /* ERROR, unless <!--<; not caught */
 	       break;
 
            /* State 2 : reading attribute name */
 	   case 2:
 	       if(c == '>')
 	           state = 0;
 	       if(c == '=')
 	           state = 3;
 	       if(c == ' ' || c == '\n')
 	           continue;
 	       break;
 
            /* State 3 : searching for start of attribute value */
 	   case 3:
 	       if(c == '>')
 	           state = 0; /* ERROR */
 	       if(c == '"')
 	           state = 4;
 	       break;
 
            /* State 4 : reading attribute value */
 	   case 4:
 	       if(c == '>')
 	           state = 0; /* ERROR */
 	       if(c == '"')
 	           state = 2;
 	       if(c == '\n')
 	           continue;
 	       break;
 
            /* States 5, 6, 7 : searching for end of comment */
 	   case 5:
 	       if(c == '-')
 	           state = 6;
 	       break;
 	   case 6:
 	       if(c == '-')
 	           state = 7;
 	       else
 	           state = 5;
 	       break;
 	   case 7:
 	       if(c == '>')
 	           state = 0;
 	       break;
 
 	   default:
 	       break;
        }
 
        /* Have we changed state? */
        if(state != pparser->state)
        {
 	   pparser->buffer[i++] = 0;
 
 	   switch(pparser->state)
 	   {
 	       case 0:
 		   /* We've found content */
 	           i = 0;
 	           while(pparser->buffer[i] == ' ')
 		       i ++;
 
 		   /* If there's more than just whitespace, return it */
 		   if(pparser->buffer[i] != 0)
 		   {
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_CONTENT;
 		   }
 		   break;
 
 	       case 1:
 		   /* We've found </element> */
 	           if(pparser->buffer[0] == '/')
 	           {
 		       if(pparser->depth > 0 &&
 			       strcmp(pparser->stack[pparser->depth - 1], pparser->buffer + 1) == 0)
 		       {
 			   result = PARSER_ELEMENT_END;
 		       }
 		       else
 		       {
 			   /* Error */
 			   parser_push(pparser, pparser->buffer);
 			   sprintf(pparser->buffer, gettext("Badly nested tags on line %d (expecting '/%s', found '%s')"), pparser->line, pparser->stack[pparser->depth - 2], pparser->stack[pparser->depth - 1]);
 			   parser_pop(pparser);
 
 			   parser_push(pparser, pparser->buffer);
 			   result = PARSER_ERROR;
 		       }
 	           }
 		   /* We've found the start of a comment */
 	           else if(strcmp(pparser->buffer, "!--") == 0)
 		   {
 		       state = 5;
 		   }
 		   /* We've found <element */
 		   else if(strlen(pparser->buffer) > 0 && pparser->buffer[strlen(pparser->buffer) - 1] != '/')
 		   {
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_ELEMENT_START;
 	           }
 		   /* We've found <element/> */
 		   else if(strlen(pparser->buffer) > 0)
 		   {
 		       /* Lose / suffix */
 		       pparser->buffer[strlen(pparser->buffer) - 1] = 0;
 
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_ELEMENT_END;
 		   }
 		   break;
 
 	       case 2:
 		   /* We've found /> */
 	           if(pparser->buffer[0] == '/')
 	           { 
		       result = PARSER_ELEMENT_END;
 	           }
 		   /* We've found attribute= */
 	           else if(state == 3)
 	           {
 		       /* Push attribute name */
 		       parser_push(pparser, pparser->buffer);
 	           }
 		   break;
 
 	       case 3:
 		   /* We've found attribute=>, which is an error */
 		   if(state != 4)
 		   {
 		       sprintf(pparser->buffer, gettext("Malformed attribute on line %d (in '%s')"), pparser->line, pparser->stack[pparser->depth -1]);
 
 		       /* Pop unused attribute name */
 		       parser_pop(pparser);
 
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_ERROR;
 		   }
 		   break;
 
 	       case 4:
 		   /* We've found attribute=">, which is an error */
 		   if(state == 0)
 		   {
 		       sprintf(pparser->buffer, gettext("Malformed attribute on line %d (in '%s')"), pparser->line, pparser->stack[pparser->depth -1]);
 
 		       /* Pop unused attribute name */
 		       parser_pop(pparser);
 
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_ERROR;
 		   }
 		   /* We've found attribute="value" */
 		   else
 		   {
 		       parser_push(pparser, pparser->buffer);
 		       result = PARSER_ATTRIBUTE;
 		   }
 		   break;
 
 	       default:
 		   break;
 	   }
 
 	   pparser->state = state;
 	   i = 0;
        }
        else
        {
	    if(i > pparser->size - 2)
	    {
		pparser->size *= 2;
		tmp = malloc(pparser->size);
		if(tmp == NULL)
		    fatal(gettext("Out of memory in parser_parse()"));
		for(j = 0; j < i; j ++)
		    tmp[j] = pparser->buffer[j];

		free(pparser->buffer);
		pparser->buffer=tmp;

	    }
	    else
 	        pparser->buffer[i++] = c;
        }
 
 
    }
    
    pparser->result = result;

#ifdef PARSER_DEBUG
    printf("[%d]:",result);
    for(i = 0; i < pparser->depth; i ++)
    {
	printf("'%s'%s", pparser->stack[i], i == pparser->depth -1 ? "" : ", ");
    }
    printf("\n");
#endif

    return result;
}

