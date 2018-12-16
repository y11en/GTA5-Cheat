#include "stdafx.h"
#include "hack.h"


hack* hack::m_instance = nullptr;
Memory* hack::m_mem = nullptr;

int raw_god_mod(Memory* mem);
int raw_hackinit(Memory* mem);
int raw_transfer_addr(Memory* mem);
int raw_set_bunker_profit(Memory* mem, int n);

int hack::open(std::string process, std::string module)
{
	int retCode = -1;
	if (module.length() == 0)
	{
		retCode = m_mem->attach(process.c_str());
	}
	else
	{
		retCode = m_mem->attach(process.c_str(), module.c_str());
	}
	if (retCode > 0)
		return raw_hackinit(m_mem);
	else
		return retCode;
}
int hack::god_mod()
{
	return raw_god_mod(m_mem);
}

int hack::set_bunker_profit(int n)
{
	return raw_set_bunker_profit(m_mem, n);
}
int hack::transfer_addr()
{
	return raw_transfer_addr(m_mem);
 }
int hack::close()
{
	return 0;
}

/*
.text:00000000013FEF14 4C 8D 05 55 45 57 01       lea     r8, g_gobalPTR
.text:00000000013FEF1B 4D 8B 08                    mov     r9, [r8]
*/

int raw_hackinit(Memory* mem)
{
	int retCode = 0;
	// 找到世界指针
#if __release__
	VMProtectBegin("raw_hackinit");
#endif
/*
// 特征码找
	uint64_t world_sig = mem->patternScan("48 8B 05 ? ? ? ? 48 8B 40 08 48 85 C0");
	uint64_t gobal_sig = mem->patternScan("4C 8D 05 ? ? ? ? 4D 8B 08 4D 85 C9 74 11");
	
	int offset_EIP = 0;
	if (!mem->read<int>(world_sig + 3, &offset_EIP))
	{
		retCode = -1;
		goto _exit;
	}

	gobal::d::worldPTR = world_sig + offset_EIP + 7;

	if (!mem->read<int>(gobal_sig + 3, &offset_EIP))
	{
		retCode = -2;
		goto _exit;
	}

	gobal::d::gobalPTR = gobal_sig + offset_EIP + 7;
*/

	// 1.46 偏移 
	gobal::d::worldPTR = 0x24839c8 + mem->getBaseAddress();
	gobal::d::gobalPTR = 0x2d0f3f0 + mem->getBaseAddress();
	gobal::d::blipPTR = 0x2078700 + mem->getBaseAddress();

	


	// 无敌模式
	gobal::s::godMod = false;
	gobal::d::godMod_heath = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x280 });
	gobal::d::godMod_switch = mem->getAddress(gobal::d::worldPTR, { 0x8,0x189 });
	gobal::d::name = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x10B8, 0x7C });

	// 传送
	//gobal::d::address_player_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x50 });
	//gobal::d::address_player_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x54 });
	//gobal::d::address_player_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x58 });

	//gobal::d::address_car_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x90 });
	//gobal::d::address_car_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x94 });
	//gobal::d::address_car_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x98 });
	// 目的地
	//gobal::d::address_waypoint = mem->getAddress(mem->getBaseAddress() + 0x2CA3218, { 0xD8, 0x340, 0x6C8 });

	//地堡

	// 1.44
	// gobal::d::bunk_num = mem->getAddress(gobal::d::gobalPTR - 0x128, { 0x1180,0x4088 });
	
	// 1.46
	gobal::d::bunk_num = mem->getAddress(gobal::d::gobalPTR - 0x128, { 0x1180,0x4088 }) + 0xA0;

	//printf("address_waypoint = %I64x\n", gobal::d::address_waypoint);

	//printf("address_car_Xpos = %I64x\n", gobal::d::address_car_Xpos);
	//printf("address_car_Ypos = %I64x\n", gobal::d::address_car_Ypos);
	//printf("address_car_Zpos = %I64x\n", gobal::d::address_car_Zpos);
_exit:
#if __release__
	VMProtectEnd();
#endif
	return retCode;
}

int raw_god_mod(Memory* mem)
{
	bool bval = 0;
	float fval = 200.0;
	if (! gobal::s::godMod)
	{
		bval = 1;
		fval = 2600;
	}

	if (mem->write<BYTE>(gobal::d::godMod_switch, bval) &&
		mem->write<float>(gobal::d::godMod_heath, fval))
	{
		gobal::s::godMod ^= 1;
	}
	return gobal::s::godMod;
}

int raw_set_bunker_profit(Memory* mem,int n)
{
	int retcode = 0;
	gobal::s::max_profit = n;
	if (!mem->write<int>(gobal::d::bunk_num, (int)gobal::s::max_profit))
	{
		gobal::s::max_profit = 5;
		retcode = -1;
	}
	return retcode;
}

int raw_transfer_addr(Memory* mem)
{
	int i = 0;
	float _x, _y, _z;
	for (int i = 2000; i; i--)
	{
		UINT64 n = mem->getAddress(gobal::d::blipPTR + i * sizeof(PVOID));
		int _b1 = 0, _b2 = 0, _b3 = 0;

		mem->read<int>(n, &_b1, { 0x40 });
		mem->read<int>(n, &_b2, { 0x48 });

		if ((n > 0) &&
			8 == _b1 &&
			84 == _b2)
		{
			// 传送
			mem->read(gobal::d::worldPTR, &_b3, { 0x8,0x1468 });

			if (_b3 == 2)
			{
				gobal::d::address_car_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x50 });
				gobal::d::address_car_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x54 });
				gobal::d::address_car_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x30, 0x58 });

				gobal::d::address_player_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x90 });
				gobal::d::address_player_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x94 });
				gobal::d::address_player_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0xD28, 0x98 });
			}
			else
			{
				gobal::d::address_car_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x50 });
				gobal::d::address_car_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x54 });
				gobal::d::address_car_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x30, 0x58 });

				gobal::d::address_player_Xpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x90 });
				gobal::d::address_player_Ypos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x94 });
				gobal::d::address_player_Zpos = mem->getAddress(gobal::d::worldPTR, { 0x8, 0x98 });
			}
			
			//printf("[debug]gobal::d::blipPTR = %I64X n= %I64d b1= %d _b2= %d \n", gobal::d::blipPTR, n, _b1, _b2);

			mem->read<float>(n, &_x, { 0x10 });
			mem->read<float>(n, &_y, { 0x14 });

			//_z = GTA.read <float>(n + 0x10, {});

			_z = -210.0f;

			mem->write<float>(gobal::d::address_car_Xpos, _x);
			mem->write<float>(gobal::d::address_car_Ypos, _y);
			mem->write<float>(gobal::d::address_car_Zpos, _z);

			mem->write<float>(gobal::d::address_player_Xpos, _x);
			mem->write<float>(gobal::d::address_player_Ypos, _y);
			mem->write<float>(gobal::d::address_player_Zpos, _z);

		}
	}
	return i;
}

#include <sapi.h>
#pragma comment(lib,"ole32.lib")	//CoInitialize CoCreateInstance需要调用ole32.dll
#pragma comment(lib,"sapi.lib")		//sapi.lib在SDK的lib目录,必需正确配置

int string2voc(WCHAR*voc)
{
#if __release__ 
	VMProtectBegin("string2voc");
#endif
	ISpVoice * pVoice = NULL;
	if (voc)
	{
		//COM初始化： 
		if (FAILED(::CoInitialize(NULL)))
			return FALSE;

		//获取ISpVoice接口： 
		HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
		if (SUCCEEDED(hr))
		{
			hr = pVoice->Speak(voc, 0, NULL);
			pVoice->Release();
			pVoice = NULL;
		}

		//千万不要忘记： 
		::CoUninitialize();
	}

#if __release__ 
	VMProtectEnd();
#endif
	return TRUE;

}