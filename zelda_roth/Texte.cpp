/*

    Zelda Return of the Hylian

    Copyright (C) 2005-2008  Vincent Jouillat

    Please send bugreports with examples or suggestions to www.zeldaroth.fr

*/

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "Texte.h"
#include "Menu.h"
#include "Joueur.h"
#include "Monde.h"
#include "Projectile.h"
#include "Jeu.h"

Texte::Texte(Jeu* jeu) : gpJeu(jeu), vitesse(40), av(0), x(0), y(0), w(0), h(0),
id(0), idsuiv(0), def(false), cadre(false), texte(""), buffer("") {
    lastAnimTime = SDL_GetTicks();
    imageFont = IMG_Load("data/images/texte/font.png");
    SDL_SetColorKey(imageFont,SDL_SRCCOLORKEY,SDL_MapRGB(imageFont->format,0,0,255));
    imageCoeur = IMG_Load("data/images/menu/coeur.png");
    SDL_SetColorKey(imageCoeur,SDL_SRCCOLORKEY,SDL_MapRGB(imageCoeur->format,0,0,255));
}

Texte::~Texte() {
    SDL_FreeSurface(imageFont);
    SDL_FreeSurface(imageCoeur);
}

void Texte::chercheText() {
    ostringstream os;
    int tmp;
    switch (id) {
        case 1  : texte = "You find a shield!!!**Your defense rises by one point!"; break;
#ifdef DINGUX
        case 2  : texte = "You find a sword!!!**You can now fight monsters with the B button!!!*Hold B to charge a spin attack!!!"; break;
#else
	case 2  : texte = "You find a sword!!!**You can now fight monsters with the key Z (or W)!!!*Hold Z to charge a spin attack!!!"; break;
#endif
        case 3  : 
            tmp = 4-(gpJeu->getJoueur()->nbQuarts()%4);
            os << tmp;
            texte = "You find a piece of heart!!!";
            if (tmp<4) buffer = "Again "+os.str()+" before having a new heart!!!";
            else buffer = "Your heart count just increases!!!";
            break;
        case 11 : texte = "N: Link's Home*W: Hyrule Field*E: Forest Temple"; break;
        case 12 : texte = "Mountain Temple**Not for cardiacs, scaredy-cats, and generally faint of heart."; break;
        case 13 : texte = "W: Desert entrance*N: Mountain Temple"; break;
        case 14 : texte = "N: Mountain Temple*S: Hyrule Field"; break;
        case 15 : texte = "N: Death Mountain"; break;
        case 16 : texte = "E: Forest Entrance"; break;
        case 17 : texte = "S: Lake Hylia*E: Haunted Graveyard"; break;
        case 18 : texte = "S: To Kakariko Village"; break;
        case 19 : texte = "N: Kakariko Village"; break;
        case 20 : texte = "N: Hyrule Field"; break;
        case 21 : texte = "W: Fire Land"; break;
        case 22 : texte = "E: Shadow Village*- Not for the living -"; break;
        case 23 : texte = "Dark Temple**If you are a ghost, seeking a job, you'd better come here to hang out with us."; break;
        case 24 : texte = "N: Shadow Village*W: Lake Hylia"; break;
        case 25 : texte = "N: Haunted Graveyard**No entry"; break;
        case 26 : texte = "Shadow Village"; break;
        case 27 : texte = "I am selling a bottle for 100 rupees, are you interested?*              YES ?            no  "; 
            if ((gpJeu->getJoueur()->hasBouteille(0) && gpJeu->getZone()==8)
            || (gpJeu->getJoueur()->hasBouteille(1) && gpJeu->getZone()==5)
            || (gpJeu->getJoueur()->hasBouteille(2) && gpJeu->getZone()==6)) {id=32; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<100) idsuiv=29;
            else idsuiv=30;
            break;
        case 28 : texte = "I am selling a bottle for 100 rupees, are you interested?*              yes             NO ?"; idsuiv=31; break;
        case 29 : texte = "Sorry, you don't have enough rupees"; break;
        case 30 : texte = "Thank you, here is your bottle."; break;
        case 31 : texte = "Come again if you change your mind."; break;
        case 32 : texte = "Sorry, that was my only bottle."; break;
        case 33 : texte = "You find an empty bottle!!!**It will help you to stock potions!!!"; break;
        case 35 : texte = "The inhabitants of that village have a dialect out of the ordinary, I don't understand anything..."; break;
        case 36 : texte = "@+@+@+@+@+@+@@@+*@=++=@=+@=+@=+=@*+@+@+@+=+="; break;
        case 37 : texte = "Wangle chief's permit if you really want to pass!!!"; break;
        case 38 : texte = "The chief allows you to pass??? Grrrr... So move along!!!"; break;
        case 39 : texte = "Get out of my way!!!"; break;
        case 40 : texte = "It's not common to have visitors around here."; break;
        case 41 : texte = "Local monsters don't fear a lot of things, fortunately they only attack humans."; break;
        case 42 : texte = "Really?*You understand what I say?"; break;
        case 43 : texte = "N: Desert and Gerudo Village"; break;
        case 44 : texte = "S: Lake Hylia*W: Hyrule Castle"; break;
        case 45 : texte = "Kakariko Village"; break;
        case 46 : texte = "W: Hyrule Castle"; break;
        case 47 : texte = "What a good day!!!"; break;
        case 48 : texte = "But why did I accept to take care of that rubbish???*I loathe hens!!!"; break;
        case 49 : texte = "You can find a lot of things in the shop of this village."; break;
        case 50 : texte = "S: Kakariko Village*W: Desert*E: Death Mountain"; break;
        case 51 : texte = "Gerudo Village"; break;
        case 52 : texte = "Lost in the desert?*You are here:*                        X"; break;
        case 54 : texte = "S: To Hyrule Castle"; break;
        case 55 : texte = "Desert Temple**Come to try our bath.*(water is not for drinking)"; break;
        case 56 : texte = "Forest Temple**Save the trees, eat beaver!!!"; break;
        case 57 : texte = "Lake Temple**Compulsory bathing cap."; break;
        case 58 : texte = "Ice Temple**The staff wishes to remind you that a dungeon is not a ice rink."; break;
        case 59 : texte = "Did you see the blacksmith of that village?*He is said to be very gifted to improve adventurers' equipment, a lot of people visit him."; break;
        case 60 : texte = "The Temple is in the deepest desert, in an ancient oasis."; break;
        case 61 : texte = "S: Fire Land*E: Kakariko Village"; break;
        case 62 : texte = "Hyrule Castle"; break;
        case 63 : texte = "E: Kakariko Village"; break;
        case 64 : texte = "W: Turtle Rock*E: Lake Hylia"; break;
        case 65 : texte = "Hidden Temple**Here rests the Sword of Evils Bane, the Master Sword."; break;
        case 66 : texte = "N: To the Haunted Graveyard"; break;
#ifdef DINGUX
        case 67 : texte = "You find the Dungeon Map!!!*Press X to see the map."; break;
#else
        case 67 : texte = "You find the Dungeon Map!!!*Press P to see the map."; break;
#endif
        case 68 : texte = "You find the Compass!!!*You can locate the boss and chests on the plan."; break;
        case 69 : texte = "You find the Boss Key!!!"; break;
        case 70 : texte = "You find a small key!!!*Go near a door to open it."; break;
#ifdef DINGUX
        case 71 : texte = "You find the Gloves!!!*Use them to lift some object setting them up or pressing A."; break;
#else
        case 71 : texte = "You find the Gloves!!!*Use them to lift some object setting them up or pressing C."; break;
#endif
        case 72 : 
            texte = "You find a Magic Crystal!!!"; 
            tmp = 7-gpJeu->getJoueur()->nbCristaux();
            os << tmp;
            if (tmp==6 && !gpJeu->getJoueur()->getAvancement()) buffer = "For what may it be used...?";
            else if (tmp>2) buffer = "There are "+os.str()+" left to find!!!";
            else if (tmp==2) buffer = "Only 2 left to find!!!";
            else if (tmp==1) buffer = "You lack only one!!!";
            else if (tmp==0) buffer = "You have all the crystals, run to the castle and save Zelda!!!";
            break;
        case 80 : texte = "You find the Hookshot!!!*Use it to overcome obstacles."; break;
        case 81 : texte = "You find the Lantern!!!*Use it to shut up flames."; break;
        case 82 : texte = "You find the Flippers!!!*Set them up to go in the water."; break;
        case 83 : texte = "You find the Magic Hammer!!!*Use it to squash obstacles."; break;
        case 84 : texte = "You find the Fire Rod!!!*From now on you are able to shoot out powerful flames."; break;
        case 85 : texte = "You find the Ice Rod!!!*Use it to freeze anything from a distance."; break;
        case 86 : texte = "You find the Master Sword!!!*Even Ganon could not stand up to its power!!! (in theory)"; break;
        case 87 : texte = "Congratulation Link, you have succeeded in saving me!!!***Let's find Ganon quickly, we have to reclaim the Triforce!"; break;
        case 89 : texte = "The secret passage behind the throne room leads to Ganon. Hurry up!"; break;
        case 90 : texte = "We are very near, follow me!"; break;
        case 91 : texte = "I'm afraid you are not able to defeat Ganon with your present weapons...*Go and speak to the chief of the village Kakariko, I am sure he will find a solution."; break;
        case 92 : texte = "Ganon is just behind that door, I will cure your wounds."; break;
        case 93 : texte = "Ganon is still somewhere in the castle."; break;
        case 94 : texte = "You should wait princess Zelda!!!"; break;
        case 95 : texte = "Wouldn't you have the spooky feeling to forget someone by any chance???"; break;
        case 96 : texte = "Zelda is waiting for you!!!"; break;
        case 97 : texte = "You find the Triforce!!!"; break;
        case 98 : texte = "You find the book of Mudora!!!**From now on, you understand the ancient Hylian!!!"; break;
        case 99 : texte = "Congratulation Link, for finding me. As a reward, I give you the Din Pendent, it raises your defense by one point."; break;
        case 100 : texte = "Congratulation Link, for finding me. As a reward, I give you the Nayru Pendent, it rises your defense by two points!!!"; break;
        case 101 : texte = "..."; break;
        case 102 : texte = "You obtain a magic pendent!!!**Your defense just raised!!!"; break;
        case 103 : texte = "Congratulation Link, for finding me. As a reward, I will double the busload of your magic meter!!!"; break;
        case 104 : texte = "Your magic meter is twofold!!!"; break;
        case 105 : texte = "Come back when you have an empty bottle and I will sell you a red potion which restores energy."; break;
        case 106 : texte = "A red potion for 60 rupees, are you interested?*              YES ?            no  "; 
            if (gpJeu->getJoueur()->hasBouteille(0)!=1
            && gpJeu->getJoueur()->hasBouteille(1)!=1
            && gpJeu->getJoueur()->hasBouteille(2)!=1) {id=105; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<60) idsuiv=29;
            else idsuiv=108; break;
        case 107 : texte = "A red potion for 60 rupees, are you interested?*              yes              NO ?"; break;
        case 108 : texte = "Thank you, here is your potion.";break;
        case 109 : texte = "You get a red potion!!!*Drink it to restore your energy!!!"; break;
        case 110 : texte = "You get a green potion!!!*Drink it to restore your magic!!!"; break;
        case 111 : texte = "You get a blue potion!!!*Drink it to restore your energy and your magic!!!"; break;
        case 112 : texte = "Hello, what would you like to drink?"; break;
        case 113 : texte = "-Hiccup!- A fairy is said to give enchanted objects... -Hiccup!- ...to the adventurers who would find her... -Hiccup!!!-"; break;
        case 114 : texte = "One heart for 10 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getVie()+gpJeu->getJoueur()->getBoostVie()
            == gpJeu->getJoueur()->getVieMax()) {id=128; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<10) idsuiv=29;
            else idsuiv=129; break;
        case 115 : texte = "One heart for 10 rupees, ok?**              yes              NO ?"; break;
        case 116 : texte = "A little bit of magic for 20 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getMagie()+gpJeu->getJoueur()->getBoostMagie()
            == gpJeu->getJoueur()->getMagieMax() || !gpJeu->getJoueur()->hasObjet(O_LANTERNE)) {
                id=128; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<20) idsuiv=29;
            else idsuiv=129; break;
        case 117 : texte = "A little bit of magic for 20 rupees, ok?**              yes              NO ?"; break;
        case 118 : texte = "Some magic for 30 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getMagie()+gpJeu->getJoueur()->getBoostMagie()
            == gpJeu->getJoueur()->getMagieMax() || !gpJeu->getJoueur()->hasObjet(O_LANTERNE)) {
                id=128; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<30) idsuiv=29;
            else idsuiv=129; break;
        case 119 : texte = "Some magic for 30 rupees, ok?**              yes              NO ?"; break;
        case 120 : texte = "5 arrows for 30 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getFleche() == gpJeu->getJoueur()->getFlecheMax() 
            || !gpJeu->getJoueur()->hasObjet(O_ARC)) {id=128; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<30) idsuiv=29;
            else idsuiv=129; break;
        case 121 : texte = "5 arrows for 30 rupees, ok?**              yes              NO ?"; break;
        case 122 : texte = "One bomb for 30 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getBombe() == gpJeu->getJoueur()->getBombeMax() 
            || !gpJeu->getJoueur()->hasObjet(O_SAC_BOMBES)) {id=128; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<30) idsuiv=29;
            else idsuiv=129; break;
        case 123 : texte = "One bomb for 30 rupees, ok?**              yes              NO ?"; break;
        case 124 : texte = "One bow for 1000 rupees, ok?**              YES ?            no  "; 
            idsuiv=29; break;
        case 125 : texte = "One bow for 1000 rupees, ok?**              yes              NO ?"; break;
        case 126 : texte = "One bow for 50 rupees, ok?**              YES ?            no  "; 
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<50) idsuiv=29;
            else idsuiv=129; break;
        case 127 : texte = "One bow for 50 rupees, ok?**              yes              NO ?"; break;
        case 128 : texte = "You don't need this right now."; break;
        case 129 : texte = "Thank you."; break;
        case 130 : texte = "You get the bow!!!*Use it to reach a distant target."; break;
        case 131 : texte = "Choose what you want."; break;
        case 132 : texte = "What???*You are in mission for the chief???*Okay, so I'm going to give you a discount for the bow"; break;
        case 133 : texte = "Hello Link, I am the chief of that village, I suspected you were coming."; idsuiv=134; break;
        case 134 : texte = "Since this morning, monsters stride along Hyrule, I tried to talk about that with Zelda, but as you perhaps already saw, a powerful spell blocks the access to the castle..."; idsuiv=136; break;
        case 136 : texte = "You say Ganon is responsible for all of that? He stole the Triforce and he holds the princess Zelda captive?"; idsuiv=137; break;
        case 137 : texte = "Mmmm... The situation is more serious than I thought..."; idsuiv=138; break;
        case 138 : texte = "We have to act very quickly, you must face Ganon again!"; idsuiv=139; break;
        case 139 : texte = "How to go in the castle? I may have an idea..."; idsuiv=140; break;
        case 140 : texte = "As you know, the power of the 7 wise men was locked in 7 crystals by wizard Aghanim when he was trying to open up the path to the Dark World, where Ganon was."; idsuiv=141; break;
        case 141 : texte = "Yet, even if you raised the descendants of the 7 wise men from death after you defeated Ganon and found the Triforce, these crystals have kept their power."; idsuiv=142; break;
        case 142 : texte = "They have been hidden deep in the 7 temples, gather them all, and you'll be able to get over Ganon's spell."; idsuiv=143; break;
        case 143 : texte = "However that won't be easy, Ganon will surely send his best units to protect these crystals..."; idsuiv=144; break;
        case 144 : texte = "A last thing, you won't go far with your present equipment. Go and see the arms dealer and tell him that you're coming on my behalf. He will probably make a gesture for you."; break;
        case 145 : texte = "Go and see the arms dealer and tell him that you're coming on my behalf. He will probably make a gesture for you."; break;
        case 146 : texte = "You have to gather the 7 crystals to have a chance to save Zelda and Hyrule..."; break;
        case 147 : texte = "You found the 7 crystals? So go right now to the castle, first you have to deliver the princess, she will know better than I how to save Hyrule."; break;
        case 148 : texte = "Your weapons are null and void against Ganon? Therefore, you have to find the Master Sword. It is said to have been concealed in a hidden temple.*Besides, the way leading to the Sword of Evils Bane is said to cross over a village populated by monsters...*I wonder if such a village exists..."; break;
        case 150 : texte = "However, I'm afraid that your present equipment is inadequate to reach this temple, you may go to see the blacksmith of Gerudo village..."; break;
        case 151 : texte = "It's about time to brave Ganon and take the Triforce back!"; break;
        case 152 : texte = "Hey! You have to pay to open one of my chests!!!"; break;
        case 153 : texte = "One of these chests contains a piece of heart, do you want to try for 10 rupees?*You will be allowed to open just one chest, ok?               YES ?            no  "; 
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<10) idsuiv=29; break;
        case 154 : texte = "One of these chests contains a piece of heart, do you want to try for 10 rupees?*You will be allowed to open just one chest, ok?               yes              NO ?"; break;
        case 155 : texte = "Choose a chest."; break;
        case 156 : texte = "I don't have anymore prizes to bring into play, sorry."; break;
        case 157 : texte = "You lose! This chest is empty. Try again!"; break;
        case 158 : texte = "Hello! If you are looking for the blacksmith, he lives a little farther."; break;
        case 159 : texte = "Hello Link, I am the chief of this village.*You should go to see the blacksmith and show him what you want him to temper in your equipment.";
            buffer = "I have been informed of your quest, so I have arranged it with him.*All will be free for you."; break;
        case 160 : texte = "You don't need to drink this potion now."; break;
        case 161 : texte = "A green potion for 40 rupees, are you interested?*              YES ?            no  "; 
            if (gpJeu->getJoueur()->hasBouteille(0)!=1
            && gpJeu->getJoueur()->hasBouteille(1)!=1
            && gpJeu->getJoueur()->hasBouteille(2)!=1) {id=163; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<40) idsuiv=29;
            else idsuiv=108; break;
        case 162 : texte = "A green potion for 40 rupees, are you interested?*              yes              NO ?"; break;
        case 163 : texte = "Come back when you have an empty bottle and I will sell you a green potion which restores magic."; break;
        case 164 : texte = "Come back later, I am overbooked!"; break;
        case 165 : texte = "May I do something for you?";
            if (gpJeu->getJoueur()->getEpee()==1) {id=166; chercheText();break;}
            if (gpJeu->getJoueur()->hasObjet(O_GANTS)==1 && gpJeu->getJoueur()->getObjet()==8) {
                id=169; chercheText();break;}
            if (gpJeu->getJoueur()->hasObjet(O_ARC)==2 && gpJeu->getJoueur()->getObjet()==0 
            && gpJeu->getJoueur()->nbCristaux()==7) {
                id=172; chercheText();break;}
            break;
        case 166 : texte = "Do you want me to temper your sword?**              YES ?            no  "; break;
        case 167 : texte = "Do you want me to temper your sword?**              yes              NO ?"; break;
        case 168 : texte = "Your sword triples in power and you can now throw some magic attack, so good!!!*"; break;
        case 169 : texte = "Do you want me to upgrade your gloves?**              YES ?            no  "; break;
        case 170 : texte = "Do you want me to upgrade your gloves?**              yes              NO ?"; break;
        case 171 : texte = "The power of your gloves helps you to lift heavy rocks."; break;
        case 172 : texte = "Do you want me to upgrade your bow?**              YES ?            no  "; break;
        case 173 : texte = "Do you want me to upgrade your bow?**              yes              NO ?"; break;
        case 174 : texte = "Your bow now shoots silver arrows with a cataclysmic power!!!"; break;
        case 175 : texte = "Come back when you have an empty bottle and I will sell you a blue potion which restores energy and magic."; break;
        case 176 : texte = "A blue potion for 160 rupees, are you interested?*              YES ?            no  "; 
            if (gpJeu->getJoueur()->hasBouteille(0)!=1
            && gpJeu->getJoueur()->hasBouteille(1)!=1
            && gpJeu->getJoueur()->hasBouteille(2)!=1) {id=175; chercheText();break;}
            if (gpJeu->getJoueur()->getRubis() + gpJeu->getJoueur()->getBoostRubis()<160) idsuiv=29;
            else idsuiv=108; break;
        case 177 : texte = "A blue potion for 160 rupees, are you interested?*              yes              NO ?"; break;
        case 178 : texte = "Our village has been constituted in the aim to stop careless adventurers from reaching the temple, I don't allow just anyone to pass."; break;
        case 179 : texte = "Mmmm... You already found 4 crystals?*I have got to admit you impress me...";
            buffer = "All right, you are allowed to cross over the graveyard, at your own risk."; break;
        case 181 : texte = "Talk to the guard at the north of the village, he will let you pass."; break;
        case 182 : texte = "I see you're still alive...*Surprising."; break;
        case 183 : texte = "The dark temple is the starting point of the way reaching a legendary object. This village just exists to dissuade adventurers from approaching."; break;
        case 184 : texte = "So you found the Sword?*But don't believe it is yours for all that.";
            buffer = "The Master Sword is among the most treasured heritage of Hyrule with the Triforce, you will have to bring it back in his sanctuary when your mission is completed."; break;
        case 186 : texte = "All the inhabitants of this village are spirits who responded to princess Zelda's call."; break;
        case 187 : texte = "When you returned peace to Hyrule and reunified the Light World with the Golden Land, Zelda secretly asked volunteer spirits to create this village to block";
            buffer = "the access to a very high valued object."; break;
        
        case 189 : texte = "Welcome Link!*Let me explain to you what I am doing here."; idsuiv=190; break;
        case 190 : texte = "As you certainly have noticed, monsters appeared in Hyrule. It's the first time I can see monsters since your victory on Ganon."; idsuiv=191; break;
        case 191 : texte = "Well, I have decided to list all of them."; idsuiv=192; break;
        case 192 : texte = "Unfortunately, I'm really too fearful to meet them, so I need your help."; idsuiv=193; break;
        case 193 : texte = "You have to fight each kind of monster and come to give me your report."; idsuiv=194; break;
        case 194 : texte = "Each time you have defeated 7 new kinds of enemies, I will give you one piece of heart."; idsuiv=195; break;
#ifdef DINGUX
        case 195 : texte = "If you want to see what enemies you've already defeated, press L."; break;
#else
        case 195 : texte = "If you want to see what enemies you've already defeated, press M."; break;
#endif
        
        
        case 196 : 
            if (gpJeu->getJoueur()->getCoeur(43)) {
                if (gpJeu->getJoueur()->nbEnnemis() >= 46) {id=205; chercheText();return;}
                else {id=204; chercheText();return;}
            }
            for (int i = 38; i < 44; i++) 
                if (!gpJeu->getJoueur()->getCoeur(i) 
                && (gpJeu->getJoueur()->nbEnnemis()/7)+37>=i) {
                    id=203; chercheText();return;
                }
            tmp = 7-(gpJeu->getJoueur()->nbEnnemis()%7);
            os << tmp;
            if (tmp>1) texte = "Again "+os.str()+" different enemies before the next reward.";
            else texte = "You just lack only one enemy before the next reward!!!";
            break;
        case 203 : texte = "Take this gift for your contribution on my project:"; break;
        case 204 : texte = "I'm sorry, but I don't have a gift for you anymore..."; break;
        case 205 : texte = "I can't take it in, you succeeded in defeating all kinds of enemies!!!*Congratulation Link!!!"; break;
        
        case 206 : texte = "                    Level 1:**                  Forest Temple"; break;
        case 207 : texte = "                    Level 2:**                  Water Temple"; break;
        case 208 : texte = "                    Level 3:**                 Mountain Temple"; break;
        case 209 : texte = "                    Level 4:**                  Desert Temple"; break;
        case 210 : texte = "                    Level 5:**                   Dark Temple"; break;
        case 211 : texte = "                    Level 6:**                   Fire Temple"; break;
        case 212 : texte = "                    Level 7:**                   Ice Temple"; break;
        case 213 : texte = "                    Level ?:**                  Hidden Temple"; break;
        case 214 : texte = "                  Last Level:**                 Hyrule Castle"; break;
        
        case 215 :texte = "                 Already tired?                  ?                 CONTINUE                 ?                   Save and quit                               Quit without saving            "; break;
        case 216 :texte = "                 Already tired?                                    Continue                     ?               SAVE AND QUIT              ?                Quit without saving            "; break;
        case 217 :texte = "                 Already tired?                                    Continue                                     Save and quit                  ?            QUIT WITHOUT SAVING           ?"; break;
        
        case 223 : texte = "At the right time when Link touched the Triforce, monsters disappeared and peace recurred.**Then Princess Zelda made a great decision: she touched in turn the Triforce and made her wish.**Triforce had always been the origin of troubles in the Kingdom, sooner or later, another evil creature would find it.**Just when princess touched the relic, it disappeared from Hyrule forever.";
            buffer = "Since this day, Hyrule people have lived in peace, without fear of new disasters.**Thus the legend of Zelda, of the Triforce and of the Golden Land is achieved.**Master Sword is said to rest safely somewhere and to be the last heirloom of Link's quest..."; break;
        case 224 : texte = "Congratulation Link, for finding me. As a reward, I give you the Farore Pendent, it raises your defense by one point..."; break;
        case 225 : texte = "Do you want to save your game?**                    ? YES ?*                      no  "; break;
        case 226 : texte = "Do you want to save your game?**                      yes  *                    ? NO ?"; break;
        case 227 : texte = "Game saved."; break;
        
        case 228 : texte = "The Kingdom of Hyrule has been in peace since Link, the last knight of Hyrule, had defeated the malicious Ganon and reclaimed the precious Triforce to him."; idsuiv = 229; break;
        case 229 : texte = "Nobody knows what Link's wish to the Triforce was, but it had the effect of reunifying the Light and Dark World, and upraising the 7 wise men's descendants."; idsuiv = 230; break;
        case 230 : texte = "Next, Link handed Triforce and Master Sword over to Princess Zelda, and people started to believe that peace would last.*But the people were wrong..."; idsuiv=231; break;
        case 231 : texte = "Unfortunately, Link's wish also had negative effects. Ganon and his henchmen were resurrected and got ready to attack."; idsuiv=232; break;
        case 232 : texte = "Somewhere in Hyrule Forest, Link is sleeping without suspecting that Ganon has already moved into the attack, until a"; 
            buffer="well-known voice talk to him during his sleep..."; break;
        case 233 : texte = "Help me!*Help me!*That's me! Zelda!*I'm talking to you by telepathy."; idsuiv = 234; break;
        case 234 : texte = "I am a prisoner in the dungeon of the castle!*I need your help!*Ganon is back, and he surely has already found the Triforce..."; idsuiv=235; break;
        case 235 : texte = "Come quickly to the castle Link, you are my only hope..."; break;
#ifndef DINGUX
        case 236 : texte = "HELP: Press F1 to consult help."; break;
#endif // no Help for Dingux
    }
    
    
}

void Texte::affiche(SDL_Surface* gpScreen, std::string s, int a, int b) {
    for (int i = 0; i < (int)s.length(); i++) {
        afficheLettre(gpScreen, s.at(i),a,b);
        a+=6;
    }
}

void Texte::draw(SDL_Surface* gpScreen) {
    
    if (cadre) drawCadre(gpScreen);
    
    if (id==3 && texte == "You find a piece of heart!!!") {
        SDL_Rect src; SDL_Rect dst;
        src.x=16*(gpJeu->getJoueur()->nbQuarts()%4);
        if(src.x==0)src.x=16*4;
        src.y=0; src.w=16; src.h=16; dst.x=160-8; dst.y=120-8+16*5;
        SDL_BlitSurface(imageCoeur, &src, gpScreen, &dst);
    }
    
    int a = x+8; int b = y+8;
    for (int i = 0; i < av; i++) {
        afficheLettre(gpScreen, texte.at(i),a,b);
        a+=6;
        if (a > x+w-16) {a=x+8; b+=16;}
    }
    
    if(SDL_GetTicks() > lastAnimTime + vitesse && def && av < (int)texte.length()) {
        lastAnimTime = SDL_GetTicks();
        do av++;
        while (av < (int)texte.length() && texte.at(av-1) == ' ');
        if (texte.at(av-1) != ' ') gpJeu->getAudio()->playSound(0,1);
    }
}

bool Texte::isFinished() {return (av==(int)texte.length());}

int Texte::getId() {return id;}

void Texte::changeId(int i) {
    id=i; idsuiv=0; buffer="";
    chercheText();
    decoupeText();
    if (av>(int)texte.length()) av=(int)texte.length();
}

void Texte::drawCadre(SDL_Surface* gpScreen) {
    SDL_Rect src;
    SDL_Rect dst;
    
    src.w=8; src.h=8; src.x = 103; src.y = 100; dst.x = x; dst.y = y;
    SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    
    src.x = 112;
    for (int i = 8; i < w-8; i+=16) {
        dst.x = x+i; src.w=16;
        while (dst.x + src.w > x+w && src.w>0) src.w--;
        if (src.w>0) SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    }
    
    src.w=8; src.x = 129; dst.x = x+w-8;
    SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    
    src.y = 109; src.w=8;
    for (int j = 8; j < h-8; j+=16) {
        dst.y = y + j;
        src.x = 103; dst.x = x; src.h=16;
        while (dst.y + src.h > y+h && src.h>0) src.h--;
        if (src.h>0) SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    
        src.x = 129; dst.x = x+w-8;
        if (src.h>0)SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    }
    
    src.h=8; src.x = 103; src.y = 126; dst.x = x; dst.y = y+h-8;
    SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    
    src.x = 112;
    for (int i = 8; i < w-8; i+=16) {
        dst.x = x+i; src.w=16;
        while (dst.x + src.w > x+w && src.w>0) src.w--;
        if (src.w>0) SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
    }
    
    src.w=8; src.x = 129; dst.x = x+w-8;
    SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
}

void Texte::setTexte(int idTxt, int vx, int vy, int vw, int vh, bool cadr, bool defil, int vit) {
    if (idTxt == 0) return;
    id = idTxt; 
    idsuiv = 0;
    buffer = "";
    chercheText();
    
    x = vx; y = vy; w = vw; h = vh;
    decoupeText();
        
    def=defil;
    if (def) av = 0;
    else av = texte.length();
    
    cadre = cadr;
    
    vitesse = vit;
}

void Texte::decoupeText() {
    //compte le nombre de caractères possibles et largeur et en hauteur
    int nbcol = (w-16)/6 -1;
    int nblig = (h-16)/16;
    int tailleMax = nbcol * nblig;
    int taille;
    
    //parcours du texte à afficher; à chaque début de mot, 
    //vérifie que le mot peut tenir sur la ligne
    for (int i = 0; i < (int)texte.length(); i++) {
        
        //supprime les espaces isolés en début de ligne
        if (texte.at(i)==' ' && texte.at(i+1)!=' ' && i%nbcol == 0) texte.erase(i,1);
        //recherche du début du prochain mot
        while(texte.at(i)==' ' && i < (int)texte.length()-1) i++;
        
        //saute une ligne si trouve une étoile
        if (texte.at(i)=='*') {
            texte.erase(i,1);//replace(i, 1, " ");
            int nb = (nbcol)-(i%(nbcol));
            for (int j = 0; j < nb; j++) texte.insert(i," ");
            continue;
        }
        
        //si le mot dépasse
        taille = tailleMot(i);
        if ((i%nbcol)+taille>nbcol) {
            if  (i < tailleMax) {
                //si le mot ne tient pas sur une ligne, on le coupe avec des tirets
                if (taille>nbcol) {
                    texte.insert(((i/nbcol)+1)*nbcol-1,"--");
                    i = 1+((i/nbcol)+1)*nbcol;
                }
                //sinon, on ajoute des espaces pour faire commencer le mot à la ligne
                else while((i%nbcol) != 0) {texte.insert(i," "); i++;}
            }
        }
        
    }
    
    // si le texte est trop grand, on le coupe en deux
    if ((int)texte.length() > tailleMax) {
        buffer = texte.substr(tailleMax);
        texte = texte.substr(0, tailleMax);
    }
}

int Texte::tailleMot(int deb) {
    int i = deb;
    int total = 0;
    while (texte.at(i)!=' ') {total++; i++; if (i >= (int)texte.length()) return total;}
    return total;
}

void Texte::afficheLettre(SDL_Surface* gpScreen, char c, int vx, int vy) {
    SDL_Rect src;
    SDL_Rect dst;
    
    int val = (int)c;
    
    dst.x=vx; dst.y=vy;
    src.h=16;src.w=8;
    
    if(val==32) return;
    
    // /
    if(val==47) {src.x=52;src.y=151;}
    
    // @ hylien
    if(val==64) {src.x=4;src.y=151;}
            
    // + hylien
    if(val==43) {src.x=20;src.y=151;}
            
    // = hylien
    if(val==61) {src.x=36;src.y=151;}
            
    //minuscules a-z
    if(val>=97 && val<=122) {src.x=4+16*((val-97)%10); src.y=52+16*((val-97)/10);}
            
    //majuscules A-Z
    if(val>=65 && val<=90) {src.x=6+16*((val-65)%10); src.y=2+16*((val-65)/10);}   
    // ç
    if(val==-25) {src.x=148;src.y=34;}
    // é
    if(val==-23) {src.x=100;src.y=84;}
    // ê
    if(val==-22) {src.x=116;src.y=84;}
    // è
    if(val==-24) {src.x=132;src.y=84;}
    // ë
    if(val==-21) {src.x=132;src.y=151;}
    // à
    if(val==-32) {src.x=148;src.y=84;}
    // â
    if(val==-30) {src.x=148;src.y=103;}
    // ä
    if(val==-28) {src.x=148;src.y=135;}
    // î
    if(val==-18) {src.x=84;src.y=119;}
    // ï
    if(val==-17) {src.x=116;src.y=151;}
    // û
    if(val==-5) {src.x=84;src.y=103;}
    // ù
    if(val==-7) {src.x=148;src.y=151;}
    // ü
    if(val==-4) {src.x=116;src.y=135;}
    // ö
    if(val==-10) {src.x=132;src.y=135;}
    // ô
    if(val==-12) {src.x=148;src.y=119;}
            
    //ponctuation
    // -
    if(val==45) {src.x=102;src.y=34;}
    // .
    if(val==46) {src.x=118;src.y=34;}
    // ,
    if(val==44) {src.x=134;src.y=34;}
    // !
    if(val==33) {src.x=3;src.y=135;}
    // ?
    if(val==63) {src.x=19;src.y=135;}
    // (
    if(val==40) {src.x=35;src.y=135;}
    // )
    if(val==41) {src.x=51;src.y=135;}            
    // ' ( avec @ )
    if(val==39) {src.x=67;src.y=135;}
    // :
    if(val==58) {src.x=83;src.y=135;}
    // ... ( avec % )
    if(val==37) {src.x=101;src.y=135;}
    // >
    if(val==62) {src.x=100;src.y=151;}
    // <
    if(val==60) {src.x=84;src.y=151;}
            
    //chiffres            
    if(val>=48 && val<=57) {src.x=3+16*((val-48)%5); src.y=103+16*((val-48)/5);}
    
    SDL_BlitSurface(imageFont, &src, gpScreen, &dst);
}

bool Texte::hasNext() {
    return (buffer != "" || idsuiv > 0);
}

bool Texte::suite() {
    if (av < (int)texte.length()) {
        av = texte.length();
        return true;
    }
    if (!hasNext()) {
        gpJeu->getAudio()->playSound(18);
        return gpJeu->finTexte(id);
    }
    if (buffer != "") {
        texte = buffer;
        buffer = "";
    }
    else {
        id = idsuiv;
        idsuiv = 0;
        chercheText();
    }
    decoupeText();
    if (def) av = 0;
    else av = texte.length();
    gpJeu->getAudio()->playSound(17);
    return true;
}
