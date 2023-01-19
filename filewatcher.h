#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <string>

//--------------------------------------------------------------------------------------------------------------
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

class FileWatcher {
public:
  FileWatcher(const std::string& filePath);
  ~FileWatcher();

  const std::string& filePath() { return filePath_; }
  bool waitForFileChange(int timeoutMs = INFINITE);
  bool peekFileChange();

private:
  void preparePeek();

  const std::string filePath_;
  std::string dirPath_;
  HANDLE hWatchDirWait_{INVALID_HANDLE_VALUE};
  OVERLAPPED overlappedWait_{0};
  unsigned char pNotifyBufWait_[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};

  HANDLE hWatchDirCheck_{INVALID_HANDLE_VALUE};
  OVERLAPPED overlappedCheck_{0};
  unsigned char pNotifyBufCheck_[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};
};

#endif // FILEWATCHER_H
