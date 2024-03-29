#include <conio.h>

#include "logger.h"
#include "pretzelprocess.h"
#include "stringhelpers.h"

using namespace std;

const std::string versionString = "v1.2";

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

  const string& songTxtFilePath_;
  const DWORD restartIntervalMs_;
  const DWORD maxWaitForTrackFinishMs_ = 15 * 60 * 1000; // 15 minutes
};

//--------------------------------------------------------------------------------------------------------------
// PretzelWatcherApp
//--------------------------------------------------------------------------------------------------------------

PretzelWatcherApp::PretzelWatcherApp(const string& songTxtFilePath, DWORD restartIntervalMs)
    : songTxtFilePath_(songTxtFilePath), restartIntervalMs_(restartIntervalMs) {

}

int PretzelWatcherApp::run() {
  printf("Track Info File: %s\n", songTxtFilePath_.c_str());
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

  PretzelProcess pretzel("Pretzel Rocks", "Chrome_WidgetWin_1", pThis->songTxtFilePath_);

  Logger::log("Looking for Pretzel process...\n");

  if (!pretzel.isRunning()) {
    Logger::logError("Pretzel is not running. Quitting...\n");

    return 2;
  }

  Logger::logSuccess("Pretzel is running at '%s' with process ID: 0x%X\n", pretzel.exePath().c_str(),
      pretzel.getProcessId());

  ULONGLONG startTimeMs = GetTickCount64();
  ULONGLONG restartInitTimeMs = 0;
  bool pendingRestart = false;

  while (pThis->doWork_) {
    Sleep(1000);

    const ULONGLONG curTimeMs = GetTickCount64();
    const ULONGLONG diffMs = curTimeMs - startTimeMs;

    if (diffMs < pThis->restartIntervalMs_) {

      const ULONGLONG timeRemainingS = (pThis->restartIntervalMs_ - diffMs) / 1000;

      const ULONGLONG h = timeRemainingS / (60 * 60);
      const ULONGLONG m = (timeRemainingS / 60) % 60;
      const ULONGLONG s = timeRemainingS % 60;

      SetConsoleTitleA(__("Pretzel Watcher %s - Next restart in: %02dh %02dm %02ds", versionString.c_str(), h, m, s));
      continue;
    }

    if (!pendingRestart) {
      restartInitTimeMs = GetTickCount64();
      pendingRestart = true;
    }

    const ULONGLONG timeRemainingS = (pThis->maxWaitForTrackFinishMs_ - (GetTickCount64() - restartInitTimeMs)) / 1000;

    const ULONGLONG h = timeRemainingS / (60 * 60);
    const ULONGLONG m = (timeRemainingS / 60) % 60;
    const ULONGLONG s = timeRemainingS % 60;

    Logger::logWarning("Waiting for end of current song before restart (Timeout in: %02dh %02dm %02ds)\r", h, m, s);

    if (timeRemainingS <= 0)
      Logger::logWarning("\nTrack didn't finish in time. Forcing restart.\n");
    else if (!pretzel.watcher().peekFileChange())
      continue;
    else
      Logger::logSuccess("\nSong finished. Now restarting Pretzel app...\n");

    do {
      if (!pretzel.close()) {
        Logger::logError("Unable to stop pretzel app! Quitting...\n");
        return 10;
      }

      if (!pretzel.launch()) {
        Logger::logError("Failed to relaunch Pretzel app! Quitting...\n");
        return 3;
      }

      if (!pretzel.playMusic()) {
        Logger::logWarning("Pretzel didn't seem to start up properly. Restarting again...\n");
        continue;
      }

      break;
    } while (true);

    Logger::logSuccess("Pretzel running.\n");

    pendingRestart = false;

    startTimeMs = GetTickCount64();
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------------------
// main
//--------------------------------------------------------------------------------------------------------------

int main(int argc, char* ppArgv[]) {
  printf("--- Pretzel Watcher %s ---\n\n", versionString.c_str());

  if (argc != 3) {
    printf("Usage: PretzelWatcher.exe <Track info file> <Restart interval minutes>\n");
    printf("Example: PretzelWatcher.exe C:\\Users\\Public\\Documents\\current_song.txt 120\n");

    return 1;
  }

  const string songTxtFilePath = ppArgv[1];
  const DWORD restartIntervalMs = atoi(ppArgv[2]) * 1000 * 60;

  PretzelWatcherApp app(songTxtFilePath, restartIntervalMs);

  int error = app.run();

  _getch();

  return error;
}
