#ifndef PRETZELPROCESS_H
#define PRETZELPROCESS_H

#include <string>
#include <windows.h>

//--------------------------------------------------------------------------------------------------------------
// PretzelProcess
//--------------------------------------------------------------------------------------------------------------

class PretzelProcess {
public:
  PretzelProcess(const std::string& title, const std::string& className);

  DWORD getProcessId() const;
  const std::string& exePath() const { return exePath_; }

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

  const std::string title_;
  const std::string className_;
  const std::string exePath_;
};

#endif // PRETZELPROCESS_H
