#include "directX.h"
#include <fstream>
#include <xstring>
#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <vector>
#include "vector.h"

LPSTR MenuOpenKeyString[255];

int currentOption;
int optionCount;

bool optionPress = false;
bool menuPress = false;

bool leftPress = false;
bool rightPress = false;

int submenu = 0;
int submenuLevel;
int lastSubmenu[20];
int lastOption[20];

int options_rect_x = 20;
int options_rect_width = 400;
int options_rect_height = 50;
//green not selected
int options_rect_r = 85;
int options_rect_g = 239;
int options_rect_b = 196;
int options_rect_a = 255;
//white text everywhere
int options_r = 223;
int options_g = 230;
int options_b = 233;
//rgba(0, 184, 148,1.0)
int title_rect_x = 20;
int title_rect_y = 10;
int title_rect_width = 400;
int title_rect_height = 50;
//Dark menu
int title_rect_r = 45;
int title_rect_g = 52;
int title_rect_b = 54;
int title_rect_a = 255;
//White text
int title_r = 223;
int title_g = 230;
int title_b = 233;
//white text
int selected_r = 0;
int selected_g = 184;
int selected_b = 148;
int selected_a = 90;

struct Offsets
{
public:
	std::vector<unsigned int> AFkArray;
	std::vector<unsigned int> WantedlvlArray;
	std::vector<unsigned int> BunkerArray;
	std::vector<unsigned int> HealthArray;
	std::vector<unsigned int> GodModeArray;
	std::vector<unsigned int> RagdollArray;
	std::vector<unsigned int> XPosArray;
	std::vector<unsigned int> YPosArray;
	std::vector<unsigned int> ZPosArray;
	std::vector<unsigned int> XPosArrayCar;
	std::vector<unsigned int> YPosArrayCar;
	std::vector<unsigned int> ZPosArrayCar;
	std::vector<unsigned int> CarGodModeArray;
	std::vector<unsigned int> ObjectSpawnMoneyArray;
	std::vector<unsigned int> EggNeededResupliesArray;
	std::vector<unsigned int> EggTime1Array;
	std::vector<unsigned int> EggTime2Array;
	std::vector<unsigned int> EggOnlineSnowArray;
	std::vector<unsigned int> InvisibleArray;
	std::vector<unsigned int> SprintSpeedArray;
	std::vector<unsigned int> SwimSpeedArray;
	std::vector<unsigned int> DmgMultArray;
	std::vector<unsigned int> BulletTypeArray;
	std::vector<unsigned int> CurrentAmmoArray;
	std::vector<unsigned int> MeleeDmgMultArray;
	std::vector<unsigned int> BulletDmgArray;
	std::vector<unsigned int> BulletMassArray;
	std::vector<unsigned int> BulletsinBatchArray;
	std::vector<unsigned int> MuzzleVelocityArray;
	std::vector<unsigned int> WeaponRangeArray;
	std::vector<unsigned int> WeaponRecoilArray;
	std::vector<unsigned int> BatchSpreadArray;
	std::vector<unsigned int> ReloadSpeedArray;
	std::vector<unsigned int> PenetrationArray;
	std::vector<unsigned int> ForceOnPedArray;
	std::vector<unsigned int> ForceOnVehicleArray;
	std::vector<unsigned int> ForceOnHeliArray;
	std::vector<unsigned int> LockOnSpeedArray;
	std::vector<unsigned int> LockOnRangeArray;
	std::vector<unsigned int> RocketSpeedArray;
	std::vector<unsigned int> RocketLifeTimeArray;
	std::vector<unsigned int> HomingAbilityArray;
	std::vector<unsigned int> ThrowableGravityArray;
	std::vector<unsigned int> WeaponWheelShotArray;
	std::vector<unsigned int> VehicleInvisibleArray;
	std::vector<unsigned int> UnlmVehicleMissleArray;
	std::vector<unsigned int> UnlmAirBombArray;
	std::vector<unsigned int> UnlmAirCounterArray;
	std::vector<unsigned int> UnlmOppressorMkIIMisslesArray;
	std::vector<unsigned int> UnlmTampaMisslesArray;
	std::vector<unsigned int> EngineHealth1Array;
	std::vector<unsigned int> EngineHealth2Array;
	std::vector<unsigned int> EngineHealth3Array;
	std::vector<unsigned int> BoostArray;
	std::vector<unsigned int> SeatBeltArray;
	std::vector<unsigned int> AccelerationArray;
	std::vector<unsigned int> BrakeForceArray;
	std::vector<unsigned int> HandBrakeArray;
	std::vector<unsigned int> DeformMultArray;
	std::vector<unsigned int> TractionCurveMinArray;
	std::vector<unsigned int> TractionCurveMaxArray;
	std::vector<unsigned int> GravityArray;
	std::vector<unsigned int> VehicleMassArray;
	std::vector<unsigned int> VehicleBuoyancyArray;
	std::vector<unsigned int> RocketFuelLvlArray;
	std::vector<unsigned int> RocketRechargeSpeedArray;
	std::vector<unsigned int> BoostMaxSpeedArray;
	std::vector<unsigned int> VehicleDamageMultArray;
	std::vector<unsigned int> VehicleWeaponDmgMultArray;
	std::vector<unsigned int> ColisionDmgMultArray;
	std::vector<unsigned int> ThrustArray;
	std::vector<unsigned int> ThrustDeluxoArray;
	std::vector<unsigned int> RPMultiplierArray;
	std::vector<unsigned int> MaxHealthArray;
	DWORD MenuOpenKey;
	DWORD MenuBackKey;
	DWORD MenuUpKey;
	DWORD MenuDownKey;
	DWORD MenuLeftKey;
	DWORD MenuRightKey;
	DWORD MenuConfirmKey;
}Offset;

void menuTitle(char *option)
{
	FillRGB(title_rect_x, title_rect_y, title_rect_width, title_rect_height, title_rect_r, title_rect_g, title_rect_b, title_rect_a);
	DrawString(option, title_rect_x + 5, title_rect_y, title_r, title_g, title_b, pFontLarge);
}

void menuOption(char *option, int id)
{
	optionCount = id;
	if (currentOption <= 10 && optionCount <= 10)
	{
		FillRGB(options_rect_x, (id * 50) + 10, options_rect_width, options_rect_height, options_rect_r, options_rect_g, options_rect_b, options_rect_a);
		DrawString(option, options_rect_x + 5, (id * 50) + 10, options_r, options_g, options_b, pFontMedium);
	}
	else if ((optionCount > (currentOption - 10)) && optionCount <= currentOption)
	{
		FillRGB(options_rect_x, ((id - (currentOption - 10)) * 50) + 10, options_rect_width, options_rect_height, options_rect_r, options_rect_g, options_rect_b, options_rect_a);
		DrawString(option, options_rect_x + 5, ((id - (currentOption - 10)) * 50) + 10, options_r, options_g, options_b, pFontMedium);
	}
}

int getOption()
{
	if (optionPress)
	{
		return currentOption;
	}
	else return 0;
}

void menuBoolOption(char* option, bool b00l, int id)
{
	char buf[30];
	if (b00l)
	{
		_snprintf_s(buf, sizeof(buf), "%s : Enabled", option);
		menuOption(buf, id);
	}
	else
	{
		_snprintf_s(buf, sizeof(buf), "%s : Disabled", option);
		menuOption(buf, id);
	}
}

void menuIntOption(char *option, int *var, int min, int max, int id)
{
	char buf[100];
	_sprintf_p(buf, sizeof(buf), "%s < %i >", option, *var);
	menuOption(buf, id);
	if (currentOption == optionCount)
	{
		if (rightPress)
		{
			if (*var >= max)
				*var = min;
			else
				*var = *var + 1;
			rightPress = false;
		}
		else if (leftPress)
		{
			if (*var <= min)
				*var = max;
			else
				*var = *var - 1;
			leftPress = false;
		}
	}
}

void menuFloatOption(char *option, float *var, int min, int max, float increment, int id)
{
	char buf[100];
	_sprintf_p(buf, sizeof(buf), "%s < %0.1f >", option, *var);
	menuOption(buf, id);
	if (currentOption == optionCount)
	{
		if (rightPress)
		{
			if (*var >= max)
				*var = min;
			else
				*var = *var + increment;
			rightPress = false;
		}
		else if (leftPress)
		{
			if (*var <= min)
				*var = max;
			else
				*var = *var - increment;
			leftPress = false;
		}
	}
}

void changeSubmenu(int newSubmenu)
{
	lastSubmenu[submenuLevel] = submenu;
	lastOption[submenuLevel] = currentOption;
	currentOption = 1;
	submenu = newSubmenu;
	submenuLevel++;
}

void submenuOption(char *option, int newSubmenu, int id)
{
	menuOption(option, id);
	if (currentOption == optionCount && optionPress)
		changeSubmenu(newSubmenu);
}

int getKeyPressed(int key)
{
	return GetAsyncKeyState(key);
}

void checkButtons()
{
	if (getKeyPressed(Offset.MenuOpenKey))
	{
		submenu = 1;
		submenuLevel = 0;
		currentOption = 1;
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuBackKey))
	{
		if (submenu == 1)
		{
			submenu = 0;
		}
		else {
			submenu = lastSubmenu[submenuLevel - 1];
			currentOption = lastOption[submenuLevel - 1];
			submenuLevel--;
		}
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuDownKey))
	{
		if (currentOption < optionCount)
		{
			currentOption++;
		}
		else if (currentOption = optionCount)
		{
			currentOption = 1;
		}
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuUpKey))
	{
		if (currentOption > 1)
		{
			currentOption--;
		}
		else if (currentOption = 1)
		{
			currentOption = optionCount;
		}
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuConfirmKey))
	{
		optionPress = true;
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuLeftKey))
	{
		leftPress = true;
		menuPress = true;
	}
	if (getKeyPressed(Offset.MenuRightKey))
	{
		rightPress = true;
		menuPress = true;
	}
}

void normalMenuActions()
{
	if (currentOption > 10)
	{
		FillRGB(options_rect_x, (10 * 50) + 10, options_rect_width, options_rect_height, selected_r, selected_g, selected_b, selected_a);
	}
	else
	{
		FillRGB(options_rect_x, (currentOption * 50) + 10, options_rect_width, options_rect_height, selected_r, selected_g, selected_b, selected_a);
	}
	if (optionCount > 10)
	{
		FillRGB(options_rect_x, (11 * 50) + 10, options_rect_width, 35, title_rect_r, title_rect_g, title_rect_b, title_rect_a);
		std::string optionCountString = std::to_string(optionCount);
		char const *optionCountStringChar = optionCountString.c_str();
		char* OptionCountChar = (char *)optionCountStringChar;
		std::string currentOptionString = std::to_string(currentOption);
		char const *currentOptionStringChar = currentOptionString.c_str();
		char* currentOptionChar = (char *)currentOptionStringChar;

		DrawString(currentOptionChar, options_rect_x + 5, ((10 + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
		DrawString("/", options_rect_x + 40, ((10 + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
		DrawString(OptionCountChar, options_rect_x + 80, ((10 + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
	}
	else
	{
		FillRGB(options_rect_x, ((optionCount + 1) * 50) + 10, options_rect_width, 35, title_rect_r, title_rect_g, title_rect_b, title_rect_a);
		std::string optionCountString = std::to_string(optionCount);
		char const *optionCountStringChar = optionCountString.c_str();
		char* OptionCountChar = (char *) optionCountStringChar;
		std::string currentOptionString = std::to_string(currentOption);
		char const *currentOptionStringChar = currentOptionString.c_str();
		char* currentOptionChar = (char *)currentOptionStringChar;
		
		DrawString(currentOptionChar, options_rect_x + 5, ((optionCount + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
		DrawString("/", options_rect_x + 40, ((optionCount + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
		DrawString(OptionCountChar, options_rect_x + 80, ((optionCount + 1) * 50) + 5, options_r, options_g, options_b, pFontMedium);
	}
}

bool ReadSettingsFile(std::wstring* Data)
{
	std::wifstream FileReader;
	if (std::wifstream(".\\Onra2.ini"))
	{
		FileReader.open(".\\Onra2.ini");
		int currentline = 0;
		while (!FileReader.eof())
		{
			std::getline(FileReader, Data[currentline]);
			currentline++;
		}
		FileReader.close();
		return true;
	}
	else
	{
		return false;
	}
}

D3DCOLOR WColor(std::wstring ColorInfo)
{
	int a, r, g, b;
	a = std::stoi(ColorInfo.substr(0, 3));
	r = std::stoi(ColorInfo.substr(3, 3));
	g = std::stoi(ColorInfo.substr(6, 3));
	b = std::stoi(ColorInfo.substr(9, 3));
	//std::cout << "a = " << a << std::endl;
	return D3DCOLOR_ARGB(a, r, g, b);
}

std::vector<unsigned int> split(std::wstring Data)
{
	std::vector<unsigned int> result;
	DWORD64 i = 0, j, n = Data.length();
	do
	{
		j = Data.find(',', i);
		if (j == std::wstring::npos)   j = n;
		result.push_back(std::stoul(Data.substr(i, j), nullptr, 16));
		i = j + 1;

	} while (j != n);
	return result;
}

std::vector<unsigned int> Pointer(std::wstring Data)
{
	std::vector<unsigned int> OffsetsArray;

	std::vector<unsigned int> arr = split(Data);
	for (unsigned int i = 0; i < arr.size(); i++)
	{
		//std::cout << arr[i] << std::endl;
		std::cout << "Reading Config" << std::endl;
	}

	OffsetsArray = arr;

	//unsigned int a, b, c;
	//a = std::stoul(Data.substr(0, 1), nullptr, 16);//0,3//255,255,255//0,1,1,4,5,3
	//b = std::stoul(Data.substr(1, 4), nullptr, 16);//3,3
	//c = std::stoul(Data.substr(5, 3), nullptr, 16);//6,3
	//OffsetsArray = { a, b, c };
	//std::cout << "a = " << std::hex << a << std::endl;
	//std::cout << "b = " << std::hex << b << std::endl;
	//std::cout << "c = " << std::hex << c << std::endl;
	return OffsetsArray;
}

bool ParseSettings(std::wstring* Data)
{
	try
	{
		char CharsToDelete[] = "() ";//,
		for (int a = 1; a < 200; a++)
		{
			for (int b = 0; b < strlen(CharsToDelete); ++b)
			{
				Data[a].erase(std::remove(Data[a].begin(), Data[a].end(), CharsToDelete[b]), Data[a].end());
			}
			Data[a] = Data[a].substr(Data[a].find(L"=") + 1);
		}
		//offsets
		Offset.AFkArray = Pointer(Data[1].c_str());
		Offset.WantedlvlArray = Pointer(Data[2].c_str());
		Offset.BunkerArray = Pointer(Data[3].c_str());
		Offset.HealthArray = Pointer(Data[4].c_str());
		Offset.GodModeArray = Pointer(Data[5].c_str());
		Offset.RagdollArray = Pointer(Data[6].c_str());
		Offset.XPosArray = Pointer(Data[7].c_str());
		Offset.YPosArray = Pointer(Data[8].c_str());
		Offset.ZPosArray = Pointer(Data[9].c_str());
		Offset.XPosArrayCar = Pointer(Data[10].c_str());
		Offset.YPosArrayCar = Pointer(Data[11].c_str());
		Offset.ZPosArrayCar = Pointer(Data[12].c_str());
		Offset.CarGodModeArray = Pointer(Data[13].c_str());
		Offset.ObjectSpawnMoneyArray = Pointer(Data[14].c_str());
		Offset.EggNeededResupliesArray = Pointer(Data[15].c_str());
		Offset.EggTime1Array = Pointer(Data[16].c_str());
		Offset.EggTime2Array = Pointer(Data[17].c_str());
		Offset.EggOnlineSnowArray = Pointer(Data[18].c_str());
		Offset.InvisibleArray = Pointer(Data[20].c_str());
		Offset.SprintSpeedArray = Pointer(Data[21].c_str());
		Offset.SwimSpeedArray = Pointer(Data[22].c_str());
		//aimbot here
		Offset.CurrentAmmoArray = Pointer(Data[24].c_str());
		Offset.BulletTypeArray = Pointer(Data[25].c_str());
		Offset.DmgMultArray = Pointer(Data[26].c_str());
		Offset.MeleeDmgMultArray = Pointer(Data[27].c_str());
		Offset.BulletDmgArray = Pointer(Data[28].c_str());
		Offset.BulletMassArray = Pointer(Data[29].c_str());
		Offset.BulletsinBatchArray = Pointer(Data[30].c_str());
		Offset.MuzzleVelocityArray = Pointer(Data[31].c_str());
		Offset.WeaponRangeArray = Pointer(Data[32].c_str());
		Offset.WeaponRecoilArray = Pointer(Data[33].c_str());
		Offset.BatchSpreadArray = Pointer(Data[34].c_str());
		Offset.ReloadSpeedArray = Pointer(Data[35].c_str());
		Offset.PenetrationArray = Pointer(Data[36].c_str());
		Offset.ForceOnPedArray = Pointer(Data[37].c_str());
		Offset.ForceOnVehicleArray = Pointer(Data[38].c_str());
		Offset.ForceOnHeliArray = Pointer(Data[39].c_str());
		Offset.LockOnSpeedArray = Pointer(Data[40].c_str());
		Offset.LockOnRangeArray = Pointer(Data[41].c_str());
		Offset.RocketSpeedArray = Pointer(Data[42].c_str());
		Offset.RocketLifeTimeArray = Pointer(Data[43].c_str());
		Offset.HomingAbilityArray = Pointer(Data[44].c_str());
		Offset.ThrowableGravityArray = Pointer(Data[45].c_str());
		Offset.WeaponWheelShotArray = Pointer(Data[46].c_str());
		Offset.VehicleInvisibleArray = Pointer(Data[47].c_str());
		Offset.UnlmVehicleMissleArray = Pointer(Data[48].c_str());
		Offset.UnlmAirBombArray = Pointer(Data[49].c_str());
		Offset.UnlmAirCounterArray = Pointer(Data[50].c_str());
		Offset.UnlmOppressorMkIIMisslesArray = Pointer(Data[51].c_str());
		Offset.UnlmTampaMisslesArray = Pointer(Data[52].c_str());
		Offset.EngineHealth1Array = Pointer(Data[53].c_str());
		Offset.EngineHealth2Array = Pointer(Data[54].c_str());
		Offset.EngineHealth3Array = Pointer(Data[55].c_str());
		Offset.BoostArray = Pointer(Data[56].c_str());
		Offset.SeatBeltArray = Pointer(Data[57].c_str());
		Offset.AccelerationArray = Pointer(Data[58].c_str());
		Offset.BrakeForceArray = Pointer(Data[59].c_str());
		Offset.HandBrakeArray = Pointer(Data[60].c_str());
		Offset.DeformMultArray = Pointer(Data[61].c_str());
		Offset.TractionCurveMinArray = Pointer(Data[62].c_str());
		Offset.TractionCurveMaxArray = Pointer(Data[63].c_str());
		Offset.GravityArray = Pointer(Data[64].c_str());
		Offset.VehicleMassArray = Pointer(Data[65].c_str());
		Offset.VehicleBuoyancyArray = Pointer(Data[66].c_str());
		Offset.RocketFuelLvlArray = Pointer(Data[67].c_str());
		Offset.RocketRechargeSpeedArray = Pointer(Data[68].c_str());
		Offset.BoostMaxSpeedArray = Pointer(Data[69].c_str());
		Offset.VehicleDamageMultArray = Pointer(Data[70].c_str());
		Offset.VehicleWeaponDmgMultArray = Pointer(Data[71].c_str());
		Offset.ColisionDmgMultArray = Pointer(Data[72].c_str());
		Offset.ThrustArray = Pointer(Data[73].c_str());
		Offset.ThrustDeluxoArray = Pointer(Data[74].c_str());
		Offset.RPMultiplierArray = Pointer(Data[75].c_str());
		Offset.MaxHealthArray = Pointer(Data[76].c_str());
		/*==================================================*/
		Offset.MenuOpenKey = wcstol(Data[78].c_str(), 0, 0);
		Offset.MenuBackKey = wcstol(Data[79].c_str(), 0, 0);
		Offset.MenuUpKey = wcstol(Data[80].c_str(), 0, 0);
		Offset.MenuDownKey = wcstol(Data[81].c_str(), 0, 0);
		Offset.MenuLeftKey = wcstol(Data[82].c_str(), 0, 0);
		Offset.MenuRightKey = wcstol(Data[83].c_str(), 0, 0);
		Offset.MenuConfirmKey = wcstol(Data[84].c_str(), 0, 0);
		//Offset.EnemyBoxColor = WColor(Data[19].c_str());
		return true;
	}
	catch (int err)
	{
		MessageBox(0, L"Settings file is not properly formatted!", L"Error", MB_OK | MB_ICONERROR);
	}
	return false;
}

bool LoadSettings()
{
	std::wstring Data[200];
	if (ReadSettingsFile(Data))
	{
		if (ParseSettings(Data))
		{
			return true;
		}
	}
	return false;
}
