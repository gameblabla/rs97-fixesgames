#include "stone.h"

#include <stddef.h>

int stoneAmountOfStonesOfType(stoneType type)
{
	switch (type)
	{
		case SEASON_SPRING:
		case SEASON_SUMMER:
		case SEASON_AUTUMN:
		case SEASON_WINTER:
		case FLOWER_PLUM:
		case FLOWER_ORCH:
		case FLOWER_BAMB:
		case FLOWER_CHRY:
		return 1;

		default:
		return 4;
	}
}

int stoneCheckMatchingTypes(stoneType typeA, stoneType typeB)
{
	if (typeA == typeB)
	{
		return 1;
	}
	else if (typeA >= SEASON_SPRING && typeA <= SEASON_WINTER && typeB >= SEASON_SPRING && typeB <= SEASON_WINTER)
	{
		return 1;
	}
	else if (typeA >= FLOWER_PLUM && typeA <= FLOWER_CHRY && typeB >= FLOWER_PLUM && typeB <= FLOWER_CHRY)
	{
		return 1;
	}

	return 0;
}

const char *stoneRankText(stoneType type)
{
	switch (type)
	{
		case DOT_ONE:
		case BAMBOO_ONE:
		case CHAR_ONE:
			return "1";
		case DOT_TWO:
		case BAMBOO_TWO:
		case CHAR_TWO:
			return "2";
		case DOT_THREE:
		case BAMBOO_THREE:
		case CHAR_THREE:
			return "3";
		case DOT_FOUR:
		case BAMBOO_FOUR:
		case CHAR_FOUR:
			return "4";
		case DOT_FIVE:
		case BAMBOO_FIVE:
		case CHAR_FIVE:
			return "5";
		case DOT_SIX:
		case BAMBOO_SIX:
		case CHAR_SIX:
			return "6";
		case DOT_SEVEN:
		case BAMBOO_SEVEN:
		case CHAR_SEVEN:
			return "7";
		case DOT_EIGHT:
		case BAMBOO_EIGHT:
		case CHAR_EIGHT:
			return "8";
		case DOT_NINE:
		case BAMBOO_NINE:
		case CHAR_NINE:
			return "9";
		case WIND_EAST:
			return "E";
		case WIND_SOUTH:
			return "S";
		case WIND_WEST:
			return "W";
		case WIND_NORTH:
			return "N";
		case DRAGON_GREEN:
			return "g";
		case DRAGON_WHITE:
			return "w";
		case DRAGON_RED:
			return "r";
		case SEASON_SPRING:
			return "SP";
		case SEASON_SUMMER:
			return "SU";
		case SEASON_AUTUMN:
			return "AU";
		case SEASON_WINTER:
			return "WI";
		case FLOWER_PLUM:
			return "pl";
		case FLOWER_ORCH:
			return "or";
		case FLOWER_CHRY:
			return "ch";
		case FLOWER_BAMB:
			return "ba";

		default:
			return NULL;
	}
}
