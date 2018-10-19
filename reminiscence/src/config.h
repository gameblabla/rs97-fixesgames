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
#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
#include "file.h"
#include "intern.h"

enum ControlType { CONTROL_A, CONTROL_B };

class Config{
    int levelAllowanceEasy;
    int levelAllowanceMedium;
    int levelAllowanceHard;
    bool actual;
    char* fileName;
    char* filePath;

    ControlType controlType;

    static const uint16 version = 1;

    void DecodeLevelAllowance(uint32 levelAllowance);
    uint32 EncodeLevelAllowance();
public:
    Config();
    void Load(const char* fileName, const char* filePath);
    void Save();
    ControlType GetControlType();
    void SetControlType(ControlType type);
    int GetLevelAllowed(DifficultySetting skill);
    void SetLevelAllowed(DifficultySetting skill,int level);
};


#endif // CONFIG_H_INCLUDED
