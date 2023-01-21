#ifndef PRETZELPROCESS_H
#define PRETZELPROCESS_H

#include <string>
#include <windows.h>

#include "filewatcher.h"

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

class PretzelProcess {
public:
  PretzelProcess(const std::string& title, const std::string& className, const std::string& songTxtFilePath);

  DWORD getProcessId() const;
  const std::string& exePath() const { return exePath_; }
  FileWatcher& watcher()             { return watcher_; }

  bool isRunning() const;
  bool launch();
  bool close();

  void playMusic() const;
  void stopMusic() const;

private:
  void sendInput(BYTE vKey, BYTE bScan, DWORD dwFlags) const;
  void pressKeyGlobal(BYTE vKey, BYTE bScan) const;
  void pressKeyProcessLocal(BYTE vKey) const;
  HWND getHwnd() const;
  std::string getExePath() const;
  bool isPlaying() const;

  const std::string title_;
  const std::string className_;
  const std::string exePath_;

  FileWatcher watcher_;
};

#endif // PRETZELPROCESS_H
