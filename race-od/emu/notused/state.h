//---------------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version. See also the license.txt file for
//  additional informations.
//---------------------------------------------------------------------------

// state.h: state saving
//
//  01/20/2009 Cleaned up interface, added loading from memory (A.K.)
//             Moved struct definitions to .cpp
//  09/11/2008 Initial version (Akop Karapetyan)
//
//////////////////////////////////////////////////////////////////////

#ifndef _STATE_H
#define _STATE_H

int state_get_size();
int state_store_mem(void *state);
int state_restore_mem(void *state);

int state_store(char* filename);
int state_restore(char* filename);
int state_store(FILE *stream);
int state_restore(FILE *stream);

//=============================================================================
#endif // _STATE_H

