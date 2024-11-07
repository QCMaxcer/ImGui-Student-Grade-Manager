#pragma once

#include <Windows.h>
#include "ImGui\imgui.h"
#include <list>
using namespace std;
struct Client {
	HWND hwnd;
	list<HWND> hwndList;
	bool next_top_most = false;
	bool show_settings_window = false;
	bool show_about_window = false;
	bool show_login_window = false;
	bool show_add_student_window = false;
	bool show_modify_student_window = false;
	bool show_add_course_window = false;
	int show_save_prompt_window = 0;
	int show_load_error_window = 0;
	//bool show_delete_course_window = false;
	bool show_modify_course_window = false;
	bool show_clearall_warning_window = false;
	ImVec2 screen_size = ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));  // 屏幕大小
	ImVec4 init_window_size_pos = ImVec4(1024, 768, (screen_size.x - 1024) / 2, (screen_size.y - 768) / 2); // 初始目标窗口大小及位置
	ImVec4 window_size_pos = ImVec4(1.0f, 1.0f, screen_size.x / 2, screen_size.y / 2); // 窗口大小及位置 (一开始窗口大小为1，位置为屏幕中心)
	ImVec4 original_window_size_pos; // 记录最大化时原窗口大小及位置
	ImVec4 current_window_size_pos; // 记录当前窗口大小及位置
	bool setpos = false;
	bool isMaximize;
	bool isMaximizeAnime;
	void setWindowSizePos(const ImVec4 &size_pos);
	void setWindowContentSize(ImGuiWindowFlags& window_flags);
	void minimizeWindow();
	void minimizeAllWindows();
	void maximizeWindow();

	//void dragCheck();
	void dragRestoreWindow();
	void maximizeAnime(float);
	void launchAnime(float);

	void addWindow(HWND hwnd);
	void removeWindow(HWND hwnd);

	void setNextWindowTopMost();

	int getScreenRefreshRate();
};

extern Client client;
