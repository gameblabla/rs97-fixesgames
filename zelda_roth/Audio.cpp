/*

    Zelda Return of the Hylian

    Copyright (C) 2005-2008  Vincent Jouillat

    Please send bugreports with examples or suggestions to www.zeldaroth.fr

*/

#include <sstream>
#include <fstream>
#include <iostream>

#include <SDL/SDL.h>

#include "Audio.h"

Audio::Audio() : musiqueId(0), specialId(0) {
    SOUND = true;
    music = NULL;
    

    if(SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) SOUND = false;
   
    if (SOUND) {
	Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 2048);
        previous_volume = Mix_VolumeMusic(32);
        loadSounds();
        setVolson(32);
    }

}

Audio::~Audio() {
    if (SOUND) {
        freeSounds();
        Mix_PauseMusic();
        Mix_VolumeMusic(previous_volume);
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        Mix_CloseAudio();
    }

}

void Audio::setVolume(int volume) {
	Mix_VolumeMusic(volume);
}

void Audio::setVolson(int volson) {
    for (int i = 0; i < 40; i++)
		Mix_VolumeChunk(sons[i], volson);
}

void Audio::loadSounds() {
    sons = new Mix_Chunk*[40];

    sons[0] = Mix_LoadWAV("data/sound/text.ogg"); // lettres
    sons[1] = Mix_LoadWAV("data/sound/menu1.ogg"); // menu 1
    sons[2] = Mix_LoadWAV("data/sound/menu2.ogg"); // menu 2
    sons[3] = Mix_LoadWAV("data/sound/menu3.ogg"); // menu 3
    sons[4] = Mix_LoadWAV("data/sound/menu4.ogg"); // menu 4
    sons[5] = Mix_LoadWAV("data/sound/timewarp.ogg"); // time retour
    sons[6] = Mix_LoadWAV("data/sound/tombe.ogg"); // tombe (ennemi)
    sons[7] = Mix_LoadWAV("data/sound/hitenemy.ogg"); //shot 1
    sons[8] = Mix_LoadWAV("data/sound/killenemy.ogg"); // shot 2
    sons[9] = Mix_LoadWAV("data/sound/surprise.ogg"); // surprise
    sons[10] = Mix_LoadWAV("data/sound/monte.ogg"); // monte
    sons[11] = Mix_LoadWAV("data/sound/descend.ogg"); // descend
    sons[12] = Mix_LoadWAV("data/sound/chute.ogg"); // chute
    sons[13] = Mix_LoadWAV("data/sound/item.ogg"); // item
    sons[14] = Mix_LoadWAV("data/sound/rupee.ogg"); // rubis
    sons[15] = Mix_LoadWAV("data/sound/heart.ogg"); // coeur
    sons[16] = Mix_LoadWAV("data/sound/bomb.ogg"); // bombe
    sons[17] = Mix_LoadWAV("data/sound/textnext.ogg"); // suite texte
    sons[18] = Mix_LoadWAV("data/sound/textend.ogg"); // fin texte
    sons[19] = Mix_LoadWAV("data/sound/happy.ogg"); // trouve objet
    sons[20] = Mix_LoadWAV("data/sound/door.ogg"); // ouvre porte
    sons[21] = Mix_LoadWAV("data/sound/pics.ogg"); // pics contre mur
    sons[22] = Mix_LoadWAV("data/sound/sword.ogg"); // Epée
    sons[23] = Mix_LoadWAV("data/sound/SwordCharging.ogg"); // chargée
    sons[24] = Mix_LoadWAV("data/sound/Sword360.ogg"); // spin
    sons[25] = Mix_LoadWAV("data/sound/shoot.ogg"); // flèche
    sons[26] = Mix_LoadWAV("data/sound/hookshot.ogg"); // grappin
    sons[27] = Mix_LoadWAV("data/sound/stamp.ogg"); // pose bombe
    sons[28] = Mix_LoadWAV("data/sound/magic.ogg"); // magie
    sons[29] = Mix_LoadWAV("data/sound/burn.ogg"); // brËšle
    sons[30] = Mix_LoadWAV("data/sound/hammer.ogg"); // marteau
    sons[31] = Mix_LoadWAV("data/sound/plouf.ogg"); // plouf
    sons[32] = Mix_LoadWAV("data/sound/danger.ogg"); // danger
    sons[33] = Mix_LoadWAV("data/sound/hurt.ogg"); // link se blesse
    sons[34] = Mix_LoadWAV("data/sound/porte.ogg"); // porte objet
    sons[35] = Mix_LoadWAV("data/sound/lance.ogg"); // lance objet
    sons[36] = Mix_LoadWAV("data/sound/casse.ogg"); // casse objet
    sons[37] = Mix_LoadWAV("data/sound/charge.ogg"); // charge magie
    sons[38] = Mix_LoadWAV("data/sound/buisson.ogg"); // coupe buisson
    sons[39] = Mix_LoadWAV("data/sound/pousse.ogg"); // pousse caisse
}

void Audio::freeSounds() {
    if (SOUND) {
        for (int i = 0; i < 40; i++) Mix_FreeChunk(sons[i]);
        delete[] sons;
    }
}

void Audio::playSound(int id, int chl) {
    if (SOUND) Mix_PlayChannel(chl,sons[id],0);
}

void Audio::playMusic(int id) {
    if (SOUND) {
        if (musiqueId != id) {
	    musiqueId = id;            
            if (specialId == 0) {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = choixMusique(id);
		Mix_PlayMusic(music,-1);
		specialId = 0;
	    }
        }
    }
}


void Audio::stopMusic() {
    if (SOUND) Mix_HaltMusic();
}

void Audio::replayMusic() {
    if (SOUND) Mix_PlayMusic(music,-1);
}

Mix_Music* Audio::choixMusique(int id) {
    switch (id) {
        case 1 : return Mix_LoadMUS("data/music/Foret.mid");
        case 2 : return Mix_LoadMUS("data/music/Plaine.mid");
        case 3 : return Mix_LoadMUS("data/music/Lac.mid");
        case 4 : return Mix_LoadMUS("data/music/Mont.mid");
        case 5 : return Mix_LoadMUS("data/music/Desert.mid");
        case 6 : return Mix_LoadMUS("data/music/Ombre.mid");
        case 7 : return Mix_LoadMUS("data/music/Feu.mid");
        case 8 : return Mix_LoadMUS("data/music/Cocorico.mid");
        case 9 : return Mix_LoadMUS("data/music/Chateau.mid");
        case 10 : case 11 : return Mix_LoadMUS("data/music/Secret.mid");
        case 12 : case 13 : case 14 : return Mix_LoadMUS("data/music/Donjon1.mid");
        case 15 : return Mix_LoadMUS("data/music/DDesert.mid");
        case 16 : case 17 : case 18 : return Mix_LoadMUS("data/music/Donjon2.mid");
        case 19 : return Mix_LoadMUS("data/music/DSecret.mid");
        case 20 : return Mix_LoadMUS("data/music/DFinal.mid");
        case 21 : return Mix_LoadMUS("data/music/Home.mid");
        case 22 : case 23 : return Mix_LoadMUS("data/music/Cave.mid");
        case 24 : case 25 : case 26 : case 27 : return Mix_LoadMUS("data/music/Fee.mid"); break;
//case 28 : case 32 : case 35 : case 36 : case 38 : case 41 : case 42 : case 43 : case 44 :
//    return Mix_LoadMUS("data/music/Maison.mid"); break;
        case 29 : case 37 : case 40 : return Mix_LoadMUS("data/music/Potion.mid"); break;
        case 30 : return Mix_LoadMUS("data/music/Bar.mid"); break;
        case 31 : return Mix_LoadMUS("data/music/Magasin.mid"); break;
        case 33 : case 34 : case 39 : return Mix_LoadMUS("data/music/Jeu.mid"); break;
        case 45 : return Mix_LoadMUS("data/music/Titre.mid"); break;
        case 46 : return Mix_LoadMUS("data/music/Selection.mid"); break;
        case 47 : return Mix_LoadMUS("data/music/Debut.mid"); break;
        default : return Mix_LoadMUS("data/music/Maison.mid");
    }
}

void Audio::playSpecial(int id) {
    if (SOUND) {
        if (specialId != id) {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
            music = choixSpecial(id);
            Mix_PlayMusic(music,-1);
            specialId=id;
        }    
    }
}

void Audio::stopSpecial() {
    if (!specialId) return;
    int tmp = musiqueId;
    musiqueId = 0;
    specialId = 0;
    playMusic(tmp);
}

Mix_Music* Audio::choixSpecial(int id) {
    switch (id) {
        case 1 : return Mix_LoadMUS("data/music/Boss.mid");
        case 2 : return Mix_LoadMUS("data/music/Mort.mid");
        case 3 : return Mix_LoadMUS("data/music/Epee.mid");
        case 4 : return Mix_LoadMUS("data/music/BossF.mid");
        case 5 : return Mix_LoadMUS("data/music/Fin.mid");
        default : return Mix_LoadMUS("data/music/Boss.mid");
    }
}

