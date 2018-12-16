#pragma once
#include "pch.h"
#include <iostream>

#define GTA5_DEF_NOT_SET_WAYPOINT 64000

namespace gobal
{
	namespace s	//状态
	{
		static bool godMod;
		static float player_Xpos;
		static float player_Ypos;
		static float player_Zpos;

		static float car_Xpos;
		static float car_Ypos;
		static float car_Zpos;

		//地堡收益
		static uint64_t max_profit;

	}

	namespace d //数据
	{
		static uint64_t worldPTR;
		static uint64_t gobalPTR;
		static uint64_t blipPTR;

		static uint64_t godMod_heath;
		static uint64_t godMod_switch;
		static uint64_t name;

		//地堡货物
		static uint64_t bunk_numPTR;

		static uint64_t address_player_Xpos;
		static uint64_t address_player_Ypos;
		static uint64_t address_player_Zpos;

		static uint64_t address_car_Xpos;
		static uint64_t address_car_Ypos;
		static uint64_t address_car_Zpos;

		static uint64_t address_waypoint;
	}
	
};
