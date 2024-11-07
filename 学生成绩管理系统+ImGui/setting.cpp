#define _CRT_SECURE_NO_WARNINGS
#include "setting.h"
#include <windows.h>
#include <shobjidl.h>  // ���� IFileDialog
#include <string>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include "autoSave.h"
using namespace std;

Setting::Setting()
{
    loadSetting();
    switch (autoSaveIntervalType) {
    case AUTO_SAVE_1_MIN:
        autoSaveInterval = std::chrono::minutes(1);
        break;
    case AUTO_SAVE_5_MIN:
        autoSaveInterval = std::chrono::minutes(5);
        break;
    case AUTO_SAVE_15_MIN:
        autoSaveInterval = std::chrono::minutes(15);
        break;
    case AUTO_SAVE_30_MIN:
        autoSaveInterval = std::chrono::minutes(30);
        break;
    case AUTO_SAVE_1_HOUR:
        autoSaveInterval = std::chrono::minutes(60);
        break;
    }
}
wstring Setting::saveFileDialog()
{
    wstring filePath;

    // ��ʼ�� COM ��
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileSaveDialog* pFileSaveDialog = nullptr;

        // �����ļ�����Ի������
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSaveDialog));

        if (SUCCEEDED(hr)) {
            // �����ļ����͹�����
            COMDLG_FILTERSPEC fileTypes[] = {
                { L"Text Document (*.txt)", L"*.txt" },
                { L"All Files (*.*)", L"*.*" }
            };
            pFileSaveDialog->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);

            // ����Ĭ���ļ��������� (�� 0 ��ʾ .txt)
            pFileSaveDialog->SetFileTypeIndex(1);

            // ����Ĭ���ļ���
            pFileSaveDialog->SetFileName(L"Untitled.txt");

            // ��ʾ�����ļ��Ի���
            hr = pFileSaveDialog->Show(NULL);
            if (SUCCEEDED(hr)) {
                // ��ȡ�û�ѡ����ļ�·��
                IShellItem* pItem;
                hr = pFileSaveDialog->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // ��·��ת��Ϊ std::string
                    if (SUCCEEDED(hr)) {
                        //char path[MAX_PATH];
                        //wcstombs(path, pszFilePath, MAX_PATH);

                        filePath = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileSaveDialog->Release();
        }
        CoUninitialize();
    }
    return filePath;
}

wstring Setting::openFileDialog() {
    //string filePath;
    wstring filePath;
    // ��ʼ�� COM ��
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog* pFileOpenDialog = nullptr;

        // �����ļ��Ի������
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpenDialog));

        if (SUCCEEDED(hr)) {
            // �����ļ����͹�����
            COMDLG_FILTERSPEC fileTypes[] = {
                { L"Text Document (*.txt)", L"*.txt" }, // ����ʾ .txt �ļ�
                { L"All Files (*.*)", L"*.*" }           // ��ѡ����ʾ�����ļ�
            };
            pFileOpenDialog->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
            pFileOpenDialog->SetFileTypeIndex(1); // Ĭ��ѡ�� .txt ����

            // ��ʾ�ļ��򿪶Ի���
            hr = pFileOpenDialog->Show(NULL);
            if (SUCCEEDED(hr)) {
                // ��ȡ�û�ѡ����ļ�·��
                IShellItem* pItem;
                hr = pFileOpenDialog->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // ��·��ת��Ϊ std::string
                    if (SUCCEEDED(hr)) {
                        /*char path[MAX_PATH] =  "���";
                        wcstombs(path, pszFilePath, MAX_PATH);*/
                        filePath = pszFilePath;
                        //std::string path(wstr.begin(), wstr.end());
                        //filePath = path;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpenDialog->Release();
        }
        CoUninitialize();
    }
    //wcout << filePath << endl;
    return filePath;
}

void Setting::saveSetting()
{
    //�������������ͬһ�ļ�����
    std::string filePath = "setting.ini";
    //�������
    //locale& loc = locale::global(locale(locale(), "", LC_CTYPE));
    std::wofstream ofs(filePath);
    ofs.imbue(std::locale("chs"));
    ofs << account.isLogin << std::endl;
    ofs << show_average_score << std::endl;
    ofs << maxHistorySize << std::endl;
    ofs << autoSaveEnabled << std::endl;
    ofs << autoSaveIntervalType << std::endl;
    //ofs << autoSaveInterval.count() << std::endl;
    ofs << unfocused_framerate_limit << std::endl;
    ofs << framerate_limit << std::endl;
    ofs << vsync << std::endl;
    ofs << framerate_limit_value << std::endl;
    ofs << cur_file_path << std::endl;
    ofs << cur_file_name << std::endl;

    ofs.close();
}

void Setting::loadSetting()
{
    //��ȡ�����ļ�
    std::string filePath = "setting.ini";
    std::wifstream ifs(filePath);
    ifs.imbue(std::locale("chs"));
    ifs >> account.isLogin;
    ifs >> show_average_score;
    ifs >> maxHistorySize;
    bool auto_save_enabled;
    //int minutes;
    ifs >> auto_save_enabled;
    autoSaveEnabled = auto_save_enabled;
    //ifs >> minutes;
    ifs >> autoSaveIntervalType;

    //autoSaveInterval = std::chrono::minutes(minutes);
    ifs >> unfocused_framerate_limit;
    ifs >> framerate_limit;
    ifs >> vsync;
    ifs >> framerate_limit_value;
    ifs >> cur_file_path;
    ifs >> cur_file_name;

    ifs.close();
}

std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

string Setting::getCurFileName()
{
    return wstringToString(cur_file_name);
}
