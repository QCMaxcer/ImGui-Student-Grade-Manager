#include "client.h"
#include "ImGui\imgui_internal.h"
#include <dwmapi.h>
#include <windows.h>
#include <cmath>
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")

using namespace std;

int Client::getScreenRefreshRate()
{
    DEVMODE dm;
    dm.dmSize = sizeof(DEVMODE);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
    {
        return dm.dmDisplayFrequency;  // ˢ���ʣ�֡�ʣ�
    }
    return 0;  // ��ȡʧ��
}

void Client::setWindowSizePos(const ImVec4& size_pos)
{
    ImGui::SetNextWindowSize({ size_pos.x,size_pos.y });
    ImGui::SetNextWindowPos({ size_pos.z,size_pos.w });
}

void Client::setWindowContentSize(ImGuiWindowFlags &window_flags)
{
    if (isMaximize)
    {
        //dragCheck();
        //ImGui::SetNextWindowSizeConstraints(screen_size, screen_size); //���ô��ڴ�С����Ϊ��Ļ��С
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
    }
    else
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(512, 0), ImVec2(FLT_MAX, FLT_MAX));
    }
}

void Client::minimizeWindow()
{
    BOOL enable = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &enable, sizeof(enable));

    // ���л�������
    AnimateWindow(hwnd, 300, AW_HIDE | AW_BLEND);  // ʹ�ý���Ч��
    ShowWindow(hwnd, SW_MINIMIZE);
}

void Client::minimizeAllWindows()
{
    for (auto hwnd : hwndList)
    {
        BOOL enable = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &enable, sizeof(enable));

        // ���л�������
        AnimateWindow(hwnd, 300, AW_HIDE | AW_BLEND);  // ʹ�ý���Ч��
        ShowWindow(hwnd, SW_MINIMIZE);
    }
}

void Client::maximizeWindow()
{
    ImVec2 window_size = ImGui::GetWindowSize(); // ��ȡ��ǰ���ڴ�С
    ImVec2 window_pos = ImGui::GetWindowPos(); // ��ȡ��ǰ����λ��
    window_size_pos = { window_size.x,window_size.y,window_pos.x,window_pos.y };
    isMaximizeAnime = true;
    if (!isMaximize) // ����׼�����
    {
        ImVec2 original_window_size = ImGui::GetWindowSize(); // ��¼��ǰ���ڴ�С
        ImVec2 original_window_pos = ImGui::GetWindowPos(); // ��¼��ǰ����λ��
        original_window_size_pos = { original_window_size.x,original_window_size.y,original_window_pos.x,original_window_pos.y };
        isMaximize = true;
        return;
    }
    //Բ��
    DWM_WINDOW_CORNER_PREFERENCE dwmPreference = DWM_WINDOW_CORNER_PREFERENCE::DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &dwmPreference, sizeof(dwmPreference));
    isMaximize = false;
}



//void Client::dragCheck()
//{
//    if (current_window_size_pos.z != 0 || current_window_size_pos.w != 0)
//    {
//        maximizeWindow();
//    }
//}

void Client::maximizeAnime(float speed = 1.0f / 240.0f)
{
    if (isMaximize) //�Ŵ󶯻�
    {
        window_size_pos = ImLerp(window_size_pos, { screen_size.x,screen_size.y,0,0 }, speed);
        if (abs(screen_size.x - window_size_pos.x) < 1.0f)
        {
            setWindowSizePos({ screen_size.x,screen_size.y,0,0 });
            // ȡ��Բ��
            DWM_WINDOW_CORNER_PREFERENCE dwmPreference = DWM_WINDOW_CORNER_PREFERENCE::DWMWCP_DEFAULT;
            DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &dwmPreference, sizeof(dwmPreference));
            //ShowWindow(hwnd, SW_MAXIMIZE);
            isMaximizeAnime = false;
            return;
        }
    }
    else //��С����
    {
        window_size_pos = ImLerp(window_size_pos, original_window_size_pos, speed);
        if (abs(window_size_pos.x - original_window_size_pos.x) < 1.0f)
        {
            setWindowSizePos(original_window_size_pos);
            isMaximizeAnime = false;
            return;
        }
    }
    setWindowSizePos(window_size_pos);
}

void Client::launchAnime(float speed = 1.0f / 240.0f)
{
    if (setpos)
    {
        return;
    }

    //window_size_pos.x += speed * 50, window_size_pos.y += speed * 50;
    window_size_pos = ImLerp(window_size_pos, init_window_size_pos, speed);
    //window_size_pos = ImVec2((screen_size.x - window_size.x) / 2, (screen_size.y - window_size.y) / 2);
    if (init_window_size_pos.x - window_size_pos.x < 1.0f && init_window_size_pos.y - window_size_pos.y < 1.0f)
    {
        //ImGui::SetNextWindowPos(target_window_pos);
        setWindowSizePos(window_size_pos);
        setpos = true;
    }
    setWindowSizePos(window_size_pos);
}

// ���洰�ھ��
void Client::addWindow(HWND hwnd) {

    hwndList.push_back(hwnd);
    if (next_top_most)
    {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        next_top_most = false;
    }
    //Բ��+�ö�
    DWM_WINDOW_CORNER_PREFERENCE dwmPreference = DWM_WINDOW_CORNER_PREFERENCE::DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &dwmPreference, sizeof(dwmPreference));
}

// �Ƴ����ھ��
void Client::removeWindow(HWND hwnd) {
    hwndList.remove(hwnd);
}

// �����ö�
void Client::setNextWindowTopMost() {
    next_top_most = true;
    //SetWindowPos(hwndList.back(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}
