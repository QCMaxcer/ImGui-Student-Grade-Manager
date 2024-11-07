#pragma once
#include <string>
#include <condition_variable>
using namespace std;

struct Account {
	bool isLogin = false;
};

enum autoSaveIntervalType
{
	AUTO_SAVE_1_MIN,
	AUTO_SAVE_5_MIN,
	AUTO_SAVE_15_MIN,
	AUTO_SAVE_30_MIN,
	AUTO_SAVE_1_HOUR,
};

struct Setting {
	Account account;
	bool show_average_score = true;
	int maxHistorySize = 64;
	bool auto_save = false;
	//int auto_save_interval = 1;

	std::atomic<bool> autoSaveEnabled{ false };  // 控制是否启用自动保存
	int autoSaveIntervalType = 0;
	std::chrono::minutes autoSaveInterval;  // 自动保存的时间间隔

	bool unfocused_framerate_limit = true;
	bool framerate_limit = false;
	bool vsync = true;
	int framerate_limit_value = 30;
	wstring cur_file_path = L"";
	wstring cur_file_name = L"";
	wstring saveFileDialog();
	wstring openFileDialog();
	string getCurFileName();
	void saveSetting();
	void loadSetting();
	Setting();
};

extern Setting setting;