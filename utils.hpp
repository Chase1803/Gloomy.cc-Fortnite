/*
	*Gloomy.cc
 	*https://github.com/Chase1803
	
	Copyright (c) 2022 Chase1803
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>

int 
g_width,
g_height,
g_pid
;

uint64_t 
g_base_address,
pattern_uworld,
pattern_gnames
;

HWND fortnite_wnd;

const char* LEntityNames[] =
{
"FortPickupAthena",
"Tiered_Chest",
"Vehicl",
"Valet_Taxi",
"Tiered_Ammo",
"Valet_BigRig",
"Valet_BasicTr",
"Valet_SportsC",
"Valet_BasicC",
};

Sandy64 Drive;

uintptr_t getBaseAddress(uintptr_t pid) {
	return Drive.GetModuleBase(pid, XorStr("FortniteClient-Win64-Shipping.exe").c_str());
}

template<typename T>
inline bool read_array(uintptr_t pid, uintptr_t address, T* array, size_t len) {
	return Drive.ReadPtr(pid, address, array, sizeof(T) * len);
}

template <typename T> T read(uintptr_t pid, uintptr_t address) {
	T t{};
	Drive.ReadPtr(pid, address, &t, sizeof(T));
	return t;
}

template<typename T>
inline bool write(int pid, uint64_t address, const T& value) {
	return Drive.WritePtr(pid, address, (PVOID)&value, sizeof(T));
}

std::string read_ascii(uintptr_t pid, const std::uintptr_t address, std::size_t size)
{
	std::unique_ptr<char[]> buffer(new char[size]);
	Drive.ReadPtr(pid, address, buffer.get(), size);
	return std::string(buffer.get());
}

std::wstring read_unicode(uintptr_t pid, const std::uintptr_t address, std::size_t size)
{
	const auto buffer = std::make_unique<wchar_t[]>(size);
	Drive.ReadPtr(pid, address, buffer.get(), size * 2);
	return std::wstring(buffer.get());
}

std::wstring read_wstr(uintptr_t pid, uintptr_t address)
{
	wchar_t buffer[1024 * sizeof(wchar_t)];
	Drive.ReadPtr(pid, address, &buffer, 64 * sizeof(wchar_t));
	return std::wstring(buffer);
}

wchar_t* readString(uintptr_t pid, DWORD64 Address, wchar_t* string)
{
	if (read<uint16_t>(pid, Address + 0x10) > 0)
	{
		for (int i = 0; i < read<uint16_t>(pid, Address + 0x10) * 2; i++)
		{
			string[i] = read<wchar_t>(pid, Address + 0x14 + (i * 2));
		}
	}
	return string;
}

namespace wndhide
{
	inline uintptr_t get_module_base(uint32_t process_id, const wchar_t* module_name)
	{
		uintptr_t base_address = 0;
		const HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
		if (snapshot_handle != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32 module_entry;
			module_entry.dwSize = sizeof(module_entry);
			if (Module32First(snapshot_handle, &module_entry))
			{
				do
				{
					if (!_wcsicmp(module_entry.szModule, module_name))
					{
						base_address = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
						break;
					}
				} while (Module32Next(snapshot_handle, &module_entry));
			}
		}
		CloseHandle(snapshot_handle);
		return base_address;
	}

	inline uintptr_t get_module_export(HANDLE process_handle, uintptr_t module_base, const char* export_name)
	{
		SIZE_T dummy_read_size;
		IMAGE_DOS_HEADER dos_header = { 0 };
		IMAGE_NT_HEADERS64 nt_headers = { 0 };

		if (!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base), &dos_header, sizeof(dos_header), &dummy_read_size) || dos_header.e_magic != IMAGE_DOS_SIGNATURE ||
			!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base + dos_header.e_lfanew), &nt_headers, sizeof(nt_headers), &dummy_read_size) || nt_headers.Signature != IMAGE_NT_SIGNATURE)
			return 0;

		const auto export_base = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		const auto export_base_size = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		if (!export_base || !export_base_size)
			return 0;

		const auto export_data = static_cast<PIMAGE_EXPORT_DIRECTORY>(VirtualAlloc(nullptr, export_base_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!export_data)
			return 0;

		if (!ReadProcessMemory(process_handle, reinterpret_cast<void*>(module_base + export_base), export_data, export_base_size, &dummy_read_size))
		{
			VirtualFree(export_data, 0, MEM_RELEASE);
			return 0;
		}

		const auto delta = reinterpret_cast<uint64_t>(export_data) - export_base;

		const auto name_table = reinterpret_cast<uint32_t*>(export_data->AddressOfNames + delta);
		const auto ordinal_table = reinterpret_cast<uint16_t*>(export_data->AddressOfNameOrdinals + delta);
		const auto function_table = reinterpret_cast<uint32_t*>(export_data->AddressOfFunctions + delta);

		for (auto i = 0u; i < export_data->NumberOfNames; ++i)
		{
			const std::string current_function_name = std::string(reinterpret_cast<char*>(name_table[i] + delta));

			if (!_stricmp(current_function_name.c_str(), export_name))
			{
				const auto function_ordinal = ordinal_table[i];
				if (function_table[function_ordinal] <= 0x1000)
					return 0;

				const auto function_address = module_base + function_table[function_ordinal];

				if (function_address >= module_base + export_base && function_address <= module_base + export_base + export_base_size)
				{
					// Forwarded export
					VirtualFree(export_data, 0, MEM_RELEASE);
					return 0;
				}

				VirtualFree(export_data, 0, MEM_RELEASE);
				return function_address;
			}
		}

		VirtualFree(export_data, 0, MEM_RELEASE);
		return 0;
	}

	inline bool hide_window(uint32_t process_id, HWND window_id, bool hide = true)
	{
		const HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
		if (!process_handle || process_handle == INVALID_HANDLE_VALUE)
			return false;

		const uintptr_t user32_base = get_module_base(process_id, L"user32.dll");
		if (!user32_base)
			return false;

		const uintptr_t function_pointer = get_module_export(process_handle, user32_base, "SetWindowDisplayAffinity");
		if (!function_pointer)
			return false;

		unsigned char shellcode_buffer[] = "\x48\x89\x4C\x24\x08\x48\x89\x54\x24\x10\x4C\x89\x44\x24\x18\x4C\x89\x4C\x24"
			"\x20\x48\x83\xEC\x38\x48\xB9\xED\xFE\xAD\xDE\xED\xFE\x00\x00\x48\xC7\xC2\xAD"
			"\xDE\x00\x00\x48\xB8\xAD\xDE\xED\xFE\xAD\xDE\x00\x00\xFF\xD0\x48\x83\xC4\x38"
			"\x48\x8B\x4C\x24\x08\x48\x8B\x54\x24\x10\x4C\x8B\x44\x24\x18\x4C\x8B\x4C\x24"
			"\x20\xC3";

		const uint32_t mask = hide ? 0x00000011 : 0x00000000; // CWDA_EXCLUDEFROMCAPTURE : CWDA_NONE

		*reinterpret_cast<uintptr_t*>(shellcode_buffer + 26) = reinterpret_cast<uintptr_t>(window_id); // window hwnd
		*reinterpret_cast<uint32_t*>(shellcode_buffer + 37) = mask; // flags
		*reinterpret_cast<uintptr_t*>(shellcode_buffer + 43) = function_pointer; // function ptr

		void* allocated_base = VirtualAllocEx(process_handle, 0x0, sizeof(shellcode_buffer), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (!allocated_base)
			return false;

		SIZE_T dummy_size;
		if (!WriteProcessMemory(process_handle, allocated_base, shellcode_buffer, sizeof(shellcode_buffer), &dummy_size))
			return false;

		HANDLE thread_handle = CreateRemoteThread(process_handle, nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(allocated_base), nullptr, 0, nullptr);
		if (!thread_handle || thread_handle == INVALID_HANDLE_VALUE)
			return false;

		// wait for shellcode to execute
		Sleep(1000);

		memset(shellcode_buffer, 0, sizeof(shellcode_buffer));
		if (!WriteProcessMemory(process_handle, allocated_base, shellcode_buffer, sizeof(shellcode_buffer), &dummy_size))
			return false;

		return true;
	}
}

#define M_PI 3.14159265358979323846264338327950288419716939937510

typedef struct
{
	DWORD R;
	DWORD G;
	DWORD B;
	DWORD A;
} RGBA;
int acount = 500;

class Vector2
{
public:
	double X;
	double Y;
};

class FRotator
{
public:
	double Pitch;
	double Yaw;
	double Roll;
};


static class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	double x;
	double y;
	double z;

	inline double Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vector3 operator/(double v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	inline double Distance(Vector3 v)
	{
		return double(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	inline double Length() {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(double flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }
};



struct FPlane : Vector3
{
	double W = 0;


	Vector3 ToVector3()
	{
		Vector3 value;
		value.x = this->x;
		value.y = this->y;
		value.z = this->y;

		return value;
	}
};

struct FMatrix
{
	FPlane XPlane;
	FPlane YPlane;
	FPlane ZPlane;
	FPlane WPlane;
};

struct FTransform
{
	FPlane  rot;
	Vector3 translation;
	char    pad[8];
	Vector3 scale;

	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		double x2 = rot.x + rot.x;
		double y2 = rot.y + rot.y;
		double z2 = rot.z + rot.z;

		double xx2 = rot.x * x2;
		double yy2 = rot.y * y2;
		double zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		double yz2 = rot.y * z2;
		double wx2 = rot.W * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		double xy2 = rot.x * y2;
		double wz2 = rot.W * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		double xz2 = rot.x * z2;
		double wy2 = rot.W * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};
static D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}
static D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0)) {
	float radPitch = (rot.x * double(M_PI) / 180.f);
	float radYaw = (rot.y * double(M_PI) / 180.f);
	float radRoll = (rot.z * double(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

namespace Globals {
	uintptr_t LocalPlayer = 0, LocalPawn = 0, LocalPawnRootComponent = 0;
	int selectedbone = 0, hitbox = 0;


	bool menuIsOpen = false;

	Vector3 LocalPlayerRelativeLocation = Vector3(0.0f, 0.0f, 0.0f);

	float Width = GetSystemMetrics(SM_CXSCREEN), Height = GetSystemMetrics(SM_CYSCREEN), ScreenCenterX = 0.0f, ScreenCenterY = 0.0f;
}

RGBA ESPColor = { 255, 255, 255, 255 };
RGBA MenuColor = { 255, 165, 0, 255 };
RGBA ESPColor2 = { 255, 215, 0, 255 };

#include "decryption.asm"

namespace camera
{
	float m_FovAngle;
	Vector3 m_CameraRotation, m_CameraLocation;
}

namespace g_functions
{
	Vector3 ConvertWorld2Screen(Vector3 WorldLocation) {

		camera::m_CameraRotation.z = 0;

		if (camera::m_CameraRotation.y < 0)
			camera::m_CameraRotation.y += 360;

		D3DMATRIX matrix = Matrix(camera::m_CameraRotation);

		auto dist = WorldLocation - camera::m_CameraLocation;

		auto transform = Vector3(
			dist.Dot(Vector3(
				matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]
			)),
			dist.Dot(Vector3(
				matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]
			)),
			dist.Dot(Vector3(
				matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]
			))
		);

		if (transform.z < 1.f)
			transform.z = 1.f;


		float ScreenCenterX = Globals::Width / 2;
		float ScreenCenterY = Globals::Height / 2;

		Vector3 Screenlocation;
		Screenlocation.x = ScreenCenterX + transform.x * (ScreenCenterX / tanf(camera::m_FovAngle * (float)M_PI / 360.f)) / transform.z;
		Screenlocation.y = ScreenCenterY - transform.y * (ScreenCenterX / tanf(camera::m_FovAngle * (float)M_PI / 360.f)) / transform.z;

		return Screenlocation;
	}

	FTransform f_boneIndex(uint64_t mesh, int index)
	{
		uint64_t bonearray = read<uint64_t>(g_pid, mesh + 0x590);
		if (!bonearray) 
		bonearray = read<DWORD_PTR>(g_pid, mesh + 0x5a0);
	
		return read<FTransform>(g_pid, bonearray + (index * 0x60));
	}

	Vector3 f_getbonewithIndex(DWORD_PTR mesh, int id)
	{
		FTransform bone = f_boneIndex(mesh, id);
		FTransform ComponentToWorld = read<FTransform>(g_pid, mesh + 0x240);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}


}

class Color
{
public:
	RGBA red = { 255,0,0,255 };
	RGBA Magenta = { 255,0,255,255 };
	RGBA yellow = { 255,255,0,255 };
	RGBA grayblue = { 128,128,255,255 };
	RGBA green = { 128,224,0,255 };
	RGBA darkgreen = { 0,224,128,255 };
	RGBA brown = { 192,96,0,255 };
	RGBA pink = { 255,168,255,255 };
	RGBA DarkYellow = { 216,216,0,255 };
	RGBA SilverWhite = { 236,236,236,255 };
	RGBA purple = { 144,0,255,255 };
	RGBA Navy = { 88,48,224,255 };
	RGBA skyblue = { 0,136,255,255 };
	RGBA graygreen = { 128,160,128,255 };
	RGBA blue = { 0,96,192,255 };
	RGBA orange = { 255,128,0,255 };
	RGBA peachred = { 255,80,128,255 };
	RGBA reds = { 255,128,192,255 };
	RGBA darkgray = { 96,96,96,255 };
	RGBA Navys = { 0,0,128,255 };
	RGBA darkgreens = { 0,128,0,255 };
	RGBA darkblue = { 0,128,128,255 };
	RGBA redbrown = { 128,0,0,255 };
	RGBA purplered = { 128,0,128,255 };
	RGBA greens = { 0,255,0,255 };
	RGBA envy = { 0,255,255,255 };
	RGBA black = { 0,0,0,255 };
	RGBA gray = { 128,128,128,255 };
	RGBA white = { 255,255,255,255 };
	RGBA blues = { 30,144,255,255 };
	RGBA lightblue = { 135,206,250,160 };
	RGBA Scarlet = { 220, 20, 60, 160 };
	RGBA white_ = { 255,255,255,200 };
	RGBA gray_ = { 128,128,128,200 };
	RGBA black_ = { 0,0,0,200 };
	RGBA red_ = { 255,0,0,200 };
	RGBA Magenta_ = { 255,0,255,200 };
	RGBA yellow_ = { 255,255,0,200 };
	RGBA grayblue_ = { 128,128,255,200 };
	RGBA green_ = { 128,224,0,200 };
	RGBA darkgreen_ = { 0,224,128,200 };
	RGBA brown_ = { 192,96,0,200 };
	RGBA pink_ = { 255,168,255,200 };
	RGBA darkyellow_ = { 216,216,0,200 };
	RGBA silverwhite_ = { 236,236,236,200 };
	RGBA purple_ = { 144,0,255,200 };
	RGBA Blue_ = { 88,48,224,200 };
	RGBA skyblue_ = { 0,136,255,200 };
	RGBA graygreen_ = { 128,160,128,200 };
	RGBA blue_ = { 0,96,192,200 };
	RGBA orange_ = { 255,128,0,200 };
	RGBA pinks_ = { 255,80,128,200 };
	RGBA Fuhong_ = { 255,128,192,200 };
	RGBA darkgray_ = { 96,96,96,200 };
	RGBA Navy_ = { 0,0,128,200 };
	RGBA darkgreens_ = { 0,128,0,200 };
	RGBA darkblue_ = { 0,128,128,200 };
	RGBA redbrown_ = { 128,0,0,200 };
	RGBA purplered_ = { 128,0,128,200 };
	RGBA greens_ = { 0,255,0,200 };
	RGBA envy_ = { 0,255,255,200 };

	RGBA glassblack = { 0, 0, 0, 160 };
	RGBA GlassBlue = { 65,105,225,80 };
	RGBA glassyellow = { 255,255,0,160 };
	RGBA glass = { 200,200,200,60 };

	RGBA filled = { 0, 0, 0, 150 };

	RGBA Plum = { 221,160,221,160 };

};
Color Col;


static const char* Hitbox[] =
{
	"Head",
	"Neck",
	"Chest"
};

int select_hitbox()
{
	int hitbox = 0;

	if (Globals::hitbox == 0)
		hitbox = 66;
	else if (Globals::hitbox == 2)
		hitbox = 65;
	else if (hitbox == 1)
		hitbox = 7;
	return hitbox;
}

namespace hotkeys
{
	int aimkey;
	int airstuckey;
	int instares;
}

static int keystatus = 0;
static int realkey = 0;

bool GetKey(int key)
{
	realkey = key;
	return true;
}
void ChangeKey(void* blank)
{
	keystatus = 1;
	while (true)
	{
		for (int i = 0; i < 0x87; i++)
		{
			if (GetKeyState(i) & 0x8000)
			{
				hotkeys::aimkey = i;
				keystatus = 0;
				return;
			}
		}
	}
}




static const char* keyNames[] =
{
	"Keybind",
	"Left Mouse",
	"Right Mouse",
	"Cancel",
	"Middle Mouse",
	"Mouse 5",
	"Mouse 4",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
};

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}
void HotkeyButton(int aimkey, void* changekey, int status)
{
	const char* preview_value = NULL;
	if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
		Items_ArrayGetter(keyNames, aimkey, &preview_value);

	std::string aimkeys;
	if (preview_value == NULL)
		aimkeys = "Select Key";
	else
		aimkeys = preview_value;

	if (status == 1)
	{

		aimkeys = "Press the Key";
	}
	if (ImGui::Button(aimkeys.c_str(), ImVec2(125, 20)))
	{
		if (status == 0)
		{
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
		}
	}
}

std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}
void DrawString(float fontSize, int x, int y, RGBA* color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), text.c_str());
}

void DrawFilledRect(int x, int y, int w, int h, RGBA* color)
{
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), 0, 0);
}

void DrawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color, RGBA* color2)
{
	DrawFilledRect(x + borderPx, y, w / 3, borderPx, color); //top 
	DrawFilledRect(x + w - w / 3 + borderPx, y, w / 3, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h / 3, color); //left 
	DrawFilledRect(x, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color2); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w / 3, borderPx, color); //bottom 
	DrawFilledRect(x + w - w / 3 + borderPx, y + h + borderPx, w / 3, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h / 3, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h / 3 + borderPx * 2, borderPx, h / 3, color2);//right 
}
void DrawCircleFilled(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}
void DrawNormalBox(int x, int y, int w, int h, int borderPx, RGBA* color, RGBA* color2)
{
	DrawFilledRect(x + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x + w - w + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h, color); //left 
	DrawFilledRect(x, y + h - h + borderPx * 2, borderPx, h, color2); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w - w + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h + borderPx * 2, borderPx, h, color2);//right 
}
void DrawCircle(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), segments);
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness)
{
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), thickness);
}



float bA1mb0tF0VV4lue = 150.0f, bA1mb0tSm00th1ngV4lue = 2.0f;

int bD1st4nce, bH1tb0x = 0, bE5pD1st4nce = 280, bA1mD1st4nce = 280, bB0xS1ze = 2.0f, bAut0L0ckD1st = 1, bLootRendering = 40;
static  float FOVChangerValue = 100.0f;
float boatmulti = 1.0f;
float boatspeed = 1.0f;


Vector3 AimbotCorrection(float bulletVelocity, float bulletGravity, float targetDistance, Vector3 targetPosition, Vector3 targetVelocity) {
	Vector3 recalculated = targetPosition;
	float gravity = fabs(bulletGravity);
	float time = targetDistance / fabs(bulletVelocity);
	/* Bullet drop correction */
	//float bulletDrop = (gravity / 250) * time * time;
	//recalculated.z += bulletDrop * 120;
	/* Player movement correction */
	//recalculated.x += time * (targetVelocity.x);
	//recalculated.y += time * (targetVelocity.y);
	//recalculated.z += time * (targetVelocity.z);
	return recalculated;
}

float powf_(float _X, float _Y) {
	return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
}
float sqrtf_(float _X) {
	return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
}

double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

double GetDistance(double x1, double y1, double z1, double x2, double y2) {
	//return sqrtf(powf_((x2 - x1), 2) + powf_((y2 - y1), 2));
	return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
}
std::string string_To_UTF8(const std::string& str)
{
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t* pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

void DrawText1(int x, int y, const char* str, RGBA* color)
{
	ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), utf_8_2.c_str());
}

void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {
	float lineW = (W / 3);
	float lineH = (H / 3);

	//corners
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

static int Depth;

static class CKey {
private:
	BYTE bPrevState[0x100];
public:
	CKey() {
		memset(bPrevState, 0, sizeof(bPrevState));
	}

	BOOL IsKeyPushing(BYTE vKey) {
		return GetAsyncKeyState(vKey) & 0x8000;
	}

	BOOL IsKeyPushed(BYTE vKey) {
		BOOL bReturn = FALSE;

		if (IsKeyPushing(vKey))
			bPrevState[vKey] = TRUE;
		else
		{
			if (bPrevState[vKey] == TRUE)
				bReturn = TRUE;
			bPrevState[vKey] = FALSE;
		}

		return bReturn;
	}
};

static CKey Key;
static bool isaimbotting;
bool isVis;

char* wchar_to_char(const wchar_t* pwchar)
{
	int currentCharIndex = 0;
	char currentChar = pwchar[currentCharIndex];

	while (currentChar != '\0')
	{
		currentCharIndex++;
		currentChar = pwchar[currentCharIndex];
	}

	const int charCount = currentCharIndex + 1;

	char* filePathC = (char*)malloc(sizeof(char) * charCount);

	for (int i = 0; i < charCount; i++)
	{
		char character = pwchar[i];

		*filePathC = character;

		filePathC += sizeof(char);

	}
	filePathC += '\0';

	filePathC -= (sizeof(char) * charCount);

	return filePathC;
}

void DrawLString(float fontSize, int x, int y, ImU32 color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 2;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), color, text.c_str());
}

Vector3 calc_angle(Vector3& zaz, Vector3& daz) {
	Vector3 dalte = zaz - daz;
	Vector3 ongle;
	double hpm = sqrt(dalte.x * dalte.x + dalte.y * dalte.y);
	ongle.y = atan(dalte.y / dalte.x) * 57.295779513082;
	ongle.x = (atan(dalte.z / hpm) * 57.295779513082) * -1;
	if (dalte.x >= 0.f) ongle.y += 180;
	return ongle;
}


#include <array>

std::uintptr_t find_signature(const char* sig, const char* mask)
{
	auto buffer = std::make_unique<std::array<std::uint8_t, 0x100000>>();
	auto data = buffer.get()->data();

	for (std::uintptr_t i = 0u; i < (2u << 25u); ++i)
	{
		Drive.ReadPtr(g_pid, g_base_address + i * 0x100000, data, 0x100000);

		if (!data)
			return 0;

		for (std::uintptr_t j = 0; j < 0x100000u; ++j)
		{
			if ([](std::uint8_t const* data, std::uint8_t const* sig, char const* mask)
				{
					for (; *mask; ++mask, ++data, ++sig)
					{
						if (*mask == 'x' && *data != *sig) return false;
					}
					return (*mask) == 0;
				}(data + j, (std::uint8_t*)sig, mask))
			{
				std::uintptr_t result = g_base_address + i * 0x100000 + j;
				std::uint32_t rel = 0;

				Drive.ReadPtr(g_pid, result + 3, &rel, sizeof(std::uint32_t));

				if (!rel)
					return 0;

				return result + rel + 7;
			}
		}
	}

	return 0;
}

template<class T>
T PatternScan(const char* signature, int offset) {
    int instructionLength = offset + sizeof(T);
    IMAGE_DOS_HEADER dos_header = read<IMAGE_DOS_HEADER>(g_pid, g_base_address);
    IMAGE_NT_HEADERS64 nt_headers = read<IMAGE_NT_HEADERS64>(g_pid, g_base_address + dos_header.e_lfanew);

    const size_t target_len = nt_headers.OptionalHeader.SizeOfImage;

    static auto patternToByte = [](const char* pattern)
    {
        auto bytes = std::vector<int>{};
        const auto start = const_cast<char*>(pattern);
        const auto end = const_cast<char*>(pattern) + strlen(pattern);

        for(auto current = start; current < end; ++current)
        {
            if(*current == '?')
            {
                ++current;
                bytes.push_back(-1);
            }
            else { bytes.push_back(strtoul(current, &current, 16)); }
        }
        return bytes;
    };

    auto patternBytes = patternToByte(signature);
    const auto s = patternBytes.size();
    const auto d = patternBytes.data();

    auto target = std::unique_ptr<uint8_t[]>(new uint8_t[target_len]);
    if(read_array(g_pid, g_base_address, target.get(), target_len)) {
        for(auto i = 0ul; i < nt_headers.OptionalHeader.SizeOfImage - s; ++i)
        {
            bool found = true;
            for(auto j = 0ul; j < s; ++j)
            {
                if(target[static_cast<size_t>(i) + j] != d[j] && d[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if(found) {
                return read<T>(g_pid, g_base_address + i + offset) + i + instructionLength;
            }
        }
    }

    return NULL;
}

typedef struct Playertest
{
	uint64_t ACurrentActor,
        USkeletalMeshComponent;
	DWORD_PTR USceneComponent;
	std::string GNames;
}Playertest;
std::vector<Playertest> PLIST;

typedef struct _LootEntity {
	std::string GNames;
	uintptr_t ACurrentItem;
}LootEntity;
static std::vector<LootEntity> LootentityList;


float closestDistance = FLT_MAX;
DWORD_PTR closestPawn = NULL;
int LocalTeam;
static bool targetlocked = false;

uintptr_t 
GWorld,
LocalPlayerController,
MyHUD
;
uint64_t PlayerCameraManager;
bool InLobby = false;

void menucolors()
{

	ImGuiStyle& s = ImGui::GetStyle();

	ImGui::StyleColorsDark();

	const ImColor bgSecondary = ImColor(15, 15, 15, 255);
	s.Colors[ImGuiCol_WindowBg] = ImColor(32, 32, 32, 255);
	s.Colors[ImGuiCol_ChildBg] = bgSecondary;
	s.Colors[ImGuiCol_FrameBg] = ImColor(65, 64, 64, 255);
	s.Colors[ImGuiCol_FrameBgActive] = ImColor(35, 37, 39, 255);
	s.Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 255);
	s.Colors[ImGuiCol_CheckMark] = ImColor(255, 0, 0, 255);
	s.Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255, 150);
	s.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255, 255);
	s.Colors[ImGuiCol_ResizeGrip] = ImColor(24, 24, 24, 255);
	s.Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 255);
	s.Colors[ImGuiCol_HeaderHovered] = ImColor(0, 0, 0, 255);
	s.Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 255);
	s.Colors[ImGuiCol_TitleBg] = ImColor(247, 255, 25, 255);
	s.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(247, 255, 25, 255);
	s.Colors[ImGuiCol_TitleBgActive] = ImColor(247, 255, 25, 255);
	s.Colors[ImGuiCol_FrameBgHovered] = ImColor(65, 64, 64, 255);
	s.Colors[ImGuiCol_ScrollbarBg] = ImColor(0, 0, 0, 255);
	s.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(128, 128, 128, 255);
	s.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(255, 255, 255, 255);
	s.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(255, 255, 255, 255);
	s.Colors[ImGuiCol_Header] = ImColor(42, 42, 42, 255);
	s.Colors[ImGuiCol_HeaderHovered] = ImColor(50, 50, 50, 255);
	s.Colors[ImGuiCol_HeaderActive] = ImColor(50, 50, 50, 255);
	s.Colors[ImGuiCol_PopupBg] = ImColor(15, 15, 15, 255);
	s.Colors[ImGuiCol_Button] = ImColor(30, 30, 30, 255);//
	s.Colors[ImGuiCol_ButtonHovered] = ImColor(128, 128, 128, 150);
	s.Colors[ImGuiCol_ButtonActive] = ImColor(14, 179, 97);
}

std::wstring s2ws(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

int get_fn_processid() {
	BYTE target_name[] = { 'F','o','r','t','n','i','t','e','C','l','i','e','n','t','-','W','i','n','6','4','-','S','h','i','p','p','i','n','g','.','e','x','e', 0 };
	std::wstring process_name = s2ws(std::string((char*)target_name));
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);

	if (!Process32First(snapshot, &entry)) {
		return 0;
	}

	while (Process32Next(snapshot, &entry)) {
		if (std::wstring(entry.szExeFile) == process_name) {
			return entry.th32ProcessID;
		}
	}

	return 0;
}

void background()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Once);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.20f / 1.f * 2.f));
	static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
	ImGui::Begin(XorStr("##background").c_str(), nullptr, flags);
	ImGui::End();
	ImGui::PopStyleColor();
}

void cursor()
{
	ImGui::SetNextWindowPos(ImGui::GetIO().MousePos);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Once);


	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.20f / 1.f * 2.f));
	static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove;
	ImGui::Begin(XorStr("##cursor").c_str(), nullptr, flags);
	ImGui::End();
	//ImGui::PopStyleColor();
}

ImColor cRainbow = ImGui::ColorConvertFloat4ToU32(ImVec4(255.0, 0.0, 0.0, 255.0));


void decoration()
{
	auto pos = ImGui::GetWindowPos();
	auto size = ImGui::GetWindowSize();
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 5, pos.y + 1), ImVec2(pos.x + size.x - 5, pos.y + size.y - 1), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 6, pos.y + 2), ImVec2(pos.x + size.x - 6, pos.y + size.y - 2), ImColor(40, 40, 40, 255));
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 7, pos.y + 3), ImVec2(pos.x + size.x - 7, pos.y + size.y - 3), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 8, pos.y + 4), ImVec2(pos.x + size.x - 8, pos.y + 6), cRainbow);
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 7, pos.y + 5), ImVec2(pos.x + size.x - 7, pos.y + size.y - 3), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 7, pos.y + 6), ImVec2(pos.x + size.x - 7, pos.y + size.y - 3), ImColor(40, 40, 40, 255));
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 7, pos.y + 7), ImVec2(pos.x + size.x - 7, pos.y + size.y - 3), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 8, pos.y + 8), ImVec2(pos.x + size.x - 8, pos.y + size.y - 4), ImColor(30, 30, 30, 255));

	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 12, pos.y + 23), ImVec2(pos.x + size.x - 12, pos.y + size.y - 8), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 13, pos.y + 24), ImVec2(pos.x + size.x - 13, pos.y + size.y - 9), ImColor(40, 40, 40, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 17, pos.y + 28), ImVec2(pos.x + size.x - 17, pos.y + size.y - 13), ImColor(1, 1, 1, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 18, pos.y + 29), ImVec2(pos.x + size.x - 18, pos.y + size.y - 14), ImColor(34, 34, 34, 255));
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 18, pos.y + 29), ImVec2(pos.x + 133, pos.y + size.y - 14), ImColor(31, 31, 31, 255));


	ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 14, pos.y + 8), ImColor(255, 255, 255, 255), "Gloomy");
	ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + 50, pos.y + 8), ImColor(255, 0, 0, 255), ".cc");
	ImGui::GetWindowDrawList()->AddRect(ImVec2(pos.x + 17, pos.y + 28), ImVec2(pos.x + 133, pos.y + size.y - 13), ImColor(1, 1, 1, 255));
}

bool g_overlay_visible{ false };
bool g_fovchanger{ false };
bool g_esp_enabled{ true };
bool g_esp_distance{ true };
bool g_esp_skeleton{ false };
bool g_3d_box{ false };
bool g_aimbot{ true };
bool g_skipknocked{ true };
bool g_trigger{ false };
bool g_lineesp{ false };
bool g_boxesp{ false };
bool g_fov{ false };
bool g_circlefov{ true };
bool g_crossh{ true };
bool g_cornerboxesp{ false };
bool g_chests{ false };
bool g_vehicles{ false };
bool g_ammo{ false };
bool g_ammos{ false };
bool g_loot{ false };
bool g_curweaponesp{ true };
bool g_consumables{ false };
bool g_spoofesp{ false };
bool g_mouse_aim{ true };
bool g_mem_aim{ false };
bool g_utils{ false };
bool controller = false;
bool g_exploits_backtrack{ false };
bool g_gun_tracers{ false };
bool g_disable_gunshots{ false };
bool g_spinbot{ false };
bool g_chams{ false };
bool g_platform_esp{ false };
bool g_name_esp{ false };
bool g_boatspeed{ false };
bool g_boatfly_test{ false };

/*Exploits added by ac-1337 [ToTo]*/

bool g_RocketLeauge{ false };
bool g_Aimbotgay{ false };
bool g_carfly{ false };
bool g_airstuck{ false };
bool g_playerfly{ false };
bool g_watermark{ true };
bool g_boatrat{ false };
bool g_instarev{ false };
bool g_tpose{ false };
bool g_rapidaug{ false };
bool g_doublepump{ false };
bool g_NoColision{ false };
