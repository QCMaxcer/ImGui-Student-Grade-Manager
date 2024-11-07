#pragma once

#include "ImGui/imgui.h"
#include <map>
#include <vector>

using namespace std;
class Hotkey
{
public:
	map<int, ImGuiKey> hotKeyMap;
	//vector<ImGuiKey> hotKeyVec;

	void regHotkey(int id, ImGuiKey imguiKey);

	void detHotkey();


};