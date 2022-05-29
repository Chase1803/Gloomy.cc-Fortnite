/*This update its from ac-1337 pls rat me uwu*/
/*This update its from ac-1337 pls rat me uwu*/
/*This update its from ac-1337 pls rat me uwu*/
/*This update its from ac-1337 pls rat me uwu*/
/*
	Gloomy.cc
	https://github.com/Chase1803

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

#include "icon.h"
#include <iostream>
#include "GL/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <intrin.h>
#include "driverloader/singed.hpp"
#include "driver.h"
#include "Controller.h"
#include <d3d9.h>
#include "d3dx9.h"
#include <thread>
#include <string>
#include <stdio.h>
#include <fstream>
#include "font.h"
#include "utils.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

extern void aimbot(float x, float y);
GLFWwindow* g_window;

bool common{ false };
bool uncommon{ false };
bool rare{ false };
bool epic{ false };
bool legendary{ false };
bool mythic{ false };

struct FMinimalViewInfo
{
	Vector3 Location;
	Vector3 Rotation;
	float FOV;
};

static void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, XorStr("OverEW: %d: %s\n").c_str(), error, description);
}

void setupWindow()
{
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		std::cout << XorStr("Over Init Failed!\n").c_str();
		return;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (!monitor) {
		fprintf(stderr, XorStr("Failed to get monitor!\n").c_str());
		return;
	}

	g_width = glfwGetVideoMode(monitor)->width;
	g_height = glfwGetVideoMode(monitor)->height;

	glfwWindowHint(GLFW_FLOATING, true);
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_MAXIMIZED, false);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

	g_window = glfwCreateWindow(g_width, g_height, XorStr("Blade System Decoraytion").c_str(), NULL, NULL);

	if (g_window == NULL) {
		std::cout << XorStr("Over: 0x2!\n").c_str();
		return;
	}

	glfwSetWindowAttrib(g_window, GLFW_DECORATED, false);
	glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, true);

	glfwSetWindowMonitor(g_window, NULL, 0, 0, g_width, g_height + 1, 0);

	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, XorStr("Over: 0x1!\n").c_str());
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFont* m_pFont;
	ImFont* m_pFont1;

	ImFontConfig CustomFont;
	CustomFont.FontDataOwnedByAtlas = false;

	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFontConfig icons_config;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;

	m_pFont = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 15.5, &CustomFont);
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 20.5f, &icons_config, icons_ranges);

	menucolors();

	ImGui_ImplGlfw_InitForOpenGL(g_window, true);
	ImGui_ImplOpenGL3_Init(XorStr("#version 130").c_str());
}

void cleanupWindow()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window);
	glfwTerminate();
}

BOOL CALLBACK retreiveFortniteWindow(HWND hwnd, LPARAM lparam)
{
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (process_id == g_pid) {
		fortnite_wnd = hwnd;
	}
	return TRUE;
}

bool isFortniteFocused = false;

void activateFortniteWindow()
{
	SetForegroundWindow(fortnite_wnd);
}

void handleKeyPresses()
{
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		g_overlay_visible = !g_overlay_visible;
		glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, !g_overlay_visible);
		if (g_overlay_visible) {
			HWND overlay_window = glfwGetWin32Window(g_window);
			SetForegroundWindow(overlay_window);
		}
		else {
			activateFortniteWindow();
		}
	}
}

void CacheLevels()
{
	if (Globals::LocalPawn)
	{
		std::vector<LootEntity> tmpList;
		uintptr_t ItemLevels = read<uintptr_t>(g_pid, GWorld + 0x160);

		for (int i = 0; i < read<DWORD>(g_pid, GWorld + (0x160 + sizeof(PVOID))); ++i) {

			uintptr_t ItemLevel = read<uintptr_t>(g_pid, ItemLevels + (i * sizeof(uintptr_t)));

			for (int i = 0; i < read<DWORD>(g_pid, ItemLevel + (0x98 + sizeof(PVOID))); ++i) {
				uintptr_t ItemsPawns = read<uintptr_t>(g_pid, ItemLevel + 0x98);
				uintptr_t CurrentItemPawn = read<uintptr_t>(g_pid, ItemsPawns + (i * sizeof(uintptr_t)));
				uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, CurrentItemPawn + 0x188);
				Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
				float ItemDist = Globals::LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;

				if (ItemDist < bLootRendering) {

					int ItemIndex = read<int>(g_pid, CurrentItemPawn + 0x18);
					auto CurrentItemPawnName = GetNameFromFName(ItemIndex);

					if ((g_loot && strstr(CurrentItemPawnName.c_str(), (XorStr("FortPickupAthena")).c_str())) || strstr(CurrentItemPawnName.c_str(), (XorStr("Tiered_Chest").c_str())) ||
						(g_vehicles && strstr(CurrentItemPawnName.c_str(), XorStr("Vehicl").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_Taxi").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BigRig").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BasicTr").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Valet_SportsC").c_str()) || strstr(CurrentItemPawnName.c_str(), XorStr("Valet_BasicC").c_str()) ||
							strstr(CurrentItemPawnName.c_str(), XorStr("Tiered_Ammo").c_str())))
					{
						LootEntity LevelObjects{ };
						LevelObjects.ACurrentItem = CurrentItemPawn;
						LevelObjects.GNames = CurrentItemPawnName;
						tmpList.push_back(LevelObjects);
					}
				}
			}
		}
		LootentityList.clear();
		LootentityList = tmpList;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void CacheGame()
{
	while (true)
	{
		std::vector<Playertest> tmpList;

		GWorld = read<uintptr_t>(g_pid, pattern_uworld);
		uintptr_t GameInstance = read<uintptr_t>(g_pid, GWorld + 0x1A8);
		uintptr_t LocalPlayers = read<uintptr_t>(g_pid, GameInstance + 0x38);
		Globals::LocalPlayer = read<uintptr_t>(g_pid, LocalPlayers);
		LocalPlayerController = read<uintptr_t>(g_pid, Globals::LocalPlayer + 0x30);
		MyHUD = read<uintptr_t>(g_pid, LocalPlayerController + 0x320);
		PlayerCameraManager = read<uint64_t>(g_pid, LocalPlayerController + 0x328);
		Globals::LocalPawn = read<uintptr_t>(g_pid, LocalPlayerController + 0x310);
		Globals::LocalPawnRootComponent = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x188);
		uint64_t LocalPlayerState = read<uint64_t>(g_pid, Globals::LocalPawn + 0x290);
		LocalTeam = read<int>(g_pid, LocalPlayerState + 0x1010);

		InLobby = false;
		if (!Globals::LocalPawn) InLobby = true;

		uintptr_t PersistentLevel = read<uintptr_t>(g_pid, GWorld + 0x30);
		auto ActorArray = read<DWORD>(g_pid, PersistentLevel + 0xa0);
		auto Actors = read<uintptr_t>(g_pid, PersistentLevel + 0x98);

		for (int i = 0; i < ActorArray; ++i) {
			uintptr_t CurrentItemPawn = read<uintptr_t>(g_pid, Actors + (i * sizeof(uintptr_t)));

			int Index = read<int>(g_pid, CurrentItemPawn + 0x18);
			auto CurrentItemPawnName = GetNameFromFName(Index);

			if (strstr(CurrentItemPawnName.c_str(), "PlayerPawn_Athena_C") || strstr(CurrentItemPawnName.c_str(), "PlayerPawn"))
			{
				Playertest Actor{ };

				Actor.ACurrentActor = CurrentItemPawn;
				Actor.USkeletalMeshComponent = read<uint64_t>(g_pid, CurrentItemPawn + 0x2f0);
				Actor.GNames = CurrentItemPawnName;
				Actor.USceneComponent = Globals::LocalPawnRootComponent;

				tmpList.push_back(Actor);
			}
		}

		CacheLevels();
		PLIST.clear();
		PLIST = tmpList;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool isVisible(DWORD_PTR mesh)
{
	if (!mesh)
		return false;
	float fLastSubmitTime = read<float>(g_pid, mesh + 0x330);
	float fLastRenderTimeOnScreen = read<float>(g_pid, mesh + 0x338);

	const float fVisionTick = 0.06f;
	bool bVisible = fLastRenderTimeOnScreen + fVisionTick >= fLastSubmitTime;
	return bVisible;
}


bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
}


void WriteAngles(Vector3 Location)
{
	//Depreciated
}

bool CheatLoop()
{
	bool bValidEnemyInArea = false;
	float ClosestActorDistance = FLT_MAX;
	Vector3 ClosestActorMouseAimbotPosition = Vector3(0.0f, 0.0f, 0.0f);
	float distance, distaim;

	try
	{
		for (LootEntity LEntityList : LootentityList) {

			if (Globals::LocalPawn)
			{
				uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, LEntityList.ACurrentItem + 0x188);
				Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
				float ItemDist = Globals::LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;
				std::string null = ("");

				auto IsSearched = read<BYTE>(g_pid, (uintptr_t)LEntityList.ACurrentItem + 0xf41);
				if (IsSearched >> 7 & 1) continue;

				if (strstr(LEntityList.GNames.c_str(), ("Tiered_Chest")) && g_chests)
				{
					if (ItemDist < bLootRendering) {
						Vector3 ChestPosition;
						ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);
						std::string Text = null + ("Chest [") + std::to_string((int)ItemDist) + ("m]");
						DrawString(14, ChestPosition.x, ChestPosition.y, &Col.yellow, true, true, Text.c_str());
					}
				}

				else if ((g_vehicles && (strstr(LEntityList.GNames.c_str(), XorStr("Vehicl").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_Taxi").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BigRig").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BasicTr").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_SportsC").c_str()) || strstr(LEntityList.GNames.c_str(), XorStr("Valet_BasicC").c_str()))))
				{
					if (ItemDist < bLootRendering) {
						Vector3 VehiclePosition = g_functions::ConvertWorld2Screen(ItemPosition);
						std::string Text = null + ("Vehicle [") + std::to_string((int)ItemDist) + ("m]");
						DrawString(14, VehiclePosition.x, VehiclePosition.y, &Col.red, true, true, Text.c_str());
					}
				}

				else if (strstr(LEntityList.GNames.c_str(), ("AthenaSupplyDrop_C")) && g_loot)
				{
					if (ItemDist < bLootRendering) {
						Vector3 ChestPosition;
						ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);

						std::string Text = null + ("Supply Drop [") + std::to_string((int)ItemDist) + ("m]");
						DrawString(14, ChestPosition.x, ChestPosition.y, &Col.blue, true, true, Text.c_str());

					}
				}

				else if (strstr(LEntityList.GNames.c_str(), ("Tiered_Ammo")) && g_ammo)
				{
					if (ItemDist < bLootRendering) {
						Vector3 ChestPosition;
						ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);
						std::string Text = null + ("Ammo Box [") + std::to_string((int)ItemDist) + ("m]");
						DrawString(14, ChestPosition.x, ChestPosition.y, &Col.white, true, true, Text.c_str());

					}
				}

				else if (g_loot && strstr(LEntityList.GNames.c_str(), ("FortPickupAthena")) || strstr(LEntityList.GNames.c_str(), ("Fort_Pickup_Creative_C")))
				{
					if (ItemDist < bLootRendering) {

						auto definition = read<uint64_t>(g_pid, LEntityList.ACurrentItem + 0x2f8 + 0x18);
						BYTE tier = read<BYTE>(g_pid, definition + 0x74);

						RGBA Color, RGBAColor;
						Vector3 ChestPosition = g_functions::ConvertWorld2Screen(ItemPosition);

						if (g_loot)
						{
							auto DisplayName = read<uint64_t>(g_pid, definition + 0x90);
							auto WeaponLength = read<uint32_t>(g_pid, DisplayName + 0x38);
							wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

							Drive.ReadPtr(g_pid, (ULONG64)read<PVOID>(g_pid, DisplayName + 0x30), WeaponName, WeaponLength * sizeof(wchar_t));

							std::string Text = wchar_to_char(WeaponName);
							std::string wtf2 = Text + " [" + std::to_string((int)ItemDist) + ("m]");
							if (tier == 2 && (uncommon))
							{
								Color = Col.green;
							}
							else if ((tier == 3) && (rare))
							{
								Color = Col.blue;
							}
							else if ((tier == 4) && (epic))
							{
								Color = Col.purple;
							}
							else if ((tier == 5) && (legendary))
							{
								Color = Col.yellow;
							}
							else if ((tier == 6) && (mythic))
							{
								Color = Col.yellow;
							}
							else if ((tier == 0) || (tier == 1) && (common))
							{
								Color = Col.white;
							}

							DrawString(14, ChestPosition.x, ChestPosition.y, &Color, true, true, wtf2.c_str());
						}
					}
				}
			}
		}

		for (Playertest EntityList : PLIST)
		{
			auto identify = g_functions::f_getbonewithIndex(0, 0);
			g_functions::ConvertWorld2Screen(identify);

			uintptr_t RootComponent = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x188);
			Globals::LocalPlayerRelativeLocation = read<Vector3>(g_pid, EntityList.USceneComponent + 0x128);

			if (Globals::LocalPawn)
			{
				const auto SetupCameraRotationAndFov = [](uintptr_t LocalPlayer, uintptr_t RootComponent, Vector3& CameraRotation, float& FOVAngle)
				{
					auto CameraRotChain_tmp0 = read<uint64_t>(g_pid, LocalPlayer + 0xC8);
					auto CameraRotChain_tmp1 = read<uint64_t>(g_pid, CameraRotChain_tmp0 + 0x8);

					camera::m_CameraRotation.x = read<double>(g_pid, CameraRotChain_tmp1 + 0xAE0);

					double tester = asin(camera::m_CameraRotation.x);
					double degreees = tester * (180.0 / M_PI);

					camera::m_CameraRotation.y = read<double>(g_pid, RootComponent + 0x148);
					camera::m_CameraRotation.x = degreees;

					//set fov angle
					if (g_fovchanger)
					{
						FOVAngle = FOVChangerValue;
					}
					else
					{
						FOVAngle = 80.0f / (read<double>(g_pid, CameraRotChain_tmp1 + 0x610) / 1.19f);
					}

				};
				SetupCameraRotationAndFov(Globals::LocalPlayer, Globals::LocalPawnRootComponent, camera::m_CameraRotation, camera::m_FovAngle);
			}
			else {
				auto CameraCache = read<FMinimalViewInfo>(g_pid, PlayerCameraManager + 0x28d0 + 0x10);

				camera::m_CameraRotation = CameraCache.Rotation;
				camera::m_CameraLocation = CameraCache.Location;

				if (g_fovchanger)
				{
					camera::m_FovAngle = FOVChangerValue;
				}
				else
				{
					camera::m_FovAngle = CameraCache.FOV;
				}
			}

			// get camera location
			const auto SetupCameraLocation = [](uintptr_t LocalPlayer, Vector3& CameraLocation)
			{
				auto CameraLocChain_tmp0 = read<uint64_t>(g_pid, LocalPlayer + 0x70);
				auto CameraLocChain_tmp1 = read<uint64_t>(g_pid, CameraLocChain_tmp0 + 0x98);
				auto CameraLocChain_tmp2 = read<uint64_t>(g_pid, CameraLocChain_tmp1 + 0x180);

				CameraLocation = read<Vector3>(g_pid, CameraLocChain_tmp2 + 0x20);

			};
			SetupCameraLocation(Globals::LocalPlayer, camera::m_CameraLocation);

			uint64_t PlayerState = read<uint64_t>(g_pid, EntityList.ACurrentActor + 0x290);

			int TeamIndex = read<int>(g_pid, PlayerState + 0x1010);

			auto CameraActor = read<uintptr_t>(g_pid, PlayerCameraManager + 0x3168);

			if (g_fovchanger)
			{
				write(g_pid, PlayerCameraManager + 0x288, FOVChangerValue);
				//write(g_pid, CameraActor + 0x298, FOVChangerValue);
			}

			Vector3 vHeadBone = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 98);
			Vector3 vRootBone = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 0);

			Vector3 vHeadBoneOut = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 15));
			Vector3 vRootBoneOut = g_functions::ConvertWorld2Screen(vRootBone);

			Vector3 w2shead = g_functions::ConvertWorld2Screen(vHeadBone);

			Vector3 RootPos = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 68);
			Vector3 RootPosOut = g_functions::ConvertWorld2Screen(RootPos);

			float BoxHeight = abs(vHeadBoneOut.y - vRootBoneOut.y);
			float BoxWidth = BoxHeight * 0.80;

			Vector3 RelativeInternalLocation = read<Vector3>(g_pid, RootComponent + 0x128);
			if (!RelativeInternalLocation.x && !RelativeInternalLocation.y) {
				continue;
			}

			Vector3 RelativeScreenLocation = g_functions::ConvertWorld2Screen(RelativeInternalLocation);
			if (!RelativeScreenLocation.x && !RelativeScreenLocation.y) {
				continue;
			}

			distance = Globals::LocalPlayerRelativeLocation.Distance(RelativeInternalLocation) / 100.f;

			if (g_name_esp)
			{
				uint64_t APlayerState = read<uint64_t>(g_pid, 0x0);//APlayerState->PlayerNamePrivate*FString
				auto nameptr = read<uintptr_t>(g_pid, APlayerState + 0x370);

				uint64_t StringData = read<uint64_t>(g_pid, nameptr);
				uint32_t StringLength = read<uint32_t>(g_pid, nameptr + 0x8);
				wchar_t* OutString = new wchar_t[StringLength + 1];
				Drive.ReadPtr(g_pid, StringData, OutString, StringLength * sizeof(wchar_t));

				std::string Text = wchar_to_char(OutString);

				DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 50, &Col.white, true, true, Text.c_str());
			}

			if (g_platform_esp)
			{
				uint64_t AFortPlayerState = read<uint64_t>(g_pid, 0x0);//AFortPlayerState->Platform*FString
				auto nameptr = read<uintptr_t>(g_pid, AFortPlayerState + 0x420);

				uint64_t StringData = read<uint64_t>(g_pid, nameptr);
				uint32_t StringLength = read<uint32_t>(g_pid, nameptr + 0x8);
				wchar_t* OutString = new wchar_t[StringLength + 1];
				Drive.ReadPtr(g_pid, StringData, OutString, StringLength * sizeof(wchar_t));

				std::string Text = wchar_to_char(OutString);

				if (strstr(Text.c_str(), ("WIN")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.white, true, true, "PC");

				}
				else if (strstr(Text.c_str(), ("XBL")) || strstr(Text.c_str(), ("XSX")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.green, true, true, "Xbox");

				}
				else if (strstr(Text.c_str(), ("PSN")) || strstr(Text.c_str(), ("PS5")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.lightblue, true, true, "PSN");

				}
				else if (strstr(Text.c_str(), ("SWT")))
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.red, true, true, "Nintendo");

				}
				else
				{
					DrawString(13, vHeadBoneOut.x, vHeadBoneOut.y - 20, &Col.white, true, true, Text.c_str());
				}

			}

			Vector3 neck2 = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 98);
			Vector3 vneck2 = g_functions::ConvertWorld2Screen(neck2);
			Vector3 pelvis = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 2);
			Vector3 vpelvis = g_functions::ConvertWorld2Screen(pelvis);
			Vector3 bottom1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z));
			Vector3 bottom2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z));
			Vector3 bottom3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z));
			Vector3 bottom4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z));

			Vector3 top1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z + 15));
			Vector3 top2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z + 15));
			Vector3 top3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z + 15));
			Vector3 top4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z + 15));

			ImU32 Cay = ImGui::GetColorU32({ 255, 215, 0, 255 });

			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), Cay, 0.1f);

			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), Cay, 0.1f);

			ImGui::GetForegroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), Cay, 0.1f);
			ImGui::GetForegroundDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), Cay, 0.1f);
			RGBA Skellll;
			Skellll = { 255, 255, 255, 255 };
			DrawLine(vneck2.x, vneck2.y, vpelvis.x, vpelvis.y, &Skellll, 0.5f);

			if (g_curweaponesp)
			{
				uintptr_t ItemRootComponent = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x188);
				Vector3 ItemPosition = read<Vector3>(g_pid, ItemRootComponent + 0x128);
				float ItemDist = Globals::LocalPlayerRelativeLocation.Distance(ItemPosition) / 100.f;

				if (ItemDist < bLootRendering) {

					auto CurrentWeapon = read<uintptr_t>(g_pid, EntityList.ACurrentActor + 0x790);
					auto ItemData = read<DWORD_PTR>(g_pid, CurrentWeapon + 0x3d8);
					BYTE tier = read<BYTE>(g_pid, ItemData + 0x74);

					ImColor Color;
					if (tier == 2)
					{
						Color = IM_COL32(0, 255, 0, 255);
					}
					else if ((tier == 3))
					{
						Color = IM_COL32(0, 0, 255, 255);
					}
					else if ((tier == 4))
					{
						Color = IM_COL32(128, 0, 128, 255);
					}
					else if ((tier == 5))
					{
						Color = IM_COL32(255, 255, 0, 255);
					}
					else if ((tier == 6))
					{
						Color = IM_COL32(255, 255, 0, 255);
					}
					else if ((tier == 0) || (tier == 1))
					{
						Color = IM_COL32(255, 255, 255, 255);
					}

					auto AmmoCount = read<int>(g_pid, CurrentWeapon + 0xb24);
					auto bIsReloadingWeapon = read<bool>(g_pid, CurrentWeapon + 0x311);

					auto DisplayName = read<uint64_t>(g_pid, ItemData + 0x90);
					auto WeaponLength = read<uint32_t>(g_pid, DisplayName + 0x38);
					wchar_t* WeaponName = new wchar_t[uint64_t(WeaponLength) + 1];

					Drive.ReadPtr(g_pid, (ULONG64)read<PVOID>(g_pid, DisplayName + 0x30), WeaponName, WeaponLength * sizeof(wchar_t));
					std::string Text = wchar_to_char(WeaponName);

					if (strstr(EntityList.GNames.c_str(), "PlayerPawn_Athena_C"))
					{
						std::string Player = XorStr("Player").c_str();
						ImVec2 TextSize = ImGui::CalcTextSize(Player.c_str());
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vHeadBoneOut.x - 30 - TextSize.x / 2, vHeadBoneOut.y - 15 - TextSize.y / 2), ImGui::GetColorU32({ 255, 255, 255, 255 }), Player.c_str());
					}
					else
					{
						std::string Bot = XorStr("Bot/Npc").c_str();
						ImVec2 TextSize = ImGui::CalcTextSize(Bot.c_str());
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vHeadBoneOut.x - 30 - TextSize.x / 2, vHeadBoneOut.y - 15 - TextSize.y / 2), ImGui::GetColorU32({ 255, 255, 255, 255 }), Bot.c_str());
					}

					if (AmmoCount)
					{
						char buffer[128];
						sprintf_s(buffer, "Ammo: %i", AmmoCount);
						if (buffer != "?")
						{
							ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x, vpelvis.y + 15), IM_COL32(255, 255, 255, 255), buffer);
						}
					}

					if (bIsReloadingWeapon)
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x - 30, vpelvis.y), IM_COL32(255, 255, 255, 255), "Reloading");
					else
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vpelvis.x - 30, vpelvis.y), Color, Text.c_str());
				}
			}

			int Teamcheck = TeamIndex != LocalTeam;

			if (Teamcheck || InLobby) {

				isVis = isVisible(EntityList.USkeletalMeshComponent);
				if (distance <= bE5pD1st4nce || InLobby)
				{
					if (g_boxesp)
					{
						if (isVis)
							DrawNormalBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &ESPColor2, &ESPColor2);
						else
							DrawNormalBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, 1.0f, &ESPColor, &ESPColor);
					}

					else if (g_cornerboxesp)
					{
						if (isVis)
							DrawCorneredBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, IM_COL32(255, 215, 0, 255), 1.5);
						else
							DrawCorneredBox(vRootBoneOut.x - (BoxWidth / 2), vHeadBoneOut.y, BoxWidth, BoxHeight, IM_COL32(255, 255, 255, 255), 1.5);
					}

					if (g_esp_distance) {

						char dist[64];
						sprintf_s(dist, "%.fM", distance);
						ImVec2 TextSize = ImGui::CalcTextSize(dist);
						ImGui::GetOverlayDrawList()->AddText(ImVec2(vRootBoneOut.x - 15 - TextSize.x / 2, vRootBoneOut.y - 15 - TextSize.y / 2), ImGui::GetColorU32({ 255, 255, 255, 255 }), dist);
					}

					if (g_esp_skeleton) {
						Vector3 neck2 = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 98);
						Vector3 vneck2 = g_functions::ConvertWorld2Screen(neck2);

						Vector3 neck = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 66);
						Vector3 vneck = g_functions::ConvertWorld2Screen(neck);
						Vector3 rightChest = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 8);
						Vector3 vrightChest = g_functions::ConvertWorld2Screen(rightChest);
						Vector3 leftChest = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 37);
						Vector3 vleftChest = g_functions::ConvertWorld2Screen(leftChest);
						Vector3 leftShoulder = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 38);
						Vector3 vleftShoulder = g_functions::ConvertWorld2Screen(leftShoulder);
						Vector3 rightShoulder = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 9);
						Vector3 vrightShoulder = g_functions::ConvertWorld2Screen(rightShoulder);
						Vector3 leftElbow = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 94);
						Vector3 vleftElbow = g_functions::ConvertWorld2Screen(leftElbow);
						Vector3 rightElbow = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 10);
						Vector3 vrightElbow = g_functions::ConvertWorld2Screen(rightElbow);
						Vector3 leftWrist = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 62);
						Vector3 vleftWrist = g_functions::ConvertWorld2Screen(leftWrist);
						Vector3 rightWrist = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 33);
						Vector3 vrightWrist = g_functions::ConvertWorld2Screen(rightWrist);
						Vector3 pelvis = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 2);
						Vector3 vpelvis = g_functions::ConvertWorld2Screen(pelvis);
						Vector3 leftAss = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 76);
						Vector3 vleftAss = g_functions::ConvertWorld2Screen(leftAss);
						Vector3 rightAss = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 69);
						Vector3 vrightAss = g_functions::ConvertWorld2Screen(rightAss);
						Vector3 leftKnee = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 77);
						Vector3 vleftKnee = g_functions::ConvertWorld2Screen(leftKnee);
						Vector3 rightKnee = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 70);
						Vector3 vrightKnee = g_functions::ConvertWorld2Screen(rightKnee);
						Vector3 leftAnkle = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 78);
						Vector3 vleftAnkle = g_functions::ConvertWorld2Screen(leftAnkle);
						Vector3 rightAnkle = g_functions::f_getbonewithIndex(EntityList.USkeletalMeshComponent, 71);
						Vector3 vrightAnkle = g_functions::ConvertWorld2Screen(rightAnkle);

						RGBA ESPSkeleton;

						if (isVis)
							ESPSkeleton = { 0, 255, 0, 255 };
						else
							ESPSkeleton = { 255, 0, 0, 255 };

						DrawLine(vleftChest.x, vleftChest.y, vrightChest.x, vrightChest.y, &ESPSkeleton, 0.5f);
						DrawLine(vleftChest.x, vleftChest.y, vleftShoulder.x, vleftShoulder.y, &ESPSkeleton, 0.5f);
						DrawLine(vrightChest.x, vrightChest.y, vrightShoulder.x, vrightShoulder.y, &ESPSkeleton, 0.5f);
						DrawLine(vleftShoulder.x, vleftShoulder.y, vleftElbow.x, vleftElbow.y, &ESPSkeleton, 0.5f);
						DrawLine(vrightShoulder.x, vrightShoulder.y, vrightElbow.x, vrightElbow.y, &ESPSkeleton, 0.5f);
						DrawLine(vleftElbow.x, vleftElbow.y, vleftWrist.x, vleftWrist.y, &ESPSkeleton, 0.5f);
						DrawLine(vrightElbow.x, vrightElbow.y, vrightWrist.x, vrightWrist.y, &ESPSkeleton, 0.5f);
						DrawLine(vneck2.x, vneck2.y, vpelvis.x, vpelvis.y, &ESPSkeleton, 0.5f);
						DrawLine(vpelvis.x, vpelvis.y, vleftAss.x, vleftAss.y, &ESPSkeleton, 0.5f);
						DrawLine(vpelvis.x, vpelvis.y, vrightAss.x, vrightAss.y, &ESPSkeleton, 0.5f);
						DrawLine(vleftAss.x, vleftAss.y, vleftKnee.x, vleftKnee.y, &ESPSkeleton, 0.5f);
						DrawLine(vrightAss.x, vrightAss.y, vrightKnee.x, vrightKnee.y, &ESPSkeleton, 0.5f);
						DrawLine(vleftKnee.x, vleftKnee.y, vleftAnkle.x, vleftAnkle.y, &ESPSkeleton, 0.5f);
						DrawLine(vrightKnee.x, vrightKnee.y, vrightAnkle.x, vrightAnkle.y, &ESPSkeleton, 0.5f);
					}

					if (g_3d_box)
					{
						if (vHeadBoneOut.x != 0 || vHeadBoneOut.y != 0 || vHeadBoneOut.z != 0)
						{
							ImU32 ESPSkeleton;
							if (isVis)
								ESPSkeleton = ImGui::GetColorU32({ 255, 215, 0, 255 });
							else
								ESPSkeleton = ImGui::GetColorU32({ 255, 255, 255, 255 });

							Vector3 bottom1 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
							Vector3 bottom2 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
							Vector3 bottom3 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
							Vector3 bottom4 = g_functions::ConvertWorld2Screen(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));

							Vector3 top1 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
							Vector3 top2 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
							Vector3 top3 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
							Vector3 top4 = g_functions::ConvertWorld2Screen(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ESPSkeleton, 0.1f);

							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);
							ImGui::GetForegroundDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);
						}
					}

					if (g_lineesp)
					{
						ImU32 LineColor;
						if (isVis)
							LineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor2.R / 128.0, ESPColor2.G / 224.0, ESPColor2.B / 0.0, ESPColor2.A / 255.0));
						else
							LineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor.R / 128.0, ESPColor.G / 224.0, ESPColor.B / 0.0, ESPColor.A / 255.0));

						ImGui::GetOverlayDrawList()->AddLine(
							ImVec2(Globals::ScreenCenterX, Globals::Height),
							ImVec2(RelativeScreenLocation.x, RelativeScreenLocation.y + 30),
							LineColor,
							0.1f
						);
					}
				}


				auto dx = w2shead.x - (Globals::Width / 2);
				auto dy = w2shead.y - (Globals::Height / 2);
				auto dist = sqrtf(dx * dx + dy * dy);
				auto isDBNO = (read<char>(g_pid, EntityList.ACurrentActor + 0x6f2) >> 4) & 1;

				if (g_spinbot)
				{
					auto Mesh = read<uint64_t>(g_pid, Globals::LocalPawn + 0x2f0);
					static auto Cached = read<Vector3>(g_pid, Mesh + 0x140);

					if (GetAsyncKeyState(VK_RBUTTON)) {
						write<Vector3>(g_pid, Mesh + 0x140, Vector3(1, rand() % 361, 1));
					}
					else write<Vector3>(g_pid, Mesh + 0x140, Cached);
				}

				if (g_boatspeed)
				{
					//I love UC
					uint64_t Vehicle = read<uint64_t>(g_pid, Globals::LocalPawn + 0x2158); //FortPlayerPawn::CurrentVehicle
					write<float>(g_pid, Vehicle + 0xc6c, boatmulti);//multiplier run     AFortAthenaVehicle::CachedSpeed
					write<float>(g_pid, Vehicle + 0x8e0, boatmulti);//multiplier run     AFortAthenaVehicle::TopSpeedCurrentMultiplier
					write<float>(g_pid, Vehicle + 0x8e4, boatmulti);//multiplier run     AFortAthenaVehicle::PushForceCurrentMultiplier
					write<float>(g_pid, Vehicle + 0x770, boatspeed);//just speed         AFortAthenaVehicle::WaterEffectsVehicleMaxSpeedKmh
				}

				if (g_boatfly_test)
				{
					uint64_t Vehicle = read<uint64_t>(g_pid, Globals::LocalPawn + 0x2158);
					static auto Cached = read<char>(g_pid, Vehicle + 0x66a);
					if (GetAsyncKeyState(VK_SPACE)) write<char>(g_pid, Vehicle + 0x66a, 0);
					else write<char>(g_pid, Cached, 0);

				}
				if (dist < bA1mb0tF0VV4lue && dist < closestDistance && TeamIndex != LocalTeam && !InLobby)
				{
					if (g_skipknocked)
					{
						if (!isDBNO)
						{
							closestDistance = dist;
							closestPawn = EntityList.ACurrentActor;
						}
					}
					else
					{
						closestDistance = dist;
						closestPawn = EntityList.ACurrentActor;
					}
				}
			}
		}

		if (closestPawn != 0)
		{
			if (closestPawn && Key.IsKeyPushing(hotkeys::aimkey) or Controller::IsPressingLeftTrigger() && isFortniteFocused)
			{
				if (g_aimbot)
				{
					auto AimbotMesh = read<uint64_t>(g_pid, closestPawn + 0x2f0);
					if (!AimbotMesh)
						return false;

					auto CurrentWeapon = read<uintptr_t>(g_pid, closestPawn + 0x790);
					if (!CurrentWeapon)
						return false;

					Vector3 HeadPosition = g_functions::f_getbonewithIndex(AimbotMesh, select_hitbox());
					if (!IsVec3Valid(HeadPosition))
						return false;

					Vector3 Head = g_functions::ConvertWorld2Screen(HeadPosition);
					if (!IsVec3Valid(HeadPosition))
						return false;

					if (Head.x != 0 || Head.y != 0 || Head.z != 0)
					{
						if ((GetDistance(Head.x, Head.y, Head.z, Globals::Width / 2, Globals::Height / 2) <= bA1mb0tF0VV4lue))
						{
							if (g_mouse_aim)
							{
								aimbot(Head.x, Head.y);
							}
							/*
							else if (g_mem_aim)
							{
								Vector3 HeadPosition = g_functions::f_getbonewithIndex(AimbotMesh, 78);

								if (!IsVec3Valid(HeadPosition))
									return false;

								//WriteAngles(HeadPosition);

							}*/

							if (g_gun_tracers)
							{
								if (GetAsyncKeyState(VK_LBUTTON))
								{
									write<float>(g_pid, CurrentWeapon + 0x64, 99);
								}
								else
								{
									write<float>(g_pid, CurrentWeapon + 0x64, 1);
								}
							}
						}
					}
				}
			}
			else
			{
				closestDistance = FLT_MAX;
				closestPawn = NULL;
			}
		}
	}
	catch (...) {}
}

bool Head = true, Neck, Chest;

void runRenderTick() 
{
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	RECT rect = { 0 };

	HWND GameWnd = FindWindowA(XorStr("UnrealWindow").c_str(), XorStr("Fortnite  ").c_str());

	HWND hwnd_active = GetForegroundWindow();
	HWND overlay_window = glfwGetWin32Window(g_window);

	if (hwnd_active == GameWnd || hwnd_active == overlay_window) {
		isFortniteFocused = true;
	}
	else {
		isFortniteFocused = true;
	}

	if (isFortniteFocused)
	{
		if (GetWindowRect(GameWnd, &rect))
		{
			Globals::Width = rect.right - rect.left;
			Globals::Height = rect.bottom - rect.top;
		}

		Globals::ScreenCenterX = (Globals::Width / 2.0f);
		Globals::ScreenCenterY = (Globals::Height / 2.0f);

		CheatLoop();
	}

	float radiusx = bA1mb0tF0VV4lue * (Globals::ScreenCenterX / 100);
	float radiusy = bA1mb0tF0VV4lue * (Globals::ScreenCenterY / 100);

	float calcradius = (radiusx + radiusy) / 2;

	if (Globals::LocalPawn)
	{
		if (g_fov)
		{
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, &Col.white, 2);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.white, 2);
			DrawLine((float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.white, 2);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.white, 2);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, &Col.white, 3);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.white, 3);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, &Col.black_, 1);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.black_, 1);
			DrawLine((float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) + bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.black_, 1);
			DrawLine((float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) - bA1mb0tF0VV4lue, (float)(Globals::Width / 2) - bA1mb0tF0VV4lue, (float)(Globals::Height / 2) + bA1mb0tF0VV4lue, &Col.black_, 1);
		}

		if (g_crossh) {
			DrawLine((float)(Globals::Width / 2), (float)(Globals::Height / 2) - 8, (float)(Globals::Width / 2), (float)(Globals::Height / 2) + 8, &Col.white, 1);
			DrawLine((float)(Globals::Width / 2) - 8, (float)(Globals::Height / 2), (float)(Globals::Width / 2) + 8, (float)(Globals::Height / 2), &Col.white, 1);
		}

		if (g_circlefov) {
			ImGui::GetOverlayDrawList()->AddCircle(ImVec2(Globals::ScreenCenterX, Globals::ScreenCenterY), bA1mb0tF0VV4lue, ImGui::ColorConvertFloat4ToU32(ImVec4(ESPColor.R / 255.0, ESPColor.G / 255.0, ESPColor.B / 255.0, ESPColor.A / 255.0)), 100);
		}
	}

	if (g_overlay_visible)
		background();

	ImGuiIO& io = ImGui::GetIO();



	if (g_rapidaug) {
		uintptr_t CurrentWeapon = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x790);
		auto curWeaon = read<DWORD_PTR>(g_pid, Globals::LocalPawn + CurrentWeapon);
		write<float>(g_pid, curWeaon + 0x64, 200);

	}

	if (g_doublepump) {
		uintptr_t CurrentWeapon = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x790);
		if (CurrentWeapon) {
			write<bool>(g_pid, CurrentWeapon + 0xf41, true); //AFortWeapon    bIgnoreTryToFireSlotCooldownRestriction    0xf41    bool
		}
	}

	if (g_tpose) {

		uintptr_t mesh = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x2F0);
		write<int>(g_pid, mesh + 0x9aa, 1);
	}

	if (g_instarev) {
		write<float>(g_pid, Globals::LocalPawn + 0x3f60, .000000000000000000000001);
	}

	if (g_boatrat) {
		uint64_t VEHICLE_STATS = read<uint64_t>(g_pid, Globals::LocalPawn + 0x2158);

		write<float>(g_pid, VEHICLE_STATS + 0xc64, 2.5);//multiplier run
		write<float>(g_pid, VEHICLE_STATS + 0x8d8, 2.5);//multiplier run
		write<float>(g_pid, VEHICLE_STATS + 0x23c0, 2.5);				//FortSpaghettiVehicle::CacheBoostFX = 0x23C0
		write<float>(g_pid, VEHICLE_STATS + 0x8dc, 2.5);//multiplier run
		write<float>(g_pid, VEHICLE_STATS + 0x1bc, 2.5);//just speed
		write<float>(g_pid, VEHICLE_STATS + 0x768, 2.5);
		write<float>(g_pid, VEHICLE_STATS + 0x1a30, 0.001);
		write<float>(g_pid, VEHICLE_STATS + 0x1a40, 0.001);
		write<float>(g_pid, VEHICLE_STATS + 0x18d0, 0.001);
	}

	if (g_watermark) {
		char dist[64];
		sprintf_s(dist, "         To Open Menu Press - Insert\n", ImGui::GetIO().Framerate);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(8, 2), IM_COL32(79, 125, 249, 255), dist);

		sprintf_s(dist, "   Gloomy.cc\n", ImGui::GetIO().Framerate);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(8, 15), IM_COL32(79, 125, 249, 255), dist);

	}


	if (g_playerfly)
	{
		write<float>(g_pid, Globals::LocalPawn + 0x1ef0, 1000.f);
		write<bool>(g_pid, Globals::LocalPawn + 0x1ea0 + 0x18, true);
	}

	if (g_airstuck) {
		if (GetAsyncKeyState(VK_SHIFT)) { //shift Keybind
			write<float>(g_pid, Globals::LocalPawn + 0x64, 0.01); //CustomTimeDilation Offset
		}
		else {
			write<float>(g_pid, Globals::LocalPawn + 0x64, 1); //CustomTimeDilation Offset
		}
	}

	if (g_carfly)
	{
		uintptr_t CurrentVehicle = read<DWORD_PTR>(g_pid, Globals::LocalPawn + 0x2158);

		if (CurrentVehicle) //checks if you are in a vehicle
		{
			write<bool>(g_pid, CurrentVehicle + 0x668, false); //if in vehicle then it disables vehicle gravity
		}
		else
		{
			write<bool>(g_pid, CurrentVehicle + 0x668, true); //if not in vehicle then it enables vehicle gravity
		}
	}

	if (g_instarev) {
		write<float>(g_pid, Globals::LocalPawn + 0x3F60, .000000000000000000000001);
	}
	else
	{
		write<float>(g_pid, Globals::LocalPawn + 0x3F60, 10);
	}

	if (g_Aimbotgay) {
		bool mouse_aim = true;
		if (GetAsyncKeyState(VK_RBUTTON)) {
			if (Globals::LocalPawn) {
				uintptr_t Mesh = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x300);
				write<Vector3>(g_pid, Mesh + 0x158, Vector3(2000, -2000, 2000)); //Class Engine.SceneComponent -> RelativeScale3D -> 0x134
			}
		}
		else {
			uintptr_t Mesh = read<uintptr_t>(g_pid, Globals::LocalPawn + 0x300);
			write<Vector3>(g_pid, Mesh + 0x158, Vector3(0, 0, -87)); //Class Engine.SceneComponent -> RelativeScale3D -> 0x134
		}

	}

	if (g_RocketLeauge) {
		if (GetAsyncKeyState(VK_SHIFT)) {
			write<bool>(g_pid, Globals::LocalPawn + 0x1794, true); //bBoosting offset
		}
		else {
			write<bool>(g_pid, Globals::LocalPawn + 0x1794, false); //bBoosting offset
		}
	}

	if (g_NoColision) {
		write<float>(g_pid, Globals::LocalPawn + 0x790, 0.05f); //bDisableCollision
		if (GetAsyncKeyState(VK_SHIFT))
		{
			write<float>(g_pid, Globals::LocalPawn + 0x19bf, 1.00f); //bIsSkydivingFromLaunchPad
		}
	}


	ImVec2 pos;
	ImDrawList* draw;

	ImFont* info = nullptr;
	ImFont* iconfont = nullptr;
	ImFont* iconfont_big = nullptr;
	ImFont* info_big = nullptr;
	ImFont* two = nullptr;
	ImFont* three = nullptr;
	ImFont* tabsf = nullptr;
	ImFont* ee = nullptr;

	if (g_overlay_visible) {
		{

			static ImVec4 active = ImColor(79, 125, 249, 255);
			static ImVec4 inactive = ImColor(255, 255, 255, 255);
			static ImVec4 exitt = ImColor(255, 0, 0, 255);

			ImGuiStyle& style = ImGui::GetStyle();
			ImGuiIO& io = ImGui::GetIO();


			style.Colors[ImGuiCol_WindowBg] = ImColor(18, 18, 18, 255);
			style.Colors[ImGuiCol_ChildBg] = ImColor(18, 18, 18, 255);
			style.Colors[ImGuiCol_ButtonActive] = ImColor(14, 179, 97);
			style.Colors[ImGuiCol_FrameBg] = ImColor(14, 159, 253, 255);
			style.Colors[ImGuiCol_FrameBgActive] = ImColor(14, 159, 253, 255);
			style.Colors[ImGuiCol_Button] = ImColor(79, 125, 249, 255);
			style.Colors[ImGuiCol_SeparatorActive] = ImColor(79, 125, 249, 255);
			style.Colors[ImGuiCol_Separator] = ImColor(79, 125, 249, 255);
			style.Colors[ImGuiCol_SeparatorHovered] = ImColor(79, 125, 249, 255);
			style.Colors[ImGuiCol_Border] = ImColor(36, 36, 36, 255);


			style.FrameRounding = 3.f;
			style.WindowRounding = 8.f;
			style.ChildRounding = 0.f;
			style.ChildBorderSize = 1.0f;

			io.IniFilename = NULL;

			static int tabs = 0;



			ImGui::SetNextWindowSize({ 610,450 });
			ImGui::Begin("", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar);

			ImGui::SetCursorPos(ImVec2(-1, 0));
			ImGui::BeginChild("##tabs", ImVec2(110, 455), true);

			ImGui::PushStyleColor(ImGuiCol_Text, tabs == 0 ? active : inactive);

			ImGui::SetCursorPos(ImVec2(43, 23));
			ImGui::Text(ICON_FA_SCOPE);
			if (ImGui::IsItemClicked()) { tabs = 0; }



			ImGui::PushStyleColor(ImGuiCol_Text, tabs == 1 ? active : inactive);
			ImGui::SetCursorPos(ImVec2(43, 67));
			ImGui::Text(ICON_FA_EYE);
			if (ImGui::IsItemClicked()) { tabs = 1; }

			ImGui::PushStyleColor(ImGuiCol_Text, tabs == 2 ? active : inactive);
			ImGui::SetCursorPos(ImVec2(43, 111));
			ImGui::Text(ICON_FA_BOMB);
			if (ImGui::IsItemClicked()) { tabs = 2; }

			ImGui::PushStyleColor(ImGuiCol_Text, tabs == 3 ? active : inactive);
			ImGui::SetCursorPos(ImVec2(43, 155));
			ImGui::Text(ICON_FA_USER);
			if (ImGui::IsItemClicked()) { tabs = 3; }

			ImGui::PushStyleColor(ImGuiCol_Text, inactive);
			ImGui::SetCursorPos(ImVec2(43, 405));
			ImGui::Text(ICON_FA_SIGN_OUT);
			if (ImGui::IsItemClicked()) { exit(0); }

			ImGui::EndChild();

			ImGui::PushStyleColor(ImGuiCol_Text, inactive);

			ImGui::SetCursorPos(ImVec2(120, 10));
			ImGui::BeginChild("##menu", ImVec2(235, 370), true);



			if (tabs == 0)
			{
				ImGui::Text("					    AIMBOT");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Checkbox(XorStr("Enable Aimbot").c_str(), &g_aimbot);
				if (g_aimbot)
				{
					ImGui::SameLine(0, 1);

					HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus);	
					ImGui::Checkbox(XorStr("Controller LTrigger").c_str(), &controller);
					if (controller)
						hotkeys::aimkey = false;
				}
				if (g_aimbot)
				{
					ImGui::Checkbox(XorStr("Mouse Aimbot").c_str(), &g_mouse_aim);
					if (g_mouse_aim)
						g_mem_aim = false;
					ImGui::Checkbox(XorStr("Memory Aimbot (Humanized)").c_str(), &g_mem_aim);
					if (g_mem_aim)
						g_mouse_aim = false;
					ImGui::Checkbox(XorStr("First Person Aimbot").c_str(), &g_Aimbotgay); /*Thanks to NotSimCraftLOL*/
					ImGui::Text(XorStr("Bone Target").c_str());
					//ImGui::SameLine();
					ImGui::Checkbox(XorStr("Head").c_str(), &Head);
					if (Head)
					{
						Neck = false;
						Chest = false;
						Globals::hitbox = 0;
					}

					ImGui::SameLine();
					ImGui::Checkbox(XorStr("Neck").c_str(), &Neck);
					if (Neck)
					{
						Head = false;
						Chest = false;
						Globals::hitbox = 1;
					}

					ImGui::SameLine();
					ImGui::Checkbox(XorStr("Chest").c_str(), &Chest);
					if (Chest)
					{
						Neck = false;
						Head = false;
						Globals::hitbox = 2;
					}

				}

				if (g_aimbot)
				{
					ImGui::Text("Smoothness");
					ImGui::SliderFloat(XorStr("    ").c_str(), &bA1mb0tSm00th1ngV4lue, .5, 30);
				}
				ImGui::Checkbox(XorStr("Enable Triggerbot (Disabled)").c_str(), &g_trigger);
				ImGui::Text("Max Aimbot Distance");
				ImGui::SliderInt(XorStr("").c_str(), &bA1mD1st4nce, 10, 280);

			}

			if (tabs == 1)
			{
				ImGui::Text("					    VISUALS");
				ImGui::Separator();
				ImGui::Text("\n");

				ImGui::Checkbox(XorStr("Skeleton ESP").c_str(), &g_esp_skeleton);
				ImGui::Checkbox(XorStr("3D Bounding Box").c_str(), &g_3d_box);
				ImGui::Checkbox(XorStr("Distance ESP").c_str(), &g_esp_distance);
				ImGui::Checkbox(XorStr("Equipped Weapon ESP").c_str(), &g_curweaponesp);
				ImGui::Checkbox(XorStr("Platform ESP").c_str(), &g_platform_esp);
				ImGui::Text(" ");

				ImGui::Text("Max ESP Render Distance");
				ImGui::SliderInt(XorStr(" ").c_str(), &bE5pD1st4nce, 10, 250);
			}

			if (tabs == 2)
			{
				ImGui::Text("					    EXPLOITS");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Checkbox(XorStr("PlayerFLY").c_str(), &g_playerfly);
				ImGui::Checkbox(XorStr("Boat Speed").c_str(), &g_boatrat);
				ImGui::Checkbox(XorStr("T-POSE").c_str(), &g_tpose);
				ImGui::Checkbox(XorStr("No Bloom").c_str(), &g_gun_tracers);
				ImGui::Checkbox(XorStr("No Spread").c_str(), &g_rapidaug);


			}

			if (tabs == 3)
			{
				ImGui::Text("                      Informations");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Text("\n Thanks for using Gloomy.cc");
			}



			ImGui::EndChild();


			ImGui::SetCursorPos(ImVec2(365, 10));
			ImGui::BeginChild("##menu2", ImVec2(235, 370), true);


			if (tabs == 0)
			{
				ImGui::Text("                         SETTINGS");
				ImGui::Separator();
				ImGui::Checkbox(XorStr("Draw Crosshair").c_str(), &g_crossh);
			}

			if (tabs == 1)
			{
				ImGui::Text("                         SETTINGS");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Checkbox(XorStr("Draw Circle FOV").c_str(), &g_circlefov);
				ImGui::Checkbox(XorStr("Watermark").c_str(), &g_watermark);

				if (g_fov || g_circlefov)
				{
					ImGui::SetCursorPos(ImVec2(140, 75));
					ImGui::Text("FOV");
					ImGui::SetCursorPos(ImVec2(140, 90));
					ImGui::SliderFloat(XorStr("    ").c_str(), &bA1mb0tF0VV4lue, 10, 1000);
				}



			}

			if (tabs == 2)
			{
				ImGui::Text("                      MORE EXPLOITS");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Checkbox("Fov Changer", &g_fovchanger);
				ImGui::Checkbox(XorStr("doublepump").c_str(), &g_doublepump);
				ImGui::Checkbox(XorStr("RocketLeauge [SHIFT]").c_str(), &g_RocketLeauge);
				ImGui::Checkbox(XorStr("NoColision").c_str(), &g_NoColision);
			}

			if (tabs == 3)
			{
				ImGui::Text("                      Version");
				ImGui::Separator();
				ImGui::Text("\n");
				ImGui::Text("\n You are using Version : 1.0");
				ImGui::Text("\n");
				ImGui::Text("\n You are using : Gloomy.cc");
			}

			ImGui::EndChild();



			ImGui::SetCursorPos(ImVec2(120, 391));
			ImGui::BeginChild("##menu3", ImVec2(480, 50), true);

			ImGui::SetCursorPos(ImVec2(6, 10));
			style.Colors[ImGuiCol_Text] = ImColor(79, 125, 249, 255);
			ImGui::Text("Gloomy.cc");

			ImGui::EndChild();


			ImGui::End();



		}
	}

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(g_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_window);
}

enum InjectedInputMouseOptions
{
	Absolute = 32768,
	HWheel = 4096,
	LeftDown = 2,
	LeftUp = 4,
	MiddleDown = 32,
	MiddleUp = 64,
	Move = 1,
	MoveNoCoalesce = 8192,
	None = 0,
	RightDown = 8,
	RightUp = 16,
	VirtualDesk = 16384,
	Wheel = 2048,
	XDown = 128,
	XUp = 256
};

typedef struct _InjectedInputMouseInfo
{
	int DeltaX;
	int DeltaY;
	unsigned int MouseData;
	InjectedInputMouseOptions MouseOptions;
	unsigned int TimeOffsetInMilliseconds;
	void* ExtraInfo;
} InjectedInputMouseInfo;

typedef bool (WINAPI* InjectMouseInput_t)(InjectedInputMouseInfo* inputs, int count);

InjectMouseInput_t InjectMouseInput;


/*If you sell Gloomy ima ratio you and rat you cuz i am pro tryharder nig.*/

int main() 
{
	InjectMouseInput = reinterpret_cast<InjectMouseInput_t>(GetProcAddress(LoadLibraryA("user32.dll"), "InjectMouseInput"));

	if (GlobalFindAtomA("innit??") == 0)
	{

		system("start https://discord.gg/tc7Q4fwv5p"); //Here you can start your paste discord server

		//system("tasklist")

       /*Sleep(5000);*/

	 /*Enable only if you want to [Makes it look cool]*/

		/*Taskilling processes to load the driver*/
		system(XorStr("taskkill /F /IM EpicGamesLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM EasyAntiCheatLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM BEService.exe").c_str());
		system(XorStr("taskkill /F /IM BattleEyeLauncher.exe").c_str());
		system(XorStr("taskkill /F /IM FortniteClient-Win64-Shipping.exe").c_str());
		system(XorStr("taskkill /F /IM FortniteLauncher.exe").c_str());

		system("cls");

		system("color b");

		std::cout << "Hello. Dear User, The cheat its loading ";

		Sleep(1500);

		VulnerableDriver::Init(); /*Init the drivers*/
		GlobalAddAtomA("innit??");
	}


	HWND Entryhwnd = NULL;

	while (Entryhwnd == NULL)
	{
		system("cls");
		system("color a");
		printf(XorStr("Driver Loaded\n").c_str());
		printf(XorStr("Please launch fortnite\n").c_str());
		Sleep(1);
		Entryhwnd = FindWindowA(XorStr("UnrealWindow").c_str(), XorStr("Fortnite  ").c_str());
		Sleep(1);
	}

	system(XorStr("cls").c_str());

	g_pid = get_fn_processid();

	if (!g_pid) {
		std::cout << XorStr("Could not find Fortnite Process.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	Drive.Init();

	EnumWindows(retreiveFortniteWindow, NULL);
	if (!fortnite_wnd) {
		std::cout << XorStr("Could not find Fortnite.\n");
		system(XorStr("pause").c_str());
		return 1;
	}

	g_base_address = getBaseAddress(g_pid);

	if (!pattern_uworld) pattern_uworld = find_signature(XorStr("\x48\x89\x05\x00\x00\x00\x00\x48\x8B\x4B\x78").c_str(), XorStr("xxx????xxxx").c_str());
	//if (!pattern_gnames) pattern_gnames = PatternScan<uint64_t>("48 83 EC 28 80 3D 9D 12 82 0B 00 74 0C 48 8D 05 98", 10);

	if (!g_base_address) {
		std::cout << XorStr("Could not get base address.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	setupWindow();
	if (!g_window) {
		std::cout << XorStr("Could not setup window.\n").c_str();
		system(XorStr("pause").c_str());
		return 1;
	}

	HWND ass = FindWindowA(nullptr, XorStr("Blade System Decoraytion").c_str());
	DWORD assid = 0;
	GetWindowThreadProcessId(ass, &assid);
	//wndhide::hide_window(assid, ass, true); // hide overlay using SetWindowDisplayAffinity

	HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CacheGame), nullptr, NULL, nullptr);
	CloseHandle(handle);

	while (!glfwWindowShouldClose(g_window))
	{
		handleKeyPresses();
		runRenderTick();
	}

	cleanupWindow();
	return 0;
}


void aimbot(float x, float y)
{
	float ScreenCenterX = (Globals::Width / 2);
	float ScreenCenterY = (Globals::Height / 2);
	int AimSpeed = bA1mb0tSm00th1ngV4lue;
	float TargetX = 0;
	float TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	InjectedInputMouseInfo info = { 0 };
	info.DeltaX = TargetX;
	info.DeltaY = TargetY;
	InjectMouseInput(&info, 1);

	return;
}



/*This update its from ac-1337 pls rat me uwu*/
