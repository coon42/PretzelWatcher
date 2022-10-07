#ifndef FILEWATCHER_H
#define FILEWATCHER_H

//--------------------------------------------------------------------------------------------------------------
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

class FileWatcher {
public:
  FileWatcher(const std::string& filePath) : filePath_(filePath) { };

  const std::string& filePath() { return filePath_; }
  bool waitForFileChange();

private:
  const std::string filePath_;
};

#endif // FILEWATCHER_H
