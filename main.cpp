#include <conio.h>

#include "logger.h"
#include "pretzelprocess.h"
#include "filewatcher.h"

using namespace std;

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

  ULONGLONG startTimeMs = GetTickCount64();

  while (pThis->doWork_) {
    const ULONGLONG curTimeMs = GetTickCount64();
    const ULONGLONG diffMs = curTimeMs - startTimeMs;

    if (diffMs < pThis->restartIntervalMs_) {
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
    startTimeMs = GetTickCount64();
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
