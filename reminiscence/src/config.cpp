/* REminiscence - Flashback interpreter, GCW0 port
 * Copyright (C) 2013 Pierre Jost
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"

Config::Config()
{
    this->levelAllowanceEasy = 0;
    this->levelAllowanceMedium = 0;
    this->levelAllowanceHard = 0;
    this->fileName = NULL;
    this->filePath = NULL;

    controlType = CONTROL_A;
    this->actual = false;
}

void Config::Load(const char* fileName, const char* filePath){
    this->fileName = (char*)malloc(strlen(fileName));
    strcpy(this->fileName,fileName);
    this->filePath = (char*)malloc(strlen(filePath));
    strcpy(this->filePath,filePath);

	File f;
	if (!f.open(this->fileName, "rb", this->filePath)) {
		warning("Unable to open config file '%s'", this->fileName);
	} else {
		uint32 id = f.readUint32BE();
		if (id != 'FBCO') {
			warning("Bad config format");
		} else {
			uint16 ver = f.readUint16BE();
			if(version!=ver){
                warning("Bad config version");
			}else{
			    //read control type
			    uint32 configValue = f.readUint32BE();
			    switch(configValue){
			    case 0:
                    controlType = CONTROL_A;
                    break;
			    case 1:
                    controlType = CONTROL_B;
                    break;
			    }

			    //read level allowance
			    uint32 levelAllowance = f.readUint32BE();
			    DecodeLevelAllowance(levelAllowance);
			}
		}
	}
}

void Config::Save(){
	File f;
	if (!f.open(this->fileName, "wb", this->filePath)) {
		warning("Unable to save config file '%s'", this->fileName);
	} else {
		// header
		f.writeUint32BE('FBCO');
		f.writeUint16BE(version);
		uint32 configValue = 0;
		switch(controlType){
        case CONTROL_A:
            configValue = 0;
            break;
        case CONTROL_B:
            configValue = 1;
            break;
		}
		f.writeUint32BE(configValue);
		f.writeUint32BE(EncodeLevelAllowance());
		if (f.ioErr()) {
			warning("I/O error when saving game config");
		}
		f.close();
	}
}

ControlType Config::GetControlType(){
    return controlType;
}

void Config::SetControlType(ControlType type){
    controlType = type;
}

int Config::GetLevelAllowed(DifficultySetting skill){
    switch(skill){
    case SKILL_EASY:
        return levelAllowanceEasy;
    case SKILL_NORMAL:
        return levelAllowanceMedium;
    case SKILL_HARD:
        return levelAllowanceHard;
    }

    //in case of doubts disallow the user
    return 0;
}

void Config::SetLevelAllowed(DifficultySetting skill,int level){
    switch(skill){
    case SKILL_EASY:
        levelAllowanceEasy = level;
        break;
    case SKILL_NORMAL:
        levelAllowanceMedium = level;
        break;
    case SKILL_HARD:
        levelAllowanceHard = level;
        break;
    }
}

void Config::DecodeLevelAllowance(uint32 levelAllowance){
    //AAAA AAAA xxBB BBBB BBxx xCCC CCCC CxxD
    this->levelAllowanceEasy = (levelAllowance>>24)&0x7;
    this->levelAllowanceMedium = (levelAllowance>>14)&0x7;
    this->levelAllowanceHard = (levelAllowance>>3)&0x7;
    if((levelAllowance&0xC03807)!=0){
        actual = true;
    }
}

uint32 Config::EncodeLevelAllowance(){
    return((levelAllowanceEasy<<24)+(levelAllowanceMedium<<14)+(levelAllowanceHard<<3)+(actual?1:0));
}
