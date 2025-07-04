/*
========== Super simple timer addon
========== Modified fps_limit_addon from the reshade examples repo

In order to build, add a RESHADE_PATH env variable.
*/

#include <imgui.h>
#include <reshade.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <windowsx.h>
#include <iostream>
typedef unsigned int uint;

static std::chrono::duration<double> elapsed;
static std::chrono::time_point<std::chrono::steady_clock> start;

static char start_bind = 'O';
static char restart_bind = 'P';
static char split_bind = 'L';

static std::vector<std::chrono::time_point<std::chrono::steady_clock>> splits = {};

static bool has_started = false;
static bool first_start = true;

using namespace std::chrono;
static void on_present(reshade::api::command_queue*, reshade::api::swapchain*, const reshade::api::rect*, const reshade::api::rect*, uint32_t, const reshade::api::rect*)
{
	const auto now = steady_clock::now();
	if (first_start)
	{
		first_start = false;
		start = now;
	}
	if (!has_started && ((GetKeyState(start_bind) & 0x8000)))
	{
		has_started = true;
		start = now;
		splits.push_back(start);
	}
	if (has_started && (GetKeyState(restart_bind) & 0x8000))
	{
		// has_started remains
		start = now;
		splits.clear();
		splits.push_back(now);
	}

	elapsed = now - start;

	if (has_started && (GetAsyncKeyState(split_bind) & 0x0001))
	{
		splits.push_back(now);
	}
}

static void draw_settings(reshade::api::effect_runtime*)
{

}

static void on_overlay(reshade::api::effect_runtime*)
{
	ImGui::SetNextWindowBgAlpha(255);
	ImGui::SetNextWindowPos(ImVec2(10, 30));
	ImGui::Begin("Timers", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus);

	for (int i = 1; i < splits.size(); i++)
	{
		const duration<double> split_duration = splits.at(i) - splits.at(i - 1);
		ImGui::Text("Split %d %02d:%02d:%02d", i, duration_cast<hours>(split_duration), duration_cast<minutes>(split_duration).count() % 60, duration_cast<seconds>(split_duration).count() % 60);
	}
	// Total time.
	ImGui::Text("%02d:%02d:%02d", duration_cast<hours>(elapsed).count(), duration_cast<minutes>(elapsed) % 60, duration_cast<seconds>(elapsed) % 60);
}

extern "C" __declspec(dllexport) const char* NAME = "Simple Timer";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "Super cool timer. A basic ReShade addon meant as a companion to all sorts of speedrunners! Developed, albeit probably not maintained by BFB. ";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		if (!reshade::register_addon(hModule))
			return FALSE;
		reshade::register_event<reshade::addon_event::present>(on_present);
		reshade::register_overlay(nullptr, draw_settings);
		reshade::register_event<reshade::addon_event::reshade_overlay>(on_overlay);
		break;
	case DLL_PROCESS_DETACH:
		reshade::unregister_addon(hModule);
		break;
	}

	return TRUE;
}
