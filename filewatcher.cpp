#include <windows.h>

#include "logger.h"
#include "stringhelpers.h"
#include "filewatcher.h"

//--------------------------------------------------------------------------------------------------------------
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

FileWatcher::FileWatcher(const std::string& filePath) : filePath_(filePath) {
  char pDrive[4];
  char pDir[_MAX_DIR];
  char pFile[_MAX_FNAME];

  if (errno_t err = _splitpath_s(filePath_.c_str(), pDrive, 4, pDir, _MAX_DIR, pFile, _MAX_FNAME, NULL, NULL))
    return;

  dirPath_ = static_cast<const char*>(__("%s%s", pDrive, pDir));

  hWatchDirWait_ = CreateFileA(dirPath_.c_str(),
    FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
    NULL);

  overlappedWait_.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

  hWatchDirCheck_ = CreateFileA(dirPath_.c_str(),
    FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
    NULL);

  overlappedCheck_.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

  preparePeek();
};

FileWatcher::~FileWatcher() {
  CloseHandle(overlappedCheck_.hEvent);
  CloseHandle(hWatchDirCheck_);

  CloseHandle(overlappedWait_.hEvent);
  CloseHandle(hWatchDirWait_);
}

bool FileWatcher::waitForFileChange(int timeoutMs) {
  if (ReadDirectoryChangesW(hWatchDirWait_, pNotifyBufWait_, sizeof(pNotifyBufWait_), FALSE,
      FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL, &overlappedWait_, NULL) == 0) {
    Logger::logError("FileWatcher::waitForFileChange: ERROR: ReadDirectoryChangesW function failed.\n");
    return false;
  }

  DWORD result = WaitForSingleObject(overlappedWait_.hEvent, timeoutMs);

  if (result == WAIT_OBJECT_0) {
    DWORD bytesReturned = 0;
    if (!GetOverlappedResult(hWatchDirWait_, &overlappedWait_, &bytesReturned, FALSE)) {
      Logger::logError("Error getting overlapped result.\n");

      return false;
    }

    if (bytesReturned == 0) {
      Logger::logError("FileWatcher::waitForFileChange: ERROR: Cannot get file info. Error code: %d\n", GetLastError());
      return false;
    }

    const FILE_NOTIFY_INFORMATION* pFileInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(pNotifyBufWait_);
    char pMbFileName[_MAX_PATH];
    const int numWchars = pFileInfo->FileNameLength / sizeof(wchar_t);

    const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pFileInfo->FileName, numWchars,
        pMbFileName, sizeof(pMbFileName), NULL, NULL);

    pMbFileName[len] = '\0';

    const std::string notifiedFilePath = dirPath_ + pMbFileName;

    if (notifiedFilePath == filePath_) {
      Logger::logWarning("File changed\n");

      peekFileChange();
      preparePeek();

      if (pFileInfo->Action == FILE_ACTION_ADDED || pFileInfo->Action == FILE_ACTION_MODIFIED)
        return true;
    }
  }
  else if (result == WAIT_TIMEOUT)
    Logger::logWarning("File NOT changed\n");
  else
    Logger::logError("FileWatcher::waitForFileChange: error on waiting. Result = %X", result);

  return false;
}

void FileWatcher::preparePeek() {
  if (ReadDirectoryChangesW(hWatchDirCheck_, pNotifyBufCheck_, sizeof(pNotifyBufCheck_), FALSE,
    FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL, &overlappedCheck_, NULL) == 0) {
    Logger::logError("FileWatcher::peek: ReadDirectoryChangesW function failed.\n");
    return;
  }
}

bool FileWatcher::peekFileChange() {
  DWORD result = WaitForSingleObject(overlappedCheck_.hEvent, 0);

  if (result == WAIT_TIMEOUT)
    return false;

  if (result == WAIT_OBJECT_0) {
    DWORD bytesReturned = 0;
    GetOverlappedResult(hWatchDirCheck_, &overlappedCheck_, &bytesReturned, FALSE);

    if (bytesReturned == 0) {
      Logger::logError("FileWatcher::peekFileChange: ERROR: Cannot get file info.\n");
      return false;
    }

    const FILE_NOTIFY_INFORMATION* pFileInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(pNotifyBufCheck_);
    char pMbFileName[_MAX_PATH];

    const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pFileInfo->FileName, -1, pMbFileName,
      sizeof(pMbFileName), NULL, NULL);

    const std::string notifiedFilePath = dirPath_ + pMbFileName;

    if (notifiedFilePath == filePath_) {
      preparePeek();

      if (pFileInfo->Action == FILE_ACTION_ADDED || pFileInfo->Action == FILE_ACTION_MODIFIED)
        return true;
    }
  }
  else
    return false;

  return true;
}
