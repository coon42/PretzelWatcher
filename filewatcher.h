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

private:
  const std::string filePath_;
  std::string dirPath_;
  HANDLE hWatchDir_{INVALID_HANDLE_VALUE};
  OVERLAPPED overlapped_{0};
};

#endif // FILEWATCHER_H
