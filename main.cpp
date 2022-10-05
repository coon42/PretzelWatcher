#include <iostream>
#include <conio.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

class PretzelProcess {
public:
  PretzelProcess(const string& title, const string& className);

  DWORD getProcessId() const;

  bool isRunning() const;
  bool launch();
  bool close();

  void playMusic() const;
  void stopMusic() const;

private:
  void sendInput(BYTE vKey, BYTE bScan, DWORD dwFlags) const;
  void pressKey(BYTE vKey, BYTE bScan) const;
  HWND getHwnd() const;

  const string title_;
  const string className_;
};

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

PretzelProcess::PretzelProcess(const string& title, const string& className)
    : title_(title), className_(className) {

}

HWND PretzelProcess::getHwnd() const {
  return FindWindowA(className_.c_str(), title_.c_str());
}

DWORD PretzelProcess::getProcessId() const {
  const HWND hWnd = getHwnd();

  if (hWnd == INVALID_HANDLE_VALUE)
    return 0;

  DWORD processId = 0;
  GetWindowThreadProcessId(hWnd, &processId);

  return processId;
}

bool PretzelProcess::isRunning() const {
  const DWORD processId = getProcessId();

  if (!processId)
    return false;

  const HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

  if (!hProc)
    return false;

  return true;
}

bool PretzelProcess::launch() {
  return true;
}

bool PretzelProcess::close() {
  return true;
}

void PretzelProcess::sendInput(BYTE vKey, BYTE bScan, DWORD dwFlags) const {
  INPUT ip{0};
  ip.type = INPUT_KEYBOARD;
  ip.ki.wScan = bScan;
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;
  ip.ki.wVk = vKey;
  ip.ki.dwFlags = dwFlags;
  SendInput(1, &ip, sizeof(INPUT));
}

void PretzelProcess::pressKey(BYTE vKey, BYTE bScan) const {
  sendInput(vKey, bScan, 0);
  sendInput(vKey, bScan, KEYEVENTF_KEYUP);
}

void PretzelProcess::playMusic() const {
  Logger::log("Start playing music...\n");

  pressKey(VK_MEDIA_PLAY_PAUSE, DIK_PLAYPAUSE);
}

void PretzelProcess::stopMusic() const {
  Logger::log("Stop playing music...\n");

  pressKey(VK_MEDIA_STOP, DIK_MEDIASTOP);
}

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

class PretzelWatcherApp {
public:
  int run();

private:
  HANDLE hThread_{INVALID_HANDLE_VALUE};
  bool doWork_{false};

  void startWorkerThread();
  void stopWorkerThread();
  bool workerThreadIsRunning() const;
  static DWORD WINAPI workerThread(LPVOID lpParam);
};

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

int PretzelWatcherApp::run() {
  printf("--- Pretzel Watcher ---\n\n");
  printf("Press 'q' to quit.\n\n");

  startWorkerThread();

  while (workerThreadIsRunning()) {
    if (_kbhit()) {
      char c = _getch();

      if (c == 'q')
        stopWorkerThread();
    }

    Sleep(100);
  }

  return 0;
}

void PretzelWatcherApp::startWorkerThread() {
  Logger::log("Starting worker thread...\n");

  DWORD threadId;
  hThread_ = CreateThread(0, 0, workerThread, this, 0, &threadId);

  if (hThread_)
    doWork_ = true;
}

void PretzelWatcherApp::stopWorkerThread() {
  Logger::log("Stopping worker thread...\n");

  doWork_ = false;

  if (hThread_ != INVALID_HANDLE_VALUE) {
    WaitForSingleObject(hThread_, INFINITE);

    CloseHandle(hThread_);
    hThread_ = INVALID_HANDLE_VALUE;
  }

  Logger::logSuccess("Worker thread stopped.\n");
}

bool PretzelWatcherApp::workerThreadIsRunning() const {
  if (hThread_ == INVALID_HANDLE_VALUE)
    return false;

  if (WaitForSingleObject(hThread_, 0) == WAIT_OBJECT_0)
    return false;

  return true;
}

DWORD WINAPI PretzelWatcherApp::workerThread(LPVOID lpParam) {
  PretzelWatcherApp* pThis = static_cast<PretzelWatcherApp*>(lpParam);

  Logger::logSuccess("Worker thread started.\n");

  PretzelProcess pretzel("Pretzel Rocks", "Chrome_WidgetWin_1");

  Logger::log("Looking for Pretzel process...\n");

  if (!pretzel.isRunning()) {
    Logger::logError("Pretzel is not running. Quitting...\n");

    return 1;
  }

  Logger::logSuccess("Pretzel is running with process ID: 0x%X\n", pretzel.getProcessId());

  while (pThis->doWork_)
    Sleep(1000);

  return 0;
}

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

int main(int argc, char* ppArgv[]) {
  PretzelWatcherApp app;

  return app.run();
}
