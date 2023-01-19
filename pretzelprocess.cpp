#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Logger.h"

#include "pretzelprocess.h"

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

PretzelProcess::PretzelProcess(const std::string& title, const std::string& className)
    : title_(title), className_(className), exePath_(getExePath()) {

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

std::string PretzelProcess::getExePath() const {
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, getProcessId());

  if (!hProc)
    return "";

  char pExePath[MAX_PATH];
  DWORD bufferSize = sizeof(pExePath);

  if (QueryFullProcessImageNameA(hProc, 0, pExePath, &bufferSize) == 0)
    return "";

  CloseHandle(hProc);

  return pExePath;
}

bool PretzelProcess::isRunning() const {
  const DWORD processId = getProcessId();

  if (!processId)
    return false;

  return true;
}

bool PretzelProcess::launch() {
  STARTUPINFOA si;
  GetStartupInfoA(&si);

  PROCESS_INFORMATION pi;
  char* pCmdLine = const_cast<char*>(static_cast<const char*>(exePath_.c_str()));

  if (CreateProcessA(NULL, pCmdLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == 0)
    return false;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return true;
}

bool PretzelProcess::close() {
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, getProcessId());

  if (!hProc)
    return false;

  if (TerminateProcess(hProc, 0) == 0) {
    CloseHandle(hProc);
    return false;
  }

  CloseHandle(hProc);

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

void PretzelProcess::pressKeyGlobal(BYTE vKey, BYTE bScan) const {
  sendInput(vKey, bScan, 0);
  sendInput(vKey, bScan, KEYEVENTF_KEYUP);
}

void PretzelProcess::pressKeyProcessLocal(BYTE vKey) const {
  const HWND hWnd = getHwnd();

  SendMessage(hWnd, WM_KEYDOWN, vKey, 0);
  SendMessage(hWnd, WM_KEYUP, vKey, 0);
}

void PretzelProcess::playMusic() const {
  Logger::log("Start playing music...\n");

  pressKeyGlobal(VK_MEDIA_PLAY_PAUSE, DIK_PLAYPAUSE);
}

void PretzelProcess::stopMusic() const {
  Logger::log("Stop playing music...\n");

  pressKeyGlobal(DIK_MEDIASTOP, DIK_MEDIASTOP);
}
