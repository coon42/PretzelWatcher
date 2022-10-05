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
};

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

int PretzelWatcherApp::run() {
  printf("--- Pretzel Watcher ---\n\n");

  bool finished = false;

  printf("Press 'q' to quit.\n");

  while (!finished) {
    if (_kbhit()) {
      char c = _getch();

      if (c == 'q')
        finished = true;
    }

    Sleep(100);
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

int main(int argc, char* ppArgv[]) {
  PretzelWatcherApp app;

  return app.run();
}
