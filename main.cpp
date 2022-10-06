#include <iostream>
#include <conio.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Logger.h"
#include "stringhelpers.h"

using namespace std;

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

class PretzelProcess {
public:
  PretzelProcess(const string& title, const string& className);

  DWORD getProcessId() const;
  const string& exePath() const { return exePath_; }

  bool isRunning() const;
  bool launch();
  bool close();

  void playMusic() const;
  void stopMusic() const;

private:
  void sendInput(BYTE vKey, BYTE bScan, DWORD dwFlags) const;
  void pressKey(BYTE vKey, BYTE bScan) const;
  HWND getHwnd() const;
  string getExePath() const;

  const string title_;
  const string className_;
  const string exePath_;
};

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

PretzelProcess::PretzelProcess(const string& title, const string& className)
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

string PretzelProcess::getExePath() const {
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
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

class FileWatcher {
public:
  FileWatcher(const string& filePath) : filePath_(filePath) { };

  const string& filePath() { return filePath_; }
  bool waitForFileChange();

private:
  const string filePath_;
};

bool FileWatcher::waitForFileChange() {
  char pDrive[4];
  char pDir[_MAX_DIR];
  char pFile[_MAX_FNAME];

  if (errno_t err = _splitpath_s(filePath_.c_str(), pDrive, 4, pDir, _MAX_DIR, pFile, _MAX_FNAME, NULL, NULL))
    return false;

  const string dirPath = __("%s%s", pDrive, pDir);

  while (true) {
    const HANDLE hDir = CreateFileA(dirPath.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS,
      NULL
    );

    unsigned char pBuf[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};

    DWORD bytesReturned = 0;

    if (ReadDirectoryChangesW(hDir, pBuf, sizeof(pBuf), FALSE, FILE_NOTIFY_CHANGE_ATTRIBUTES, &bytesReturned, NULL, NULL) == 0) {
      Logger::logError("ERROR: ReadDirectoryChangesW function failed.\n");
      return false;
    }

    if (bytesReturned == 0) {
      Logger::logError("ERROR: Cannot get file info.\n");
      return false;
    }

    CloseHandle(hDir);

    const FILE_NOTIFY_INFORMATION* pFileInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(pBuf);
    char pMbFileName[_MAX_PATH];

    const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pFileInfo->FileName, -1, pMbFileName,
        sizeof(pMbFileName), NULL, NULL);

    const string notifiedFilePath = dirPath + pMbFileName;

    if (notifiedFilePath == filePath_)
      return true;
  }
}

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

class PretzelWatcherApp {
public:
  PretzelWatcherApp(const string& songTxtFilePath, DWORD restartIntervalMs);

  int run();

private:
  HANDLE hThread_{INVALID_HANDLE_VALUE};
  bool doWork_{false};

  void startWorkerThread();
  void stopWorkerThread();
  bool workerThreadIsRunning() const;
  static DWORD WINAPI workerThread(LPVOID lpParam);

  FileWatcher watcher_;
  const DWORD restartIntervalMs_;
};

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

PretzelWatcherApp::PretzelWatcherApp(const string& songTxtFilePath, DWORD restartIntervalMs)
    : watcher_(songTxtFilePath), restartIntervalMs_(restartIntervalMs) {

}

int PretzelWatcherApp::run() {
  printf("Track Info File: %s\n", watcher_.filePath().c_str());
  printf("Restart interval: %d minutes\n\n", restartIntervalMs_ / (1000 * 60));

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

    return 2;
  }

  Logger::logSuccess("Pretzel is running at '%s' with process ID: 0x%X\n", pretzel.exePath().c_str(),
      pretzel.getProcessId());

  DWORD startTimeMs = GetTickCount();

  while (pThis->doWork_) {
    DWORD curTimeMs = GetTickCount();

    if (curTimeMs - startTimeMs < pThis->restartIntervalMs_) {
      curTimeMs = GetTickCount();
      Sleep(1000);

      continue;
    }

    Logger::logWarning("Waiting for end of current song before restart...\n");

    pThis->watcher_.waitForFileChange();

    Logger::logSuccess("Song finished. Now restarting Pretzel app...\n");

    pretzel.stopMusic();
    pretzel.close();

    if (!pretzel.launch()) {
      Logger::logError("Failed to relaunch Pretzel app! Quitting...\n");
      return 3;
    }

    pThis->watcher_.waitForFileChange();

    Logger::logSuccess("Pretzel running. Start playback...\n");

    pretzel.playMusic();
    startTimeMs = GetTickCount();
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

int main(int argc, char* ppArgv[]) {
  printf("--- Pretzel Watcher v1.0 ---\n\n");

  if (argc != 3) {
    printf("Usage: PretzelWatcher.exe <Track info file> <Restart interval minutes>\n");
    printf("Example: PretzelWatcher.exe C:\\Users\\Public\\Documents\\current_song.txt 120\n");

    return 1;
  }

  const string songTxtFilePath = ppArgv[1];
  const DWORD restartIntervalMs = atoi(ppArgv[2]) * 1000 * 60;

  PretzelWatcherApp app(songTxtFilePath, restartIntervalMs);

  return app.run();
}
