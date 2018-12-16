#pragma once
using offsets = std::vector<uint64_t>;

class Memory
{
private:
	HANDLE pHandle;
	int pID;
	uint64_t BaseAddress;
	std::string FilePath;
	uint64_t iSize;
public:
	Memory() { pHandle = INVALID_HANDLE_VALUE; pID = 0; BaseAddress = 0; iSize = 0; }
	Memory(const char*, const char* = nullptr);
	int attach(const char*, const char* = nullptr);
	~Memory();
	bool isHex(char c);
	std::string getFilePath();
	uint64_t getBaseAddress();
	int getProcessId();
	uint64_t getImageSize();
	HANDLE getHandle();
	void initVars(MODULEENTRY32*);
	DWORD getProcess(const char*, PROCESSENTRY32*);
	DWORD getModule(const char*, MODULEENTRY32*, PROCESSENTRY32*);
	uint64_t getAddress(uint64_t, offsets = {});
	uint64_t patternScan(char*, std::string, uint64_t = 0);
	uint64_t patternScan(const std::string&, uint64_t = 0);
	bool createPattern(const std::string&, std::string&, std::string&);

	template<typename T> 
	int read(uint64_t Address, T* ret, offsets offsets = {})
	{
		//T ret;
		if (Address == 0)
			return -1;
		uint64_t nAddress = getAddress(Address, offsets);
		return ReadProcessMemory(pHandle, (void*)nAddress, ret, sizeof(T), 0);
	}

	template<typename T> 
	int write(uint64_t Address, T value, offsets offsets = {})
	{
		uint64_t nAddress = getAddress(Address, offsets);
		return WriteProcessMemory(pHandle, (void*)nAddress, &value, sizeof(T), 0);
	}

	std::string readString(uint64_t, int, offsets = {});
	void writeString(uint64_t, std::string&, offsets = {});
};