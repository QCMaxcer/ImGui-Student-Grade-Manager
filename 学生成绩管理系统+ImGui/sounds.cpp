#include <windows.h>
#include <thread>
#include <string>
#include <iostream>

void setVolume(DWORD volume, std::string name) {
    // Set the volume (0-1000 scale for simplicity)
    std::string command = "setaudio ";
    command += name + " volume to " + std::to_string(volume);
    mciSendStringA(command.c_str(), NULL, 0, NULL);
}

void promptSaveSounds(void* p) {
    if (0) return;
    //float volume = manager.notificationVolume * 10;
    mciSendStringA("open \"C:\\Windows\\Media\\Windows Notify Calendar.wav\" type mpegvideo alias S", NULL, 0, NULL);
    setVolume(90, "S");
    mciSendStringA("play S", NULL, 0, NULL);
    Sleep(2000);
    mciSendStringA("stop S", NULL, 0, NULL);
    mciSendStringA("close S", NULL, 0, NULL);
    //PlaySound(TEXT("C:\\Windows\\Media\\Windows Notify Calendar.wav"), NULL, SND_FILENAME | SND_ASYNC);
    _endthread();
}

void promptErrorSounds(void* p) {
    if (0) return;
    //float volume = manager.notificationVolume * 10;
    mciSendStringA("open \"C:\\Windows\\Media\\Windows User Account Control.wav\" type mpegvideo alias E", NULL, 0, NULL);
    setVolume(90, "E");
    mciSendStringA("play E", NULL, 0, NULL);
    Sleep(2000);
    mciSendStringA("stop E", NULL, 0, NULL);
    mciSendStringA("close E", NULL, 0, NULL);
    //PlaySound(TEXT("C:\\Windows\\Media\\Windows Notify Calendar.wav"), NULL, SND_FILENAME | SND_ASYNC);
    _endthread();
}