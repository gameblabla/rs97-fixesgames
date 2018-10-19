/* REminiscence - Flashback interpreter
 * Copyright (C) 2005-2011 Gregory Montoir
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

#include "game.h"
#include "mod_player.h"
#include "resource.h"
#include "systemstub.h"
#include "video.h"
#include "menu.h"


#ifdef GCW0
Menu::Menu(ModPlayer *ply, Resource *res, SystemStub *stub, Video *vid,Config *config)
	: _ply(ply), _res(res), _stub(stub), _vid(vid), _config(config) {
}
#else
Menu::Menu(ModPlayer *ply, Resource *res, SystemStub *stub, Video *vid)
	: _ply(ply), _res(res), _stub(stub), _vid(vid) {
}
#endif

void Menu::drawString(const char *str, int16 y, int16 x, uint8 color) {
	debug(DBG_MENU, "Menu::drawString()");
	uint8 v1b = _vid->_charFrontColor;
	uint8 v2b = _vid->_charTransparentColor;
	uint8 v3b = _vid->_charShadowColor;
	switch (color) {
	case 0:
		_vid->_charFrontColor = _charVar1;
		_vid->_charTransparentColor = _charVar2;
		_vid->_charShadowColor = _charVar2;
		break;
	case 1:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = _charVar1;
		_vid->_charShadowColor = _charVar1;
		break;
	case 2:
		_vid->_charFrontColor = _charVar3;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 3:
		_vid->_charFrontColor = _charVar4;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 4:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar1;
		break;
	case 5:
		_vid->_charFrontColor = _charVar2;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar5;
		break;
	case 6:
		_vid->_charFrontColor = 0x08;
		_vid->_charTransparentColor = 0xFF;
		_vid->_charShadowColor = _charVar2;
		break;
	}

	drawString2(str, y, x);

	_vid->_charFrontColor = v1b;
	_vid->_charTransparentColor = v2b;
	_vid->_charShadowColor = v3b;
}

void Menu::drawString2(const char *str, int16 y, int16 x) {
	debug(DBG_MENU, "Menu::drawString2()");
	int len = 0;
	while (*str) {
		_vid->PC_drawChar((uint8)*str, y, x + len);
		++str;
		++len;
	}
	_vid->markBlockAsDirty(x * 8, y * 8, len * 8, 8);
}

void Menu::loadPicture(const char *prefix) {
	debug(DBG_MENU, "Menu::loadPicture('%s')", prefix);
	_res->load_MAP_menu(prefix, _res->_memBuf);
	for (int i = 0; i < 4; ++i) {
		for (int y = 0; y < 224; ++y) {
			for (int x = 0; x < 64; ++x) {
				_vid->_frontLayer[i + x * 4 + 256 * y] = _res->_memBuf[0x3800 * i + x + 64 * y];
			}
		}
	}
	_res->load_PAL_menu(prefix, _res->_memBuf);
	_stub->setPalette(_res->_memBuf, 256);
}

#ifdef NEW_GCW0_MAPPING
void Menu::LoadInfoScreen(ControlType controlType){
	_vid->fadeOut();
	switch (_res->_lang) {
	case LANG_FR:
        switch(controlType){
        case CONTROL_A:
            for(int i = 0; i < 57344; ++i){
                _vid->_frontLayer[i] = _instr_f_map[i];
            }
            _stub->setPalette(_instr_f_pal, 256);
            drawString(Game::_version, 1, 1, 6);
            drawString("< type A >", 26, 1, 6);
            break;
        case CONTROL_B:
            for(int i = 0; i < 57344; ++i){
                _vid->_frontLayer[i] = _instr_f_b_map[i];
            }
            _stub->setPalette(_instr_f_b_pal, 256);
            drawString(Game::_version, 1, 1, 6);
            drawString("< type B >", 26, 1, 6);
            break;
        }
        break;
	case LANG_EN:
	case LANG_DE:
	case LANG_SP:
        switch(controlType){
        case CONTROL_A:
            for(int i = 0; i < 57344; ++i){
                _vid->_frontLayer[i] = _instr_e_map[i];
            }
            _stub->setPalette(_instr_e_pal, 256);
            drawString(Game::_version, 1, 1, 6);
            drawString("< type A >", 26, 1, 6);
            break;
        case CONTROL_B:
            for(int i = 0; i < 57344; ++i){
                _vid->_frontLayer[i] = _instr_e_b_map[i];
            }
            _stub->setPalette(_instr_e_b_pal, 256);
            drawString(Game::_version, 1, 1, 6);
            drawString("< type B >", 26, 1, 6);
            break;
        }
		break;
	}
	_vid->fullRefresh();
	_vid->updateScreen();
}
#endif

void Menu::handleInfoScreen() {
#ifdef NEW_GCW0_MAPPING
	debug(DBG_MENU, "Menu::handleInfoScreen()");
	//_vid->fadeOut();

	ControlType controlType = _config->GetControlType();

	LoadInfoScreen(controlType);

	do {
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			break;
		}

		if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_LEFT;
			switch(controlType){
            case CONTROL_A:
			    controlType = CONTROL_B;
			    break;
            case CONTROL_B:
			    controlType = CONTROL_A;
			    break;
			}
			_config->SetControlType(controlType);
			LoadInfoScreen(controlType);
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
			switch(controlType){
            case CONTROL_A:
			    controlType = CONTROL_B;
			    break;
            case CONTROL_B:
			    controlType = CONTROL_A;
			    break;
			}
			_config->SetControlType(controlType);
			LoadInfoScreen(controlType);
		}
    } while (!_stub->_pi.quit);
#else
	debug(DBG_MENU, "Menu::handleInfoScreen()");
	_vid->fadeOut();
	//todo: replace me with a GCW0 specific mapping image
	switch (_res->_lang) {
	case LANG_FR:
		loadPicture("instru_f");
		break;
	case LANG_EN:
	case LANG_DE:
	case LANG_SP:
		loadPicture("instru_e");
		break;
	}
	_vid->fullRefresh();
	_vid->updateScreen();
	do {
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			break;
		}
	} while (!_stub->_pi.quit);
#endif
}

void Menu::handleSkillScreen(DifficultySetting &new_skill) {
	debug(DBG_MENU, "Menu::handleSkillScreen()");
	static const uint8 option_colors[3][3] = { { 2, 3, 3 }, { 3, 2, 3}, { 3, 3, 2 } };
	_vid->fadeOut();
	loadPicture("menu3");
	_vid->fullRefresh();
	drawString(_res->getMenuString(LocaleData::LI_12_SKILL_LEVEL), 12, 4, 3);
	DifficultySetting skill_level = new_skill;
	do {
		drawString(_res->getMenuString(LocaleData::LI_13_EASY), 15, 14, option_colors[skill_level][0]);
		drawString(_res->getMenuString(LocaleData::LI_14_NORMAL), 17, 14, option_colors[skill_level][1]);
		drawString(_res->getMenuString(LocaleData::LI_15_EXPERT), 19, 14, option_colors[skill_level][2]);

		_vid->updateScreen();
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();

		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (skill_level != SKILL_EASY) {
				skill_level = (DifficultySetting)(skill_level - 1);
			} else {
				skill_level = SKILL_HARD;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (skill_level != SKILL_HARD) {
				skill_level = (DifficultySetting)(skill_level + 1);
			} else {
				skill_level = SKILL_EASY;
			}
		}
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			new_skill = skill_level;
			return;
		}
	} while (!_stub->_pi.quit);
	new_skill = SKILL_NORMAL;
}

bool Menu::handlePasswordScreen(DifficultySetting &new_skill, uint8 &new_level) {
	debug(DBG_MENU, "Menu::handlePasswordScreen()");
	_vid->fadeOut();
	_vid->_charShadowColor = _charVar1;
	_vid->_charTransparentColor = 0xFF;
	_vid->_charFrontColor = _charVar4;
	_vid->fullRefresh();
	char password[7];
	int len = 0;
	do {
		loadPicture("menu2");
		drawString2(_res->getMenuString(LocaleData::LI_16_ENTER_PASSWORD1), 15, 3);
		drawString2(_res->getMenuString(LocaleData::LI_17_ENTER_PASSWORD2), 17, 3);

		for (int i = 0; i < len; ++i) {
			_vid->PC_drawChar((uint8)password[i], 21, i + 15);
		}
		_vid->PC_drawChar(0x20, 21, len + 15);

		_vid->markBlockAsDirty(15 * 8, 21 * 8, (len + 1) * 8, 8);
		_vid->updateScreen();
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();
		char c = _stub->_pi.lastChar;
		if (c != 0) {
			_stub->_pi.lastChar = 0;
			if (len < 6) {
				if (c >= 'a' && c <= 'z') {
					c &= ~0x20;
				}
				if ((c >= 'A' && c <= 'Z') || (c == 0x20)) {
					password[len] = c;
					++len;
				}
			}
		}
		if (_stub->_pi.backspace) {
			_stub->_pi.backspace = false;
			if (len > 0) {
				--len;
			}
		}
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			password[len] = '\0';
			for (int level = 0; level < 8; ++level) {
				for (int skill = 0; skill < 3; ++skill) {
					if (strcmp(_passwords[level][skill], password) == 0) {
						new_level = level;
						new_skill = (DifficultySetting)skill;
						return true;
					}
				}
			}
			return false;
		}
	} while (!_stub->_pi.quit);
	return false;
}

bool Menu::handleLevelScreen(DifficultySetting &new_skill, uint8 &new_level) {
	debug(DBG_MENU, "Menu::handleLevelScreen()");
	_vid->fadeOut();
	loadPicture("menu2");
	_vid->fullRefresh();
	DifficultySetting currentSkill = new_skill;
	uint8 currentLevel = new_level;
#ifdef GCW0
    uint8 maxLevelEasy = _config->GetLevelAllowed(SKILL_EASY);
    uint8 maxLevelNormal = _config->GetLevelAllowed(SKILL_NORMAL);
    uint8 maxLevelHard = _config->GetLevelAllowed(SKILL_HARD);
    uint8 maxLevel = maxLevelEasy;
    if(maxLevelNormal>maxLevel)
        maxLevel = maxLevelNormal;
    if(maxLevelHard>maxLevel)
        maxLevel = maxLevelHard;
#endif
	do {
		static const char *levelTitles[] = {
			"Titan / The Jungle",
			"Titan / New Washington",
			"Titan / Death Tower Show",
			"Earth / Surface",
			"Earth / Paradise Club",
			"Planet Morphs / Surface",
			"Planet Morphs / Inner Core"
		};
#ifdef GCW0
		for (int i = 0; i < 7; ++i) {
			drawString(levelTitles[i], 7 + i * 2, 4, (i>maxLevel)?6:((currentLevel == i) ? 2 : 3));
		}
		_vid->markBlockAsDirty(4 * 8, 7 * 8, 192, 7 * 8);

        drawString(_res->getMenuString(LocaleData::LI_13_EASY),   23,  4, (currentLevel>maxLevelEasy)?6:((currentSkill == SKILL_EASY) ? 2 : 3));
        drawString(_res->getMenuString(LocaleData::LI_14_NORMAL), 23, 14, (currentLevel>maxLevelNormal)?6:((currentSkill == SKILL_NORMAL) ? 2 : 3));
        drawString(_res->getMenuString(LocaleData::LI_15_EXPERT), 23, 24, (currentLevel>maxLevelHard)?6:((currentSkill == SKILL_HARD) ? 2 : 3));
#else
		for (int i = 0; i < 7; ++i) {
			drawString(levelTitles[i], 7 + i * 2, 4, (currentLevel == i) ? 2 : 3);
		}
		_vid->markBlockAsDirty(4 * 8, 7 * 8, 192, 7 * 8);

        drawString(_res->getMenuString(LocaleData::LI_13_EASY),   23,  4, (currentSkill == SKILL_EASY) ? 2 : 3);
        drawString(_res->getMenuString(LocaleData::LI_14_NORMAL), 23, 14, (currentSkill == SKILL_NORMAL) ? 2 : 3);
        drawString(_res->getMenuString(LocaleData::LI_15_EXPERT), 23, 24, (currentSkill == SKILL_HARD) ? 2 : 3);
#endif
		_vid->markBlockAsDirty(4 * 8, 23 * 8, 192, 8);

		_vid->updateScreen();
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();

#ifdef GCW0
        bool needSkillFixup = false;
		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (currentLevel != 0) {
				--currentLevel;
			} else {
				currentLevel = maxLevel;
				needSkillFixup = true;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (currentLevel != maxLevel) {
				++currentLevel;
				needSkillFixup = true;
			} else {
				currentLevel = 0;
			}
		}
		if(needSkillFixup){
			switch(currentSkill){
            case SKILL_EASY:
                if(currentLevel>maxLevelEasy){
                    if(currentLevel>maxLevelNormal){
                        if(currentLevel>maxLevelHard){
                            currentLevel=SKILL_EASY;
                        }else{
                            currentSkill = SKILL_HARD;
                        }
                    }else{
                        currentSkill = SKILL_NORMAL;
                    }
                }
                break;
            case SKILL_NORMAL:
                if(currentLevel>maxLevelNormal){
                    if(currentLevel>maxLevelEasy){
                        if(currentLevel>maxLevelHard){
                            currentLevel=SKILL_EASY;
                        }else{
                            currentSkill = SKILL_HARD;
                        }
                    }else{
                        currentSkill = SKILL_EASY;
                    }
                }
                break;
            case SKILL_HARD:
                if(currentLevel>maxLevelHard){
                    if(currentLevel>maxLevelEasy){
                        if(currentLevel>maxLevelNormal){
                            currentLevel=SKILL_EASY;
                        }else{
                            currentSkill = SKILL_NORMAL;
                        }
                    }else{
                        currentSkill = SKILL_EASY;
                    }
                }
                break;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_LEFT;
			switch(currentSkill){
            case SKILL_EASY:
                if(currentLevel <= maxLevelHard){
                    currentSkill = SKILL_HARD;
                }else if(currentLevel <= maxLevelNormal){
                    currentSkill = SKILL_NORMAL;
                }
			    break;
            case SKILL_NORMAL:
                if(currentLevel <= maxLevelEasy){
                    currentSkill = SKILL_EASY;
                }else if(currentLevel <= maxLevelHard){
                    currentSkill = SKILL_HARD;
                }
			    break;
            case SKILL_HARD:
                if(currentLevel <= maxLevelNormal){
                    currentSkill = SKILL_NORMAL;
                }else if(currentLevel <= maxLevelEasy){
                    currentSkill = SKILL_EASY;
                }
			    break;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
			switch(currentSkill){
			case SKILL_EASY:
    			if(currentLevel <= maxLevelNormal){
        			currentSkill = SKILL_NORMAL;
                }else if(currentLevel <= maxLevelHard){
                    currentSkill = SKILL_HARD;
                }
			    break;
            case SKILL_NORMAL:
                if(currentLevel <= maxLevelHard){
                    currentSkill = SKILL_HARD;
                }else if(currentLevel <= maxLevelEasy){
                    currentSkill = SKILL_EASY;
                }
			    break;
            case SKILL_HARD:
                if(currentLevel <= maxLevelEasy){
                    currentSkill = SKILL_EASY;
                }else if(currentLevel <= maxLevelNormal){
                    currentSkill = SKILL_NORMAL;
                }
			    break;
			}
		}
#else
		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (currentLevel != 0) {
				--currentLevel;
			} else {
				currentLevel = 6;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (currentLevel != 6) {
				++currentLevel;
			} else {
				currentLevel = 0;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_LEFT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_LEFT;
			if (currentSkill != SKILL_EASY) {
				currentSkill = (DifficultySetting)(currentSkill-1);
			} else {
				currentSkill = SKILL_HARD;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_RIGHT) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
			if (currentSkill != SKILL_HARD) {
				currentSkill = (DifficultySetting)(currentSkill+1);
			} else {
				currentSkill = SKILL_EASY;
			}
		}
#endif
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			new_skill = currentSkill;
			new_level = currentLevel;
			return true;
		}

		if (_stub->_pi.backspace) {
			_stub->_pi.backspace = false;
			return false;
		}
	} while (!_stub->_pi.quit);
	return false;
}

bool Menu::handleTitleScreen(DifficultySetting &new_skill, uint8 &new_level) {
	debug(DBG_MENU, "Menu::handleTitleScreen()");
	bool quit_loop = false;
	int menu_entry = 0;
	bool reinit_screen = true;
	bool continue_game = true;
	_charVar1 = 0;
	_charVar2 = 0;
	_charVar3 = 0;
	_charVar4 = 0;
	_charVar5 = 0;
	_ply->play(1);
	static const struct {
		int str;
		int opt;
	} menu_items[] = {
		{ LocaleData::LI_07_START, MENU_OPTION_ITEM_START },
#ifdef ENABLE_PASSWORD_MENU
		{ LocaleData::LI_08_SKILL, MENU_OPTION_ITEM_SKILL },
		{ LocaleData::LI_09_PASSWORD, MENU_OPTION_ITEM_PASSWORD },
#else
		{ LocaleData::LI_06_LEVEL, MENU_OPTION_ITEM_LEVEL },
#endif
		{ LocaleData::LI_10_INFO, MENU_OPTION_ITEM_INFO },
		{ LocaleData::LI_11_QUIT, MENU_OPTION_ITEM_QUIT }
	};
	static const int menu_items_count = ARRAYSIZE(menu_items);
	while (!quit_loop) {
		if (reinit_screen) {
			_vid->fadeOut();
			loadPicture("menu1");
			_vid->fullRefresh();
			_charVar3 = 1;
			_charVar4 = 2;
			menu_entry = 0;
			reinit_screen = false;
		}
		int selected_menu_entry = -1;
		const int y_start = 26 - menu_items_count * 2;
		for (int i = 0; i < menu_items_count; ++i) {
			drawString(_res->getMenuString(menu_items[i].str), y_start + i * 2, 20, (i == menu_entry) ? 2 : 3);
		}

		_vid->updateScreen();
		_stub->sleep(EVENTS_DELAY);
		_stub->processEvents();

		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_UP;
			if (menu_entry != 0) {
				--menu_entry;
			} else {
				menu_entry = menu_items_count - 1;
			}
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			if (menu_entry != menu_items_count - 1) {
				++menu_entry;
			} else {
				menu_entry = 0;
			}
		}
		if (_stub->_pi.enter) {
			_stub->_pi.enter = false;
			selected_menu_entry = menu_entry;
		}

		if (selected_menu_entry != -1) {
			switch (menu_items[selected_menu_entry].opt) {
			case MENU_OPTION_ITEM_START:
				new_level = 0;
				quit_loop = true;
				break;
			case MENU_OPTION_ITEM_SKILL:
				handleSkillScreen(new_skill);
				reinit_screen = true;
				break;
			case MENU_OPTION_ITEM_PASSWORD:
				if (handlePasswordScreen(new_skill, new_level)) {
					quit_loop = true;
				} else {
					reinit_screen = true;
				}
				break;
			case MENU_OPTION_ITEM_LEVEL:
				if (handleLevelScreen(new_skill, new_level)) {
					quit_loop = true;
				} else {
					reinit_screen = true;
				}
				break;
			case MENU_OPTION_ITEM_INFO:
				handleInfoScreen();
				reinit_screen = true;
				break;
			case MENU_OPTION_ITEM_QUIT:
				continue_game = false;
				quit_loop = true;
				break;
			}
		}
		if (_stub->_pi.quit) {
			continue_game = false;
			quit_loop = true;
			break;
		}
	}
	_ply->stop();
	return continue_game;
}
