#include "stdafx.h"


Memory::Memory(const char* processName, const char* moduleName)
{
	pHandle = INVALID_HANDLE_VALUE; pID = 0; BaseAddress = 0; iSize = 0;
	attach(processName, moduleName);
}

int Memory::attach(const char* processName, const char* moduleName)
{
	PROCESSENTRY32 pe32;
	MODULEENTRY32 pe_module;
	if (Memory::pHandle == INVALID_HANDLE_VALUE)
	{
		if (Memory::getProcess(processName, &pe32) > 0 &&
			Memory::getModule(moduleName == nullptr ? processName : moduleName, &pe_module, &pe32) > 0)
		{
			Memory::initVars(&pe_module);
			Memory::pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pID);
		}
	}
	return (int)Memory::pHandle;
}


Memory::~Memory()
{
	CloseHandle(pHandle);
}

std::string Memory::getFilePath()
{
	return FilePath;
}

uint64_t Memory::getBaseAddress()
{
	return BaseAddress;
}

int Memory::getProcessId()
{
	return pID;
}

uint64_t Memory::getImageSize()
{
	return iSize;
}

HANDLE Memory::getHandle()
{
	return pHandle;
}

void Memory::initVars(MODULEENTRY32* m)
{
#ifndef _MBCS
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> w2sConverter;
	Memory::FilePath = w2sConverter.to_bytes(m->szExePath).c_str();
#else
	Memory::FilePath = m->szExePath;
#endif
	Memory::BaseAddress = (uint64_t)m->modBaseAddr;
	Memory::pID = m->th32ProcessID;
	Memory::iSize = m->modBaseSize;
}

DWORD Memory::getProcess(const char* processName, PROCESSENTRY32* process)
{
#ifndef _MBCS
	std::wstring wprocessName;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> s2wconverter;
#endif

	DWORD PID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
		goto _exit;

	process->dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnap, process) == 0)
		CloseHandle(hSnap);
#ifndef _MBCS
	wprocessName = s2wconverter.from_bytes(processName);
#endif
	do
	{
#ifndef _MBCS
		if (lstrcmp(process->szExeFile, &wprocessName[0]) == 0)
#else
		if (lstrcmp(process->szExeFile, processName) == 0)
#endif
		{
			PID = process->th32ProcessID;
			break;
		}
			
	} while (Process32Next(hSnap, process));
	CloseHandle(hSnap);

_exit:
	return PID;
}

DWORD Memory::getModule(const char* moduleName, MODULEENTRY32* module, PROCESSENTRY32* process)
{
#ifndef _MBCS
	std::wstring wmoduleName;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> s2wconverter;
#endif
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process->th32ProcessID);
	DWORD PID = 0;
	if (hSnap == INVALID_HANDLE_VALUE)
		goto _exit;

	module->dwSize = sizeof(MODULEENTRY32);
	if (Module32First(hSnap, module) == 0)
		CloseHandle(hSnap);
#ifndef _MBCS
	wmoduleName = s2wconverter.from_bytes(moduleName);
#endif
	do
	{
#ifndef _MBCS
		if (lstrcmp(module->szModule, &wmoduleName[0]) == 0)
#else
		if (lstrcmp(module->szModule, moduleName) == 0)
#endif
		{
			PID = module->th32ProcessID;
			break;
		}
			
	} while (Module32Next(hSnap, module));
	CloseHandle(hSnap);

_exit:
	return PID;
}

bool Memory::isHex(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Memory::createPattern(const std::string& pattern, std::string& pattern_result, std::string& mask_result)
{
	bool result = false;
	char buffer[2];
	std::stringstream pattern_s;
	std::stringstream mask_s;

	if (pattern.empty())
		return result;

	for (size_t i = 0, l = pattern.size() - 1; i < l; i++)
	{
		if (Memory::isHex(pattern[i]))
		{
			buffer[0] = pattern[i];
			buffer[1] = (l >= i + 1 && Memory::isHex(pattern[i + 1])) ? pattern[++i] : 0;
			pattern_s << (char)strtol(buffer, nullptr, 16);
			mask_s << 'x';
			continue;
		}
		else if (pattern[i] == '?' || pattern[i] == '*')
		{
			pattern_s << "\x90";
			mask_s << '?';
			continue;
		}
	}
	result = true;
	pattern_result = pattern_s.str();
	mask_result = mask_s.str();

	return result;
}

uint64_t Memory::patternScan(const std::string& pattern, uint64_t startAddress)
{
	std::string sub_ptr;
	std::string sub_mask;
	Memory::createPattern(pattern, sub_ptr, sub_mask);
	return Memory::patternScan(&sub_ptr[0], sub_mask, startAddress);
}

uint64_t Memory::patternScan(char* sig, std::string pattern, uint64_t startAddress)
{
	uint64_t ret = 0;
	if (startAddress < Memory::BaseAddress || startAddress >(Memory::BaseAddress + Memory::iSize))
		startAddress = Memory::BaseAddress;
	char* cBuffer = new char[Memory::iSize];
	ReadProcessMemory(Memory::pHandle, (void*)startAddress, cBuffer, Memory::iSize, 0);
	size_t patternLength = pattern.size() - 1;

	for (size_t i = 0, j = 0; i != Memory::iSize; i++)
	{
		if (cBuffer[i] == sig[j] || pattern[j] == '?')
		{
			if (j == patternLength)
			{
				ret = (startAddress + i - j);
				goto PATTERN_SCAN_END;
			}

			j++;
		}
		else if (j > 0 && (cBuffer[i] == sig[0] || pattern[0] == '?'))
		{
			j = 1;
		}
		else
		{
			j = 0;
		}
	}
PATTERN_SCAN_END:
	delete[] cBuffer;
	return ret;
}


uint64_t Memory::getAddress(uint64_t Address, offsets offsets)
{
	uint64_t nAddress = Address;
	if (Address == 0)
		return nAddress;
	size_t size = offsets.size() - 1;
	if (offsets.empty())
		return nAddress;

	ReadProcessMemory(Memory::pHandle, (void*)nAddress, &nAddress, sizeof(nAddress), 0);
	for (size_t i = 0; i != size; i++)
		ReadProcessMemory(Memory::pHandle, (void*)(nAddress + offsets[i]), &nAddress, sizeof(nAddress), 0);
	nAddress = nAddress + offsets[size];

	return nAddress;
}

std::string Memory::readString(uint64_t Address, int size, offsets offsets)
{
	std::string ret;
	if (Address == 0)
		return ret;
	uint64_t nAddress = Memory::getAddress(Address, offsets);
	char* buffer = new char[size];
	ReadProcessMemory(Memory::pHandle, (void*)nAddress, buffer, size, 0);
	ret = std::string(buffer);
	delete[] buffer;
	return ret;
}

void Memory::writeString(uint64_t Address, std::string& str, offsets offsets)
{
	uint64_t nAddress = Memory::getAddress(Address, offsets);
	WriteProcessMemory(Memory::pHandle, (void*)nAddress, &str[0], str.size(), 0);
	return;
}
