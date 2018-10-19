/*  
    xmlparser.h

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

#define PARSER_MAXDEPTH 128

struct parser
{
    FILE *file;
    int state;
    int depth;
    int line;
    int result;
    char *stack[PARSER_MAXDEPTH];
    char *buffer;
    int size;
};

enum {
    PARSER_CONTINUE,
    PARSER_CONTENT,
    PARSER_ELEMENT_START,
    PARSER_ELEMENT_END,
    PARSER_ATTRIBUTE,
    PARSER_END,
    PARSER_ERROR
};

struct parser* parser_new(char * filename);
void parser_delete(struct parser* pparser);
int parser_parse(struct parser* pparser);

/* Returns the text from parser *x, y steps from top of stack */
#define parser_text(x, y) (x->depth > y) ? x->stack[x->depth - 1 - y] : ""

/* Compares the text from parser *x, y steps from top of stack, with z */
#define parser_match(x, y, z) (x->depth > y && strcmp(x->stack[x->depth - 1 - y], z) == 0)
