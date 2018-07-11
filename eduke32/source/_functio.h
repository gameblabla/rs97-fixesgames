//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//-------------------------------------------------------------------------

// _functio.h

// file created by makehead.exe
// these headers contain default key assignments, as well as
// default button assignments and game function names
// axis defaults are also included


#ifndef _function_private_
#define _function_private_
#ifdef __cplusplus
extern "C" {
#endif
char gamefunctions[NUMGAMEFUNCTIONS][MAXGAMEFUNCLEN] =
   {
   "Move_Forward",
   "Move_Backward",
   "Turn_Left",
   "Turn_Right",
   "Strafe",
   "Fire",
   "Open",
   "Run",
   "AutoRun",
   "Jump",
   "Crouch",
   "Look_Up",
   "Look_Down",
   "Look_Left",
   "Look_Right",
   "Strafe_Left",
   "Strafe_Right",
   "Aim_Up",
   "Aim_Down",
   "Weapon_1",
   "Weapon_2",
   "Weapon_3",
   "Weapon_4",
   "Weapon_5",
   "Weapon_6",
   "Weapon_7",
   "Weapon_8",
   "Weapon_9",
   "Weapon_10",
   "Inventory",
   "Inventory_Left",
   "Inventory_Right",
   "Holo_Duke",
   "Jetpack",
   "NightVision",
   "MedKit",
   "TurnAround",
   "SendMessage",
   "Map",
   "Shrink_Screen",
   "Enlarge_Screen",
   "Center_View",
   "Holster_Weapon",
   "Show_Opponents_Weapon",
   "Map_Follow_Mode",
   "See_Coop_View",
   "Mouse_Aiming",
   "Toggle_Crosshair",
   "Steroids",
   "Quick_Kick",
   "Next_Weapon",
   "Previous_Weapon",
   "Show_Console",
   "Show_DukeMatch_Scores",
   "Dpad_Select",
   "Dpad_Aiming",
   "Inventory_Macro"
   };

#ifdef __SETUP__

#define NUMKEYENTRIES 57

// GCW0 style (front buttons movement)
char keydefaults[NUMGAMEFUNCTIONS*3][MAXGAMEFUNCLEN] =
   {
   "Move_Forward", "Space", "",
   "Move_Backward", "LAlt", "",
   "Turn_Left", "", "",
   "Turn_Right", "", "",
   "Strafe", "", "",
   "Fire", "BakSpc", "",
   "Open", "Down", "",
   "Run", "", "",
   "AutoRun", "", "",
   "Jump", "Tab", "",
   "Crouch", "Up", "",
   "Look_Up", "", "",
   "Look_Down", "", "",
   "Look_Left", "", "",
   "Look_Right", "", "",
   "Strafe_Left", "LShift", "",
   "Strafe_Right", "LCtrl", "",
   "Aim_Up", "", "",
   "Aim_Down", "", "",
   "Weapon_1", "", "",
   "Weapon_2", "", "",
   "Weapon_3", "", "",
   "Weapon_4", "", "",
   "Weapon_5", "", "",
   "Weapon_6", "", "",
   "Weapon_7", "", "",
   "Weapon_8", "", "",
   "Weapon_9", "", "",
   "Weapon_10", "", "",
   "Inventory", "", "",
   "Inventory_Left", "", "",
   "Inventory_Right", "", "",
   "Holo_Duke", "", "",
   "Jetpack", "", "",
   "NightVision", "", "",
   "MedKit", "", "",
   "TurnAround", "", "",
   "SendMessage", "", "",
   "Map", "", "",
   "Shrink_Screen", "", "",
   "Enlarge_Screen", "", "",
   "Center_View", "", "",
   "Holster_Weapon", "", "",
   "Show_Opponents_Weapon", "", "",
   "Map_Follow_Mode", "", "",
   "See_Coop_View", "", "",
   "Mouse_Aiming", "", "",
   "Toggle_Crosshair", "", "",
   "Steroids", "", "",
   "Quick_Kick", "", "",
   "Next_Weapon", "Right", "",
   "Previous_Weapon", "Left", "",
   "Show_Console", "", "",
   "Show_DukeMatch_Scores", "", "",
   "Dpad_Select", "", "",
   "Dpad_Aiming", "", "",
   "Inventory_Macro", "Enter", "",
   };

// A320 style
const char oldkeydefaults[NUMGAMEFUNCTIONS*3][MAXGAMEFUNCLEN] =
   {
   "Move_Forward", "Up", "",
   "Move_Backward", "Down", "",
   "Turn_Left", "Left", "",
   "Turn_Right", "Right", "",
   "Strafe", "LAlt", "",
   "Fire", "LCtrl", "",
   "Open", "LAlt", "",
   "Run", "", "",
   "AutoRun", "", "",
   "Jump", "LShift", "",
   "Crouch", "Tab", "",
   "Look_Up", "", "",
   "Look_Down", "", "",
   "Look_Left", "", "",
   "Look_Right", "", "",
   "Strafe_Left", "", "",
   "Strafe_Right", "", "",
   "Aim_Up", "", "",
   "Aim_Down", "", "",
   "Weapon_1", "", "",
   "Weapon_2", "", "",
   "Weapon_3", "", "",
   "Weapon_4", "", "",
   "Weapon_5", "", "",
   "Weapon_6", "", "",
   "Weapon_7", "", "",
   "Weapon_8", "", "",
   "Weapon_9", "", "",
   "Weapon_10", "", "",
   "Inventory", "Enter", "",
   "Inventory_Left", "", "",
   "Inventory_Right", "", "",
   "Holo_Duke", "", "",
   "Jetpack", "", "",
   "NightVision", "", "",
   "MedKit", "", "",
   "TurnAround", "", "",
   "SendMessage", "", "",
   "Map", "", "",
   "Shrink_Screen", "", "",
   "Enlarge_Screen", "", "",
   "Center_View", "", "",
   "Holster_Weapon", "", "",
   "Show_Opponents_Weapon", "", "",
   "Map_Follow_Mode", "", "",
   "See_Coop_View", "", "",
   "Mouse_Aiming", "", "",
   "Toggle_Crosshair", "", "",
   "Steroids", "", "",
   "Quick_Kick", "", "",
   "Next_Weapon", "", "",
   "Previous_Weapon", "", "",
   "Show_Console", "", "",
   "Show_DukeMatch_Scores", "", "",
   "Dpad_Select", "BakSpc", "",
   "Dpad_Aiming", "Space", "",
   "Inventory_Macro", "", "",
   };

static char * mousedefaults[] =
   {
   "Fire",
   "MedKit",
   "Jetpack",
   "",
   "Previous_Weapon",
   "Next_Weapon",
   "",
   "",
   "",
   ""
   };


static char * mouseclickeddefaults[] =
   {
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   ""
   };


static char * joystickdefaults[] =
   {
   "Fire",
   "Strafe",
   "Run",
   "Open",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "Aim_Down",
   "Look_Right",
   "Aim_Up",
   "Look_Left",
   };


static char * joystickclickeddefaults[] =
   {
   "",
   "Inventory",
   "Jump",
   "Crouch",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   };


static char * mouseanalogdefaults[] =
   {
   "analog_turning",
   "analog_moving",
   };


static char * mousedigitaldefaults[] =
   {
   "",
   "",
   "",
   "",
   };


static char * joystickanalogdefaults[] =
   {
   "analog_turning",
   "analog_lookingupanddown",
   "analog_strafing",
   "",
   "",
   "",
   "",
   "",
   };


static char * joystickdigitaldefaults[] =
   {
   "",
   "",
   "",
   "",
   "",
   "",
   "Run",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   };
#endif
#ifdef __cplusplus
};
#endif
#endif
