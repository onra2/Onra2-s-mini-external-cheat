#include "directX.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <TlHelp32.h>
#include <string>
#include "menu.h"
#include <cassert>
#include <vector>
#include "vector.h"
#include <TCHAR.h>
#include <cstdlib>
#include <sstream>

#pragma region d3d
IDirect3D9Ex* p_Object = 0;
IDirect3DDevice9Ex* p_Device = 0;
D3DPRESENT_PARAMETERS p_Params;

ID3DXLine* p_Line;
ID3DXFont* pFontSmall = 0;
ID3DXFont* pFontLarge = 0;
ID3DXFont* pFontMedium = 0;

int DirectXInit(HWND hWnd)
{
	if(FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(1);

	ZeroMemory(&p_Params, sizeof(p_Params));    
	p_Params.Windowed = TRUE;   
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;    
	p_Params.hDeviceWindow = hWnd;    
	p_Params.MultiSampleQuality   = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8 ;     
	p_Params.BackBufferWidth = Width;    
	p_Params.BackBufferHeight = Height;    
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;

	if(FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
		exit(1);

	if(!p_Line)
		D3DXCreateLine(p_Device, &p_Line);

	D3DXCreateFont(p_Device, 18, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Calibri", &pFontSmall);
	D3DXCreateFont(p_Device, 50, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Calibri", &pFontLarge);
	D3DXCreateFont(p_Device, 40, 0, 0, 0, false, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Calibri", &pFontMedium);
	return 0;
}
#pragma endregion

#pragma region usefullstuff
using namespace std;

DWORD64 Base;
DWORD pid;
HANDLE pHandle;

uintptr_t FindPointer(HANDLE hproc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); i++)
	{
		ReadProcessMemory(hproc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

INT64 readPointer(HANDLE hproc, DWORD64 Address)
{
	INT64 value;
	ReadProcessMemory(hproc, (INT64*)Address, &value, sizeof(value), 0);
	return value;
}

int readInteger(HANDLE hproc, DWORD64 Address)
{
	int value;
	ReadProcessMemory(hproc, (BYTE*)Address, &value, sizeof(value), 0);
	return value;
}

using std::cout;
using std::endl;
using std::string;

struct module
{
	DWORD64 dwBase, dwSize;
};

module TargetModule;
HANDLE TargetProcess;
DWORD64  TargetId;

HANDLE GetProcess(const wchar_t* processName)
{
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	do
		if (!_wcsicmp(entry.szExeFile, processName)) {
			TargetId = entry.th32ProcessID;
			CloseHandle(handle);
			TargetProcess = OpenProcess(PROCESS_ALL_ACCESS, false, TargetId);
			return TargetProcess;
		}
	while (Process32Next(handle, &entry));

	return false;
}

module GetModule(const wchar_t* moduleName) {
	HANDLE hmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, TargetId);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);

	do {
		if (!_wcsicmp(mEntry.szModule, moduleName)) {//_tcscmp
			CloseHandle(hmodule);

			TargetModule = { (DWORD64)mEntry.hModule, mEntry.modBaseSize };
			return TargetModule;
		}
	} while (Module32Next(hmodule, &mEntry));

	module mod = { (DWORD64)false, (DWORD64)false };
	return mod;
}

template <typename var>
bool WriteMemory(DWORD64 Address, var Value) {
	return WriteProcessMemory(TargetProcess, (LPVOID)Address, &Value, sizeof(var), 0);
}

template <typename var>
var ReadMemory(DWORD64 Address) {
	var value;
	ReadProcessMemory(TargetProcess, (LPCVOID)Address, &value, sizeof(var), NULL);
	return value;
}

bool MemoryCompare(const BYTE* bData, const BYTE* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++bData, ++bMask) {
		if (*szMask == 'x' && *bData != *bMask) {
			return false;
		}
	}
	return (*szMask == NULL);
}

DWORD64 FindSignature(DWORD64 start, DWORD64 size, const char* sig, const char* mask)
{
	BYTE* data = new BYTE[size];
	SIZE_T bytesRead;

	ReadProcessMemory(TargetProcess, (LPVOID)start, data, size, &bytesRead);

	for (DWORD64 i = 0; i < size; i++)
	{
		if (MemoryCompare((const BYTE*)(data + i), (const BYTE*)sig, mask)) {
			return start + i;
		}
	}
	delete[] data;
	return NULL;
}

void WriteLogFile(const char *LogText, ...)
{
	FILE *MyLog;
	fopen_s(&MyLog, "C://Onra2//LOGFILE.log", "a");


	char Buffer[1024];
	va_list ArgList;
	va_start(ArgList, LogText);
	vsnprintf_s(Buffer, 1023, LogText, ArgList);
	va_end(ArgList);

	if (MyLog)
	{
		fprintf(MyLog, "%s", Buffer);
		fclose(MyLog);
	}
}

void WriteLog(const char *LogText, ...)
{
	char Buffer[1024];
	va_list ArgList;
	va_start(ArgList, LogText);
	vsnprintf_s(Buffer, 1023, LogText, ArgList);
	va_end(ArgList);

	WriteLogFile("%s\n", Buffer);
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

#pragma endregion

#pragma region Big addresses
DWORD64 WorldPTR;
DWORD64 BlipPTR;
DWORD64 PlayerPTR;
DWORD64 GlobalPTR;
DWORD64 Carspawn;
DWORD64 ObjectSpawn;

uintptr_t WorldPtrBaseAddr;
uintptr_t BlipPtrBaseAddr;
uintptr_t AfkPtrBaseAddr;
uintptr_t dynamicPtrBaseAddr;
uintptr_t BulletInBatchAddress;
uintptr_t DeliveryAddress;
uintptr_t AFKAddress;
uintptr_t WantedAddress;
uintptr_t HealthAddress;
uintptr_t GodmodeAddresstwo;
uintptr_t RagdollAddress;
uintptr_t CarGodModeAddress;
uintptr_t InvisibleAddress;
uintptr_t CarInvisibleAddress;
uintptr_t SprintSpeedAddress;
uintptr_t SwimSpeedAddress;
uintptr_t DmgMultAddress;
uintptr_t VehicleWeaponDmgMultAddress;
uintptr_t MeleeDmgMultAddress;
uintptr_t RpMultAddress;
uintptr_t NoReloadAddress;
uintptr_t MaxHealthAddress;
uintptr_t EngineHealth1Address;
uintptr_t EngineHealth2Address;
uintptr_t EngineHealth3Address;
uintptr_t UnlmVehicleMisslesAddress;
uintptr_t UnlmVehicleAirBombAddress;
uintptr_t UnlmVehicleAirCounterAddress;
uintptr_t UnlmVehicleOppressorMisslesAddress;
uintptr_t UnlmVehicleTampaMisslesAddress;
uintptr_t SeatbeltAddress;
uintptr_t BoostAddress;
uintptr_t BulletDmgAddress;
uintptr_t BulletMassAddress;
uintptr_t MuzzleVelocityAddress;
uintptr_t RangeAddress;
uintptr_t NoRecoilAddress;
uintptr_t SpreadAddress;
uintptr_t PenetrationAddress;
uintptr_t ForceOnPedAddress;
uintptr_t ForceOnVehAddress;
uintptr_t ForceOnHeliAddress;
uintptr_t InstantLockAddress;
uintptr_t InstantRocketAddress;
uintptr_t LockonRangeAddress;
uintptr_t RocketLifetimeAddress;
uintptr_t HomingAbilityAddress;
uintptr_t ThrowableGravAddress;
uintptr_t WeaponWheelAddress;
uintptr_t CurrentAmmoAddress;
uintptr_t AccelerationAddress;
uintptr_t BrakeForceAddress;
uintptr_t HandBrakeAddress;
uintptr_t DeformMultAddress;
uintptr_t TracMinAddress;
uintptr_t TracMaxAddress;
uintptr_t GravityAddress;
uintptr_t VehicleMaxxAddress;
uintptr_t VehicleBuoyancyAddress;
uintptr_t RocketFuelAddress;
uintptr_t RocketRechargeAddress;
uintptr_t BoostMaxSpeedAddress;
uintptr_t CarDmgMultAddress;
uintptr_t CarWeaponDmgMultAddress;
uintptr_t CollisionDmgAddress;
uintptr_t ThrustAddress;
uintptr_t ThrustDeluxoAddress;
#pragma endregion

LPCSTR path = ".\\Onra2.ini";
//values
LPSTR BunkerMultString[255];

void loadSettings()
{
	HWND hWnd = FindWindowA(0, "Grand Theft Auto V");
	GetWindowThreadProcessId(hWnd, &pid);
	pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	Base = GetModuleBaseAddress(pid, L"GTA5.exe");

	/*=================================================================================================================*/
	LPCSTR SignatureWorldPTR = "\x48\x8b\x05\x00\x00\x00\x00\x45\x00\x00\x00\x00\x48\x8b\x48\x08\x48\x85\xc9\x74\x07";
	LPCSTR MaskWorldPTR = "xxx????x????xxxxxxxxx";

	LPCSTR SignatureBlipPTR = "\x4c\x8d\x05\x00\x00\x00\x00\x0f\xb7\xc1";
	LPCSTR MaskBlipPTR = "xxx????xxx";

	LPCSTR SignaturePlayerPTR = "\x48\x8b\x0d\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x48\x8b\xc8\xe8\x00\x00\x00\x00\x48\x8b\xcf";
	LPCSTR MaskPlayerPTR = "xxx????x????xxxx????xxx";

	LPCSTR SignatureGlobalPTR = "\x4c\x8d\x05\x00\x00\x00\x00\x4d\x8b\x08\x4d\x85\xc9\x74\x11";
	LPCSTR MaskGlobalPTR = "xxx????xxxxxxxx";

	if (GetProcess(L"GTA5.exe"))
	{
		module mod = GetModule(L"GTA5.exe");

		DWORD64 TempWorldPTR = FindSignature(mod.dwBase, mod.dwSize, SignatureWorldPTR, MaskWorldPTR);
		WorldPTR = (TempWorldPTR) + readInteger(pHandle, TempWorldPTR + 3) + 7;

		DWORD64 TempBlipPTR = FindSignature(mod.dwBase, mod.dwSize, SignatureBlipPTR, MaskBlipPTR);
		BlipPTR = TempBlipPTR + readInteger(pHandle, TempBlipPTR + 3) + 7;

		DWORD64 TempPlayerPTR = FindSignature(mod.dwBase, mod.dwSize, SignaturePlayerPTR, MaskPlayerPTR);
		PlayerPTR = TempPlayerPTR + readInteger(pHandle, TempPlayerPTR + 3) + 7;

		DWORD64 TempGlobalPTR = FindSignature(mod.dwBase, mod.dwSize, SignatureGlobalPTR, MaskGlobalPTR);
		GlobalPTR = TempGlobalPTR + readInteger(pHandle, TempGlobalPTR + 3) + 7;

		Carspawn = GlobalPTR;
		Carspawn = readPointer(pHandle, (Carspawn + 8 * 9));

		ObjectSpawn = GlobalPTR;
		ObjectSpawn = readPointer(pHandle, (ObjectSpawn + 0x8 * 0x10));
	}

	WorldPtrBaseAddr = (WorldPTR);
	BlipPtrBaseAddr = (BlipPTR);
	AfkPtrBaseAddr = (GlobalPTR + 0x8);
	dynamicPtrBaseAddr = (GlobalPTR - 0x128);

	DeliveryAddress = FindPointer(pHandle, dynamicPtrBaseAddr, Offset.BunkerArray);
	AFKAddress = FindPointer(pHandle, AfkPtrBaseAddr, Offset.AFkArray);
	WantedAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.WantedlvlArray);
	HealthAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.HealthArray);
	RagdollAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.RagdollArray);
	InvisibleAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.InvisibleArray);
	SprintSpeedAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.SprintSpeedArray);
	SwimSpeedAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.SwimSpeedArray);
	DmgMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.DmgMultArray);
	VehicleWeaponDmgMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleWeaponDmgMultArray);
	MeleeDmgMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.MeleeDmgMultArray);

	std::cout << "DeliveryAddress = " << std::hex << DeliveryAddress << std::endl;
	std::cout << "AFKAddress = " << std::hex << AFKAddress << std::endl;
	std::cout << "WantedAddress = " << std::hex << WantedAddress << std::endl;
}

#pragma region variables
bool AFK = false;
int Wantedstars = 0;
bool god = false;
bool Vehiclegod = false;
bool VehicleInvisible = false;
bool bRagdoll = false;
bool bTwoBillion = false;
bool egg = false;
bool invisible = false;
bool offradar = false;
float sprintspeed = 1.0f;
float swimspeed = 1.0f;
bool aimbot = false;
float DamageMult = 0.7200000286f;
float MeleeDamageMult = 1.0f;
bool RPMult = false;
bool noreload = false;
bool infiniteammo = false;
float bulletdmg = 1.0f;
int bulletmass = 1;
int bulletbatch;
float muzzlevelocity;
float Range;
bool Norecoil = false;
float Spread;
float Penetration;
float FonPed;
float FonVehicle;
float FonHeli;
float LockonRange;
float RocketLifetime;
float HomingAbility;
float ThrowGravity;
int WeaponWheelSlot;
float Boost;
bool instantLock = false;
bool instantRocket = false;
bool UnlimitedVehclAmmo = false;
bool Seatbelt = false;
float Acceleration;
float BrakeForce;
float HandBrakeForce;
float TractionMin;
float TractionMax;
float DeformMult;
float Gravity;
float VehicleMass;
float VehicleBuoyancy;
float RocketRechargeSpeed;
int RocketFuelLvl;
float BoostmaxSPeed;
float VehicleDamageMult;
float WeaponVehicleDamageMult;
float ColisionVehicleDamageMult;
float Thrust;
float ThrustDeluxo;
#pragma endregion

void MainMenu()
{
	menuTitle("Onra2 unknowncheats");
	submenuOption("Self", 2, 1);
	submenuOption("Money", 3, 2);
	submenuOption("Weapon", 5, 3);
	submenuOption("Vehicles", 6, 4);
	submenuOption("Misc", 4, 5);
	normalMenuActions();
}

void SelfMenu()
{
	//afk
	if (getOption() == 1)
	{
		AFK = !AFK;
		if (AFK)
		{
			int NoAFK = 1234567890;
			WriteProcessMemory(pHandle, (BYTE*)AFKAddress, &NoAFK, sizeof(NoAFK), 0);
		}
		else
		{
			int NoAFK = 10;
			WriteProcessMemory(pHandle, (BYTE*)AFKAddress, &NoAFK, sizeof(NoAFK), 0);
		}
	}
	//wanted
	if (getOption() == 2)
	{
		WriteProcessMemory(pHandle, (BYTE*)WantedAddress, &Wantedstars, sizeof(Wantedstars), 0);
		std::cout << "Wanted level set" << std::endl;
	}
	//Godmode
	if (getOption() == 3)
	{
		god = !god;
		if (god)
		{
			std::cout << "Godmode" << std::endl;
			float God = 10000;
			bool Godtwo = true;
			GodmodeAddresstwo = FindPointer(pHandle, WorldPtrBaseAddr, Offset.GodModeArray);
			WriteProcessMemory(pHandle, (BOOL*)GodmodeAddresstwo, &Godtwo, sizeof(Godtwo), 0);
		}
		else
		{
			int Godtwo = 0;
			WriteProcessMemory(pHandle, (BOOL*)GodmodeAddresstwo, &Godtwo, sizeof(Godtwo), 0);
		}
	}
	//Ragdoll
	if (getOption() == 4)
	{
		bRagdoll = !bRagdoll;
		if (bRagdoll)
		{
			bool ON = true;
			WriteProcessMemory(pHandle, (BOOL*)RagdollAddress, &ON, sizeof(ON), 0);
		}
		else
		{
			bool OFF = false;
			WriteProcessMemory(pHandle, (BOOL*)RagdollAddress, &OFF, sizeof(OFF), 0);
		}
	}
	//tp waypoint
	if (getOption() == 5)
	{
		int X, Y;
		float Z = -210.0f;
		uintptr_t XPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArrayCar);
		uintptr_t YPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArrayCar);
		uintptr_t ZPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArrayCar);
		uintptr_t XPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArray);
		uintptr_t YPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArray);
		uintptr_t ZPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArray);

		for (int i = 0; i < 2000; i++)
		{
			auto a = BlipPtrBaseAddr;
			auto n = readPointer(pHandle, a + (i * 8));
			if ((n > 0) && (8 == readInteger(pHandle, n + 0x40)) && (84 == readInteger(pHandle, n + 0x48)))
			{
				ReadProcessMemory(pHandle, (BYTE*)(n + 0x10), &X, sizeof(X), nullptr);
				ReadProcessMemory(pHandle, (BYTE*)(n + 0x14), &Y, sizeof(Y), nullptr);

				WriteProcessMemory(pHandle, (BYTE*)XPosAddressCar, &X, sizeof(X), 0);
				WriteProcessMemory(pHandle, (BYTE*)YPosAddressCar, &Y, sizeof(Y), 0);
				WriteProcessMemory(pHandle, (BYTE*)ZPosAddressCar, &Z, sizeof(Z), 0);

				WriteProcessMemory(pHandle, (BYTE*)XPosAddress, &X, sizeof(X), 0);
				WriteProcessMemory(pHandle, (BYTE*)YPosAddress, &Y, sizeof(Y), 0);
				WriteProcessMemory(pHandle, (FLOAT*)ZPosAddress, &Z, sizeof(Z), 0);
			}
		}

		std::cout << "Teleported" << std::endl;
	}
	//tp objective
	if (getOption() == 6)
	{
		int X, Y, Z;
		uintptr_t XPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArrayCar);
		uintptr_t YPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArrayCar);
		uintptr_t ZPosAddressCar = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArrayCar);
		uintptr_t XPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArray);
		uintptr_t YPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArray);
		uintptr_t ZPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArray);

		for (int i = 0; i < 2000; i++)
		{
			auto a = BlipPtrBaseAddr;
			auto n = readPointer(pHandle, a + (i * 8));
			if ((n > 0) && (1 == readInteger(pHandle, n + 0x40)) /*&& (5 == readInteger(pHandle, n + 0x48)) || (60 == readInteger(pHandle, n + 0x48)) || (66 == readInteger(pHandle, n + 0x48))*/)
			{
				ReadProcessMemory(pHandle, (BYTE*)(n + 0x10), &X, sizeof(X), nullptr);
				ReadProcessMemory(pHandle, (BYTE*)(n + 0x14), &Y, sizeof(Y), nullptr);
				ReadProcessMemory(pHandle, (BYTE*)(n + 0x18), &Z, sizeof(Z), nullptr);

				WriteProcessMemory(pHandle, (BYTE*)XPosAddressCar, &X, sizeof(X), 0);
				WriteProcessMemory(pHandle, (BYTE*)YPosAddressCar, &Y, sizeof(Y), 0);
				WriteProcessMemory(pHandle, (BYTE*)ZPosAddressCar, &Z, sizeof(Z), 0);

				WriteProcessMemory(pHandle, (BYTE*)XPosAddress, &X, sizeof(X), 0);
				WriteProcessMemory(pHandle, (BYTE*)YPosAddress, &Y, sizeof(Y), 0);
				WriteProcessMemory(pHandle, (FLOAT*)ZPosAddress, &Z, sizeof(Z), 0);
			}
		}

		std::cout << "Tp objective" << std::endl;
	}
	//invisible
	if (getOption() == 7)
	{
		invisible = !invisible;
		if (invisible)
		{
			int on = 66817;
			WriteProcessMemory(pHandle, (BYTE*)InvisibleAddress, &on, sizeof(on), 0);
		}
		else
		{
			int off = 66855;
			WriteProcessMemory(pHandle, (BYTE*)InvisibleAddress, &off, sizeof(off), 0);
		}
	}
	//off radar
	if (getOption() == 8)
	{
		offradar = !offradar;
		if (offradar)
		{
			//maxhealth 0
			/*std::vector<unsigned int> test = { 48 };
			DWORD64 e = FindPointer(pHandle, GlobalPTR, test);
			int c;
			int d;
			ReadProcessMemory(pHandle, (BYTE*)(e + 0x97CF0), &c, sizeof(c), nullptr);
			ReadProcessMemory(pHandle, (BYTE*)(e + 0x716B0), &d, sizeof(d), nullptr);
			int one = 1;
			int ete = 5341000;

			WriteProcessMemory(pHandle, (BYTE*)(e + 0x97F20), &d + ete, sizeof(d + ete), 0);
			WriteProcessMemory(pHandle, (BYTE*)(e + 0x7E610 + 0xCE8 * c), &one, sizeof(one), 0);
			std::cout << "e = " << std::hex << e << std::endl;*/


			//e = readPointer(getAddress("GlobalPTR+48"))
			//c = readInteger(e + 0x97CF0)
			//d = readInteger(e + 0x716B0) 
			//writeInteger(e + 0x97F20, d + 5341000)
			//writeInteger(e + 0x7E610 + 0xCE8 * c, 1)
			int nul = 0;
			MaxHealthAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.MaxHealthArray);
			WriteProcessMemory(pHandle, (BYTE*)(MaxHealthAddress), &nul, sizeof(nul), 0);
		}
		else
		{
			int nul = 2600;
			MaxHealthAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.MaxHealthArray);
			WriteProcessMemory(pHandle, (BYTE*)(MaxHealthAddress), &nul, sizeof(nul), 0);
		}
	}
	//sprint
	if (getOption() == 9)
	{
		WriteProcessMemory(pHandle, (FLOAT*)SprintSpeedAddress, &sprintspeed, sizeof(sprintspeed), 0);
		std::cout << "Sprint set" << std::endl;
	}
	//swim
	if (getOption() == 10)
	{
		WriteProcessMemory(pHandle, (FLOAT*)SwimSpeedAddress, &swimspeed, sizeof(swimspeed), 0);
		std::cout << "Swim set" << std::endl;
	}
	//rp mult
	if (getOption() == 11)
	{
		RPMult = !RPMult;
		if (RPMult)
		{
			RpMultAddress = FindPointer(pHandle, AfkPtrBaseAddr, Offset.RPMultiplierArray);
			int amount = 1120403456;
			WriteProcessMemory(pHandle, (BYTE*)RpMultAddress, &amount, sizeof(amount), 0);
		}
		else
		{
			int amount = 1065353216;
			WriteProcessMemory(pHandle, (BYTE*)RpMultAddress, &amount, sizeof(amount), 0);
		}
	}
	
	menuTitle("Self Menu");
	menuBoolOption("No AFK", AFK, 1);
	menuIntOption("Wanted level", &Wantedstars, 0, 5, 2);
	menuBoolOption("God Mode",god, 3);
	menuBoolOption("No Ragdoll", bRagdoll, 4);
	menuOption("Teleport on waypoint", 5);
	menuOption("Teleport on objective", 6);
	menuBoolOption("Invisible", invisible, 7);
	menuBoolOption("Off Radar", offradar, 8);
	menuFloatOption("Sprint/Walk Speed", &sprintspeed, 1, 50, 1, 9);
	menuFloatOption("Swim Speed", &swimspeed, 1, 50, 1, 10);
	menuBoolOption("x100 RP Mult", RPMult, 11);

	normalMenuActions();
}

void MoneyMenu() 
{
	//spawn money
	if (getOption() == 1)
	{
		float X, Y, Z;

		uintptr_t XPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArray);
		uintptr_t YPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArray);
		uintptr_t ZPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArray);

		uintptr_t MoneyValueAddressTwoThousand = ObjectSpawn - 0x86008;

		uintptr_t XPOS = ObjectSpawn - 0x85FF8;
		uintptr_t YPOS = ObjectSpawn - 0x85FF0;
		uintptr_t ZPOS = ObjectSpawn - 0x85FE8;

		uintptr_t moneySpawn = ObjectSpawn - 0x85FE0;
		uintptr_t moneySpawn2 = ObjectSpawn - 0x85FC0;

		uintptr_t moneySpawnBags = ObjectSpawn + 0x86278;

		uintptr_t moneyObjectBags = FindPointer(pHandle, ObjectSpawn, Offset.ObjectSpawnMoneyArray);

		/*=================================================================================================*/

		ReadProcessMemory(pHandle, (FLOAT*)(XPosAddress), &X, sizeof(X), 0);
		ReadProcessMemory(pHandle, (FLOAT*)(YPosAddress), &Y, sizeof(Y), 0);
		ReadProcessMemory(pHandle, (FLOAT*)(ZPosAddress), &Z, sizeof(Z), 0);
		Z += 2;

		int twothousand = 2500;
		int on = 1;
		int oon = 2;
		int bag = 2628187989;
		WriteProcessMemory(pHandle, (BYTE*)MoneyValueAddressTwoThousand, &twothousand, sizeof(twothousand), 0);

		WriteProcessMemory(pHandle, (BYTE*)XPOS, &X, sizeof(X), 0);
		WriteProcessMemory(pHandle, (BYTE*)YPOS, &Y, sizeof(Y), 0);
		WriteProcessMemory(pHandle, (BYTE*)ZPOS, &Z, sizeof(Z), 0);

		WriteProcessMemory(pHandle, (BYTE*)moneySpawn, &on, sizeof(on), 0);
		WriteProcessMemory(pHandle, (BYTE*)moneySpawn2, &on, sizeof(on), 0);
		WriteProcessMemory(pHandle, (BYTE*)moneySpawnBags, &oon, sizeof(oon), 0);
		WriteProcessMemory(pHandle, (BYTE*)moneyObjectBags, &bag, sizeof(bag), 0);
		std::cout << "Money spawned" << std::endl;
	}
	//billion
	if (getOption() == 2)
	{
		GetPrivateProfileStringA("VALUES", "BunkerMultiplier", "FAILED", (LPSTR)BunkerMultString, 255, path);
		int bunker = atoi((const char*)BunkerMultString);
		WriteProcessMemory(pHandle, (BYTE*)DeliveryAddress, &bunker, sizeof(bunker), 0);

		int DeliveryCount;
		ReadProcessMemory(pHandle, (BYTE*)DeliveryAddress, &DeliveryCount, sizeof(DeliveryCount), nullptr);
		std::cout << "Bunker Delivery count = " << std::dec << DeliveryCount << std::endl;
	}

	menuTitle("Money Menu");
	menuOption("Spawn money bag", 1);
	menuOption("Billion Dollar Bunker", 2);
	normalMenuActions();
}

void WeaponMenu()
{
	//dmg mult
	if (getOption() == 1)
	{
		WriteProcessMemory(pHandle, (BYTE*)DmgMultAddress, &DamageMult, sizeof(DamageMult), 0);
	}
	//melee dmg mult
	if (getOption() == 2)
	{
		WriteProcessMemory(pHandle, (BYTE*)MeleeDmgMultAddress, &MeleeDamageMult, sizeof(MeleeDamageMult), 0);
	}
	//no reload
	if (getOption() == 3)
	{
		noreload = !noreload;
		if (noreload)
		{
			NoReloadAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ReloadSpeedArray);
			int norld = 1148829696;
			WriteProcessMemory(pHandle, (BYTE*)NoReloadAddress, &norld, sizeof(norld), 0);
		}
		else
		{
			NoReloadAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ReloadSpeedArray);
			int norld = 1065353216;
			WriteProcessMemory(pHandle, (BYTE*)NoReloadAddress, &norld, sizeof(norld), 0);
		}
	}
	//inf ammo
	if (getOption() == 4)
	{
		infiniteammo = !infiniteammo;
		if (infiniteammo)
		{
			CurrentAmmoAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.CurrentAmmoArray);
			int ammocount = 10000;
			//ReadProcessMemory(pHandle, (BYTE*)CurrentAmmoAddress, &ammocount, sizeof(ammocount), nullptr);
			WriteProcessMemory(pHandle, (BYTE*)CurrentAmmoAddress, &ammocount, sizeof(ammocount), 0);
		}
	}

	menuTitle("Current Weapon");
	menuFloatOption("Damage Multiplier", &DamageMult, 0.7200000286, 10000, 5, 1);
	menuFloatOption("Melee Dmg Mult", &MeleeDamageMult, 1, 10000, 5, 2);
	menuBoolOption("No Reload", noreload, 3);
	menuBoolOption("Refill Ammo", infiniteammo, 4);
	submenuOption("Weapon Mods", 7, 5);
	normalMenuActions();
}

void VehicleMenu()
{
	//god car
	if (getOption() == 1)
	{
		Vehiclegod = !Vehiclegod;
		if (Vehiclegod)
		{
			std::cout << "CarGodModeAddress = " << std::hex << CarGodModeAddress << std::endl;
			CarGodModeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.CarGodModeArray);
			EngineHealth1Address = FindPointer(pHandle, WorldPtrBaseAddr, Offset.EngineHealth1Array);
			EngineHealth2Address = FindPointer(pHandle, WorldPtrBaseAddr, Offset.EngineHealth2Array);
			EngineHealth3Address = FindPointer(pHandle, WorldPtrBaseAddr, Offset.EngineHealth3Array);
			bool Godtwo = true;
			float full = 1000.0f;
			WriteProcessMemory(pHandle, (BOOL*)CarGodModeAddress, &Godtwo, sizeof(Godtwo), 0);
			WriteProcessMemory(pHandle, (FLOAT*)EngineHealth1Address, &full, sizeof(full), 0);
			WriteProcessMemory(pHandle, (FLOAT*)EngineHealth2Address, &full, sizeof(full), 0);
			WriteProcessMemory(pHandle, (FLOAT*)EngineHealth3Address, &full, sizeof(full), 0);
		}
		else
		{
			int Godtwo = 0;
			WriteProcessMemory(pHandle, (BOOL*)CarGodModeAddress, &Godtwo, sizeof(Godtwo), 0);
		}
	}
	//invisible
	if (getOption() == 2)
	{
		VehicleInvisible = !VehicleInvisible;
		if (VehicleInvisible)
		{
			CarInvisibleAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleInvisibleArray);
			int on = 66817;
			WriteProcessMemory(pHandle, (BYTE*)CarInvisibleAddress, &on, sizeof(on), 0);
		}
		else
		{
			CarInvisibleAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleInvisibleArray);
			int off = 66855;
			WriteProcessMemory(pHandle, (BYTE*)CarInvisibleAddress, &off, sizeof(off), 0);
		}
	}
	//unlim ammo veh
	if (getOption() == 3)
	{
		UnlimitedVehclAmmo = !UnlimitedVehclAmmo;
		if (UnlimitedVehclAmmo)
		{
			UnlmVehicleMisslesAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.UnlmVehicleMissleArray);
			UnlmVehicleAirBombAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.UnlmAirBombArray);
			UnlmVehicleAirCounterAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.UnlmAirCounterArray);
			UnlmVehicleOppressorMisslesAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.UnlmOppressorMkIIMisslesArray);
			UnlmVehicleTampaMisslesAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.UnlmTampaMisslesArray);

			int one = 20;
			int two = 8;
			int tree = 25;
			int four = 50;
			WriteProcessMemory(pHandle, (BYTE*)UnlmVehicleMisslesAddress, &one, sizeof(one), 0);
			WriteProcessMemory(pHandle, (BYTE*)UnlmVehicleAirBombAddress, &two, sizeof(two), 0);
			WriteProcessMemory(pHandle, (BYTE*)UnlmVehicleAirCounterAddress, &two, sizeof(two), 0);
			WriteProcessMemory(pHandle, (BYTE*)UnlmVehicleOppressorMisslesAddress, &tree, sizeof(tree), 0);
			WriteProcessMemory(pHandle, (BYTE*)UnlmVehicleTampaMisslesAddress, &four, sizeof(four), 0);
		}
	}
	//boost
	if (getOption() == 4)
	{
		BoostAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BoostArray);
		WriteProcessMemory(pHandle, (FLOAT*)BoostAddress, &Boost, sizeof(Boost), 0);
	}
	//seatbelt
	if (getOption() == 5)
	{
		Seatbelt = !Seatbelt;
		if (Seatbelt)
		{
			SeatbeltAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.SeatBeltArray);
			bool Godtwo = true;
			WriteProcessMemory(pHandle, (BOOL*)SeatbeltAddress, &Godtwo, sizeof(Godtwo), 0);
		}
		else
		{
			SeatbeltAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.SeatBeltArray);
			bool Godtwo = false;
			WriteProcessMemory(pHandle, (BOOL*)SeatbeltAddress, &Godtwo, sizeof(Godtwo), 0);
		}
	}
	//spawn car
	if (getOption() == 6)
	{
		float X, Y, Z;
		uintptr_t XPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.XPosArray);
		uintptr_t YPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.YPosArray);
		uintptr_t ZPosAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ZPosArray);

		uintptr_t CarSpawnX = Carspawn + 0xC13C8;
		uintptr_t CarSpawnY = Carspawn + 0xC13D0;
		uintptr_t CarSpawnZ = Carspawn + 0xC13D8;

		uintptr_t CarSpawnUNO = Carspawn + 0xC13A0;
		uintptr_t CarSpawnDOS = Carspawn + 0xC13B8;
		uintptr_t CarType = Carspawn + 0xC1678;

		int type = 1093792632;
		int ON = 1;

		ReadProcessMemory(pHandle, (FLOAT*)(XPosAddress), &X, sizeof(X), 0);
		ReadProcessMemory(pHandle, (FLOAT*)(YPosAddress), &Y, sizeof(Y), 0);
		ReadProcessMemory(pHandle, (FLOAT*)(ZPosAddress), &Z, sizeof(Z), 0);

		X += 5;

		WriteProcessMemory(pHandle, (FLOAT*)CarSpawnX, &X, sizeof(X), 0);
		WriteProcessMemory(pHandle, (FLOAT*)CarSpawnY, &Y, sizeof(Y), 0);
		WriteProcessMemory(pHandle, (FLOAT*)CarSpawnZ, &Z, sizeof(Z), 0);

		WriteProcessMemory(pHandle, (BYTE*)CarSpawnUNO, &ON, sizeof(ON), 0);
		WriteProcessMemory(pHandle, (BYTE*)CarSpawnDOS, &ON, sizeof(ON), 0);
		WriteProcessMemory(pHandle, (BYTE*)CarType, &type, sizeof(type), 0);

		std::cout << "Spawned" << std::endl;
	}

	menuTitle("Vehicle Menu");
	menuBoolOption("God Mode Car", Vehiclegod, 1);
	menuBoolOption("Invisibility", VehicleInvisible, 2);
	menuBoolOption("Inf Car Ammo",UnlimitedVehclAmmo, 3);
	menuFloatOption("Boost", &Boost, 1, 1000, 10, 4);
	menuBoolOption("Seatbelt", Seatbelt, 5);
	menuOption("Spawn car", 6);
	submenuOption("Vehicle Mods", 8, 7);
	normalMenuActions();
}

void MiscMenu()
{
	if (getOption() == 1)
	{
		egg = !egg;
		if (egg)
		{
			uintptr_t NeededSupplies = FindPointer(pHandle, AfkPtrBaseAddr, Offset.EggNeededResupliesArray);
			uintptr_t Timeone = FindPointer(pHandle, AfkPtrBaseAddr, Offset.EggTime1Array);
			uintptr_t Timetwo = FindPointer(pHandle, AfkPtrBaseAddr, Offset.EggTime2Array);
			uintptr_t Onlinesnow = FindPointer(pHandle, AfkPtrBaseAddr, Offset.EggOnlineSnowArray);

			int val1 = 0;
			int val3 = 24;

			WriteProcessMemory(pHandle, (BYTE*)NeededSupplies, &val1, sizeof(val1), 0);
			WriteProcessMemory(pHandle, (BYTE*)Timeone, &val1, sizeof(val1), 0);
			WriteProcessMemory(pHandle, (BYTE*)Timetwo, &val3, sizeof(val3), 0);
			WriteProcessMemory(pHandle, (BYTE*)Onlinesnow, &val1, sizeof(val1), 0);
		}
	}

	menuTitle("Misc Menu");
	menuBoolOption("Alien egg supply",egg, 1);
	normalMenuActions();
}

void WeaponMods()
{
	//bulletdmg
	if (getOption() == 1)
	{
		BulletDmgAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BulletDmgArray);
		WriteProcessMemory(pHandle, (FLOAT*)BulletDmgAddress, &bulletdmg, sizeof(bulletdmg), 0);
	}
	//bulletmass
	if (getOption() == 2)
	{
		BulletMassAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BulletMassArray);
		WriteProcessMemory(pHandle, (FLOAT*)BulletMassAddress, &bulletmass, sizeof(bulletmass), 0);
	}
	//batch
	if (getOption() == 3)
	{
		BulletInBatchAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BulletsinBatchArray);
		WriteProcessMemory(pHandle, (BYTE*)BulletInBatchAddress, &bulletbatch, sizeof(bulletbatch), 0);
	}
	//muzzlevelocity
	if (getOption() == 4)
	{
		MuzzleVelocityAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.MuzzleVelocityArray);
		WriteProcessMemory(pHandle, (FLOAT*)MuzzleVelocityAddress, &muzzlevelocity, sizeof(muzzlevelocity), 0);
	}
	//range
	if (getOption() == 5)
	{
		RangeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.WeaponRangeArray);
		WriteProcessMemory(pHandle, (FLOAT*)RangeAddress, &Range, sizeof(Range), 0);
	}
	//no recoil
	if (getOption() == 6)
	{
		Norecoil = !Norecoil;
		if (Norecoil)
		{
			int nulll = 0;
			NoRecoilAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.WeaponRecoilArray);
			WriteProcessMemory(pHandle, (FLOAT*)NoRecoilAddress, &nulll, sizeof(nulll), 0);
		}
	}
	//spread
	if (getOption() == 7)
	{
		SpreadAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BatchSpreadArray);
		WriteProcessMemory(pHandle, (FLOAT*)SpreadAddress, &Spread, sizeof(Spread), 0);
	}
	//penetration
	if (getOption() == 8)
	{
		PenetrationAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.PenetrationArray);
		WriteProcessMemory(pHandle, (FLOAT*)PenetrationAddress, &Penetration, sizeof(Penetration), 0);
	}
	//forceped
	if (getOption() == 9)
	{
		ForceOnPedAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ForceOnPedArray);
		WriteProcessMemory(pHandle, (FLOAT*)ForceOnPedAddress, &FonPed, sizeof(FonPed), 0);
	}
	//force veh
	if (getOption() == 10)
	{
		ForceOnVehAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ForceOnVehicleArray);
		WriteProcessMemory(pHandle, (FLOAT*)ForceOnVehAddress, &FonVehicle, sizeof(FonVehicle), 0);
	}
	//force heli
	if (getOption() == 11)
	{
		ForceOnHeliAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ForceOnHeliArray);
		WriteProcessMemory(pHandle, (FLOAT*)ForceOnHeliAddress, &FonHeli, sizeof(FonHeli), 0);
	}
	//instant lock
	if (getOption() == 12)
	{
		instantLock = !instantLock;
		if (instantLock)
		{
			int amount = 1036831949;
			InstantLockAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.LockOnSpeedArray);
			WriteProcessMemory(pHandle, (FLOAT*)InstantLockAddress, &amount, sizeof(amount), 0);
		}
		else
		{
			int amount = 1073741824;
			InstantLockAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.LockOnSpeedArray);
			WriteProcessMemory(pHandle, (FLOAT*)InstantLockAddress, &amount, sizeof(amount), 0);
		}
	}
	//lockonrange
	if (getOption() == 13)
	{
		LockonRangeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.LockOnRangeArray);
		WriteProcessMemory(pHandle, (FLOAT*)LockonRangeAddress, &LockonRange, sizeof(LockonRange), 0);
	}
	//instantrocket
	if (getOption() == 14)
	{
		instantRocket = !instantRocket;
		if (instantRocket)
		{
			int amount = 250000;
			InstantRocketAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.RocketSpeedArray);
			WriteProcessMemory(pHandle, (FLOAT*)InstantRocketAddress, &amount, sizeof(amount), 0);
		}
	}
	//rocketlifetime
	if (getOption() == 15)
	{
		RocketLifetimeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.RocketLifeTimeArray);
		WriteProcessMemory(pHandle, (FLOAT*)RocketLifetimeAddress, &RocketLifetime, sizeof(RocketLifetime), 0);
	}
	//homingability
	if (getOption() == 16)
	{
		HomingAbilityAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.HomingAbilityArray);
		WriteProcessMemory(pHandle, (FLOAT*)HomingAbilityAddress, &HomingAbility, sizeof(HomingAbility), 0);
	}
	//throwableGravity
	if (getOption() == 17)
	{
		ThrowableGravAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ThrowableGravityArray);
		WriteProcessMemory(pHandle, (FLOAT*)ThrowableGravAddress, &ThrowGravity, sizeof(ThrowGravity), 0);
	}
	//weaponWheelslot
	if (getOption() == 18)
	{
		WeaponWheelAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.WeaponWheelShotArray);
		WriteProcessMemory(pHandle, (BYTE*)WeaponWheelAddress, &WeaponWheelSlot, sizeof(WeaponWheelSlot), 0);
	}

	menuTitle("Weapon Mods");
	menuFloatOption("Bullet Damage", &bulletdmg, 0, 100000, 100, 1);
	menuIntOption("Bullet Mass", &bulletmass, 1000000000, 99999999999, 2);
	menuIntOption("Bullet At once", &bulletbatch, 1, 10000, 3);
	menuFloatOption("Muzzle Velocity", &muzzlevelocity, 1000, 100000, 1000, 4);
	menuFloatOption("Range", &Range, 100, 100000, 10, 5);
	menuBoolOption("No Recoil", Norecoil, 6);
	menuFloatOption("Spread", &Spread, 0, 100, 1, 7);
	menuFloatOption("Penetration", &Penetration, 0.1, 100, 1, 8);
	menuFloatOption("Force on Ped", &FonPed, 0, 10000, 100, 9);
	menuFloatOption("Force on Vehicle", &FonVehicle, 0, 10000, 100, 10);
	menuFloatOption("Force on Heli", &FonHeli, 0, 10000, 100, 11);
	menuBoolOption("Instant lock on", instantLock, 12);
	menuFloatOption("Lock on Range", &LockonRange, 0, 10000, 100, 13);
	menuBoolOption("Instant Rocket", instantRocket, 14);
	menuFloatOption("Rocket Lifetime", &RocketLifetime, 10, 1000, 1, 15);
	menuFloatOption("Homing Ability", &HomingAbility, 10, 1000, 1, 16);
	menuFloatOption("Throw Gravity", &ThrowGravity, 0, 1000, 1, 17);
	menuIntOption("WeaponWheel Slot", &WeaponWheelSlot, 0, 10, 18);
	normalMenuActions();
}

void VehicleMods()
{
	//acceleration
	if (getOption() == 1)
	{
		AccelerationAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.AccelerationArray);
		WriteProcessMemory(pHandle, (FLOAT*)AccelerationAddress, &Acceleration, sizeof(Acceleration), 0);
	}
	//brakeforce
	if (getOption() == 2)
	{
		BrakeForceAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BrakeForceArray);
		WriteProcessMemory(pHandle, (FLOAT*)BrakeForceAddress, &BrakeForce, sizeof(BrakeForce), 0);
	}
	//handbrakeforce
	if (getOption() == 3)
	{
		HandBrakeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.HandBrakeArray);
		WriteProcessMemory(pHandle, (FLOAT*)HandBrakeAddress, &HandBrakeForce, sizeof(HandBrakeForce), 0);
	}
	//deformMult
	if (getOption() == 4)
	{
		DeformMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.DeformMultArray);
		WriteProcessMemory(pHandle, (FLOAT*)DeformMultAddress, &DeformMult, sizeof(DeformMult), 0);
	}
	//trac min
	if (getOption() == 5)
	{
		TracMinAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.TractionCurveMinArray);
		WriteProcessMemory(pHandle, (FLOAT*)TracMinAddress, &TractionMin, sizeof(TractionMin), 0);
	}
	//trac max
	if (getOption() == 6)
	{
		TracMaxAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.TractionCurveMaxArray);
		WriteProcessMemory(pHandle, (FLOAT*)TracMaxAddress, &TractionMax, sizeof(TractionMax), 0);
	}
	//gravity
	if (getOption() == 7)
	{
		GravityAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.GravityArray);
		WriteProcessMemory(pHandle, (FLOAT*)GravityAddress, &Gravity, sizeof(Gravity), 0);
	}
	//vehicle mass
	if (getOption() == 8)
	{
		VehicleMaxxAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleMassArray);
		WriteProcessMemory(pHandle, (FLOAT*)VehicleMaxxAddress, &VehicleMass, sizeof(VehicleMass), 0);
	}
	//VehicleBuoyancyAddress
	if (getOption() == 9)
	{
		VehicleBuoyancyAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleBuoyancyArray);
		WriteProcessMemory(pHandle, (FLOAT*)VehicleBuoyancyAddress, &VehicleBuoyancy, sizeof(VehicleBuoyancy), 0);
	}
	//rocketFuelLVL
	if (getOption() == 10)
	{
		RocketFuelAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.RocketFuelLvlArray);
		WriteProcessMemory(pHandle, (FLOAT*)RocketFuelAddress, &RocketFuelLvl, sizeof(RocketFuelLvl), 0);
	}
	//rocketrecharge
	if (getOption() == 11)
	{
		RocketRechargeAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.RocketRechargeSpeedArray);
		WriteProcessMemory(pHandle, (FLOAT*)RocketRechargeAddress, &RocketRechargeSpeed, sizeof(RocketRechargeSpeed), 0);
	}
	//boostmax speed
	if (getOption() == 12)
	{
		BoostMaxSpeedAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.BoostMaxSpeedArray);
		WriteProcessMemory(pHandle, (FLOAT*)BoostMaxSpeedAddress, &BoostmaxSPeed, sizeof(BoostmaxSPeed), 0);
	}
	//veh dmg mult
	if (getOption() == 13)
	{
		CarDmgMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleDamageMultArray);
		WriteProcessMemory(pHandle, (FLOAT*)CarDmgMultAddress, &VehicleDamageMult, sizeof(VehicleDamageMult), 0);
	}
	//weapondmg mult
	if (getOption() == 14)
	{
		CarWeaponDmgMultAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.VehicleWeaponDmgMultArray);
		WriteProcessMemory(pHandle, (FLOAT*)CarWeaponDmgMultAddress, &WeaponVehicleDamageMult, sizeof(WeaponVehicleDamageMult), 0);
	}
	//colision dmg
	if (getOption() == 15)
	{
		CollisionDmgAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ColisionDmgMultArray);
		WriteProcessMemory(pHandle, (FLOAT*)CollisionDmgAddress, &ColisionVehicleDamageMult, sizeof(ColisionVehicleDamageMult), 0);
	}
	//thrust
	if (getOption() == 16)
	{
		ThrustAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ThrustArray);
		WriteProcessMemory(pHandle, (FLOAT*)ThrustAddress, &Thrust, sizeof(Thrust), 0);
	}
	//thrust deluxo
	if (getOption() == 17)
	{
		ThrustDeluxoAddress = FindPointer(pHandle, WorldPtrBaseAddr, Offset.ThrustDeluxoArray);
		WriteProcessMemory(pHandle, (FLOAT*)ThrustDeluxoAddress, &ThrustDeluxo, sizeof(ThrustDeluxo), 0);
	}

	menuTitle("Vehicle Mods");
	menuFloatOption("Acceleration", &Acceleration, 1, 1000, 1, 1);
	menuFloatOption("Brake Force", &BrakeForce, 0, 1000, 1, 2);
	menuFloatOption("Handbrake Force", &HandBrakeForce, 0, 1000, 1, 3);
	menuFloatOption("Deform Mult", &DeformMult, 0, 1000, 1, 4);
	menuFloatOption("Traction Min", &TractionMin, 0, 1000, 0.1f, 5);
	menuFloatOption("Traction max", &TractionMax, 0, 1000, 0.1f, 6);
	menuFloatOption("Gravity", &Gravity, 0, 1000, 0.1f, 7);
	menuFloatOption("Vehicle Mass", &VehicleMass, 0, 1000, 1, 8);
	menuFloatOption("Vehicle Buoyancy", &VehicleBuoyancy, 0, 1000, 1, 9);
	menuIntOption("Rocket Fuel lvl", &RocketFuelLvl, 0, 1000, 10);
	menuFloatOption("Rocket Recharge", &RocketRechargeSpeed, 0, 1000, 0.1f, 11);
	menuFloatOption("Boost max speed", &BoostmaxSPeed, 0, 1000, 1.0f, 12);
	menuFloatOption("Car Damage Mult", &VehicleDamageMult, 0, 1000, 1.0f, 13);
	menuFloatOption("Weapon Dmg Mult", &WeaponVehicleDamageMult, 0, 1000, 1.0f, 14);
	menuFloatOption("Collision Dmg Mult", &ColisionVehicleDamageMult, 0, 1000, 1.0f, 15);
	menuFloatOption("Thrust", &Thrust, 0, 1000, 0.1f, 16);
	menuFloatOption("Thrust Deluxo", &ThrustDeluxo, 0, 1000, 0.1f, 17);
	normalMenuActions();
}

bool firstrender = true;
int Render()
{
	if (firstrender)
	{
		LoadSettings();
		loadSettings();
		firstrender = false;
	}
	
	p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.0f, 0);
	p_Device->BeginScene();
	checkButtons();

	if (tWnd == GetForegroundWindow())
	{
		if (submenu == 1)
		{
			MainMenu();
		}
		else if (submenu == 2)
		{
			SelfMenu();
		}
		else if (submenu == 3)
		{
			MoneyMenu();
		}
		else if (submenu == 4)
		{
			MiscMenu();
		}
		else if (submenu == 5)
		{
			WeaponMenu();
		}
		else if (submenu == 6)
		{
			VehicleMenu();
		}
		else if (submenu == 7)
		{
			WeaponMods();
		}
		else if (submenu == 8)
		{
			VehicleMods();
		}

		if (menuPress == true)
		{
			Sleep(100);
		}

		menuPress = false;
		optionPress = false;
	}
	p_Device->EndScene();
	p_Device->PresentEx(0, 0, 0, 0, 0);
	return 0;
}