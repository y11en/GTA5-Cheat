// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <conio.h>
#include "stdio.h"
#include "hack.h"


#define KEY_DOWN(VKEY) ((GetAsyncKeyState(VKEY) & 0x8000) ? 1:0) //必要的，我是背下来的



void color(int c) {//改变输出的颜色，比system("color x")快得多
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// 无敌
// 传送

int initData(Memory& mem)
{
	int retCode = -1;

	// 找到世界指针
	uint64_t world_sig = mem.patternScan("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0");
	uint64_t gobal_sig = mem.patternScan("4C 8D 05 ? ? ? ? 4D 8B 08 4D 85 C9 74 11");
	uint64_t blip_sig = mem.patternScan("4C 8D 05 ? ? ? ? 0F B7 C1");
	
	/*
	.text:00000000013FEF14 4C 8D 05 55 45 57 01       lea     r8, g_gobalPTR
	.text:00000000013FEF1B 4D 8B 08                   mov     r9, [r8]
	*/

	int offset_EIP = 0;
	if (mem.read<int>(world_sig + 3, &offset_EIP))
	{
		gobal::d::worldPTR = world_sig + offset_EIP + 7;
		retCode = 0;
	}
	if (mem.read<int>(gobal_sig + 3, &offset_EIP))
	{
		gobal::d::gobalPTR = gobal_sig + offset_EIP + 7;
		retCode = 0;
	}
	if (mem.read<int>(blip_sig + 3, &offset_EIP))
	{
		gobal::d::blipPTR = blip_sig + offset_EIP + 7;
		retCode = 0;
	}
	
	// 无敌模式
	gobal::s::godMod = false;
	gobal::d::godMod_heath = mem.getAddress(gobal::d::worldPTR, { 8, 0x280 });
	gobal::d::godMod_switch = mem.getAddress(gobal::d::worldPTR, {0x8,0x189});
	gobal::d::name = mem.getAddress(gobal::d::worldPTR, { 8, 0x10B8, 0x7C });


	// 目的地
	gobal::d::address_waypoint = mem.getAddress(mem.getBaseAddress() + 0x2CA3218, { 0xD8, 0x340, 0x6C8 });
	
	//地堡
	gobal::d::bunk_numPTR = mem.getAddress(gobal::d::gobalPTR - 0x128, {0x1180,0x4088 }) + 0xA0;

	printf("gobal::d::address_player_Xpos  = %I64d\ngobal::d::address_car_Xpos = %I64d\n", 
		gobal::d::address_player_Xpos, gobal::d::address_car_Xpos);

	printf("off worldptr = %I64x\n", gobal::d::worldPTR - mem.getBaseAddress()); // 1.44 0x2413410
	printf("off gobalptr = %I64x\n", gobal::d::gobalPTR - mem.getBaseAddress()); // 1.44 0x2c9eca0
	printf("off blipptr = %I64x\n", gobal::d::blipPTR - mem.getBaseAddress()); // 1.44 
	// 1.46 24839c8
	// 1.46 2d0f3f0
	// 1.46 2078700

	return retCode;
}


uint64_t start()
{
	Memory GTA("GTA5.exe");
	float Health;
	std::string name;
	char* buff = NULL;
	const int szbuffSize = 1024;

	buff = new char[szbuffSize];
	ZeroMemory(buff, szbuffSize);
	char ch = 0;

	bool bval = 0;
	int ival = 0, i = 0;;
	float fval = 0.0;
	float _x, _y, _z;

	if (!initData(GTA))
	{
		if (GTA.read<float>(gobal::d::godMod_heath, &Health))
		{
			//printf("Health is %.2f\n", Health);
			name = GTA.readString(gobal::d::name, 12);
			//printf("Playername: %s\n", &name[0]);	
		}

		while (1)
		{
			if (_kbhit())
			{
				ch = _getch();
				if (ch <= 0)
				{
					ch = _getch();
					switch (ch)
					{
						case 61: //F3
							bval = (!gobal::s::godMod) ? 1 : 0;
							fval = (!gobal::s::godMod) ? 2600 : 0;
							if (!gobal::s::godMod)
							{
								bval = 1;
								fval = 2600;
							}
							else
							{
								bval = 0;
								fval = 200;
							}

							if (GTA.write<BYTE>(gobal::d::godMod_switch, bval) &&
								GTA.write<float>(gobal::d::godMod_heath, fval))
							{
								gobal::s::godMod ^= 1;
							}


							break;

						case 62: //F4
							for (int i = 2000; i; i--)
							{
								UINT64 n = GTA.getAddress(gobal::d::blipPTR+ i * sizeof(PVOID));
								int _b1 = 0, _b2 = 0, _b3 = 0;

								GTA.read<int>(n, &_b1, { 0x40 });
								GTA.read<int>(n, &_b2, { 0x48 });

								if ((n > 0) && 
									8 == _b1 &&
									84 == _b2 )
								{
									// 传送
									GTA.read(gobal::d::worldPTR, &_b3, { 0x8,0x1468 });

									if (_b3 == 2)
									{
										gobal::d::address_car_Xpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x50 });
										gobal::d::address_car_Ypos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x54 });
										gobal::d::address_car_Zpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x58 });

										gobal::d::address_player_Xpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x90 });
										gobal::d::address_player_Ypos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x94 });
										gobal::d::address_player_Zpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x98 });
									}
									else
									{
										gobal::d::address_car_Xpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x50 });
										gobal::d::address_car_Ypos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x54 });
										gobal::d::address_car_Zpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x58 });

										gobal::d::address_player_Xpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x90 });
										gobal::d::address_player_Ypos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x94 });
										gobal::d::address_player_Zpos = GTA.getAddress(gobal::d::worldPTR, { 0x8, 0x98 });
									}



									printf("[debug]gobal::d::blipPTR = %I64X n= %I64d b1= %d _b2= %d \n", gobal::d::blipPTR, n, _b1, _b2);

									GTA.read<float>(n, &_x, { 0x10 });
									GTA.read<float>(n, &_y, { 0x14 });

									//_z = GTA.read <float>(n + 0x10, {});

									_z = -210;

									GTA.write<float>(gobal::d::address_car_Xpos, _x);
									GTA.write<float>(gobal::d::address_car_Ypos, _y);
									GTA.write<float>(gobal::d::address_car_Zpos, _z);

									GTA.write<float>(gobal::d::address_player_Xpos, _x);
									GTA.write<float>(gobal::d::address_player_Ypos, _y);
									GTA.write<float>(gobal::d::address_player_Zpos, _z);
								
								}
							}



							break;
						case 59: //F1
							// 地堡X倍收益
							
							if (!gobal::s::max_profit)
							{
								printf("输入地堡收益倍数:");
								std::cin >> gobal::s::max_profit;
							}
							else
							{
								if ( ! GTA.write<int>(gobal::d::bunk_numPTR, gobal::s::max_profit))
								{
									gobal::s::max_profit = 0;
								}
							}
							break;
					}

				}
				else
				{

				}
				
				if (GTA.read<float>(gobal::d::godMod_heath, &Health)
					&& GTA.read<bool>(gobal::d::godMod_switch, &bval)
					&& GTA.read<float>(gobal::d::address_player_Xpos,&gobal::s::player_Xpos)
					&& GTA.read<float>(gobal::d::address_player_Ypos,&gobal::s::player_Ypos)
					&& GTA.read<float>(gobal::d::address_player_Zpos,&gobal::s::player_Zpos))
				{
					//system("cls");
					gobal::s::godMod = bval;
					sprintf_s(buff, szbuffSize - 1, "玩家昵称:%s\n当前血量:%0.2f\n当前位置:{x:%0.1f, y:%0.1f, z:%0.1f}\n无敌状态:[%s]\n紫点传送:{x:%0.1f, y:%0.1f, z:%0.1f}\n地堡收益倍数:%d",
						name.c_str(),
						Health,
						gobal::s::player_Xpos,
						gobal::s::player_Ypos,
						gobal::s::player_Zpos,
						gobal::s::godMod ? "√" : "×",
						_x == GTA5_DEF_NOT_SET_WAYPOINT ? 0.0 : _x,
						_y == GTA5_DEF_NOT_SET_WAYPOINT ? 0.0 : _y,
						_z == GTA5_DEF_NOT_SET_WAYPOINT ? 0.0 : _z,
						gobal::s::max_profit);

					printf("%s\n", buff);
				}

			}
		}
	}

	if (buff)
		delete buff;

	return 1;
}


int main()
{
	srand(time(0));
	return start();
}
