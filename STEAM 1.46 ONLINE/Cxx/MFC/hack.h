#pragma once
#include "stdafx.h"
#include <iostream>

#define GTA5_DEF_NOT_SET_WAYPOINT 64000

#define HOTKEY_BUNKER_PROFIT	4001		// Shift + F1
#define HOTKEY_GOD_MOD		4002		// Shift + F3
#define HOTKEY_TRANSFER		4003		// Shift + F4

#define GAME_PROC_NAME	"GTA5.exe"

int string2voc(WCHAR* voc);
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
		static uint64_t bunk_num;

		static uint64_t address_player_Xpos;
		static uint64_t address_player_Ypos;
		static uint64_t address_player_Zpos;

		static uint64_t address_car_Xpos;
		static uint64_t address_car_Ypos;
		static uint64_t address_car_Zpos;

		static uint64_t address_waypoint;
	}
	
};


class hack
{
private:
	hack() { }
	static hack* m_instance;
	static Memory* m_mem;
public:
	virtual ~hack() {}
	static hack* getInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new hack();
			m_mem = new Memory();
		}

		return m_instance;
	}
	static void destroyIntance()
	{
		if (m_mem != nullptr)
		{
			delete m_mem;
			m_mem = nullptr;
		}
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	static int open(std::string process, std::string module = "");
	static int god_mod();
	static int set_bunker_profit(int n);
	static int transfer_addr();
	static int close();
};
