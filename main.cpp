#include <iostream>
#include <conio.h>

#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

class PretzelProcess {
public:
  PretzelProcess(const string& title, const string& className);

  HWND getHwnd() const;
  DWORD getProcessId() const;

  bool isRunning() const;
  bool launch();
  bool close();

private:
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
  static DWORD WINAPI workerThread(LPVOID lpParam);
};

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

int PretzelWatcherApp::run() {
  printf("--- Pretzel Watcher ---\n\n");
  printf("Press 'q' to quit.\n\n");

  startWorkerThread();

  bool finished = false;

  while (!finished) {
    if (_kbhit()) {
      char c = _getch();

      if (c == 'q')
        finished = true;
    }

    Sleep(100);
  }

  stopWorkerThread();

  return 0;
}

void PretzelWatcherApp::startWorkerThread() {
  Logger::log("Starting worker thread...\n");

  DWORD threadId;
  hThread_ = CreateThread(0, 0, workerThread, this, 0, &threadId);
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

DWORD WINAPI PretzelWatcherApp::workerThread(LPVOID lpParam) {
  PretzelWatcherApp* pThis = static_cast<PretzelWatcherApp*>(lpParam);

  pThis->doWork_ = true;

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
