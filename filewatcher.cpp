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

  hWatchDir_ = CreateFileA(dirPath_.c_str(),
    FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
    NULL);

  overlapped_.hEvent = CreateEvent(NULL, FALSE, 0, NULL);
};

FileWatcher::~FileWatcher() {
  CloseHandle(overlapped_.hEvent);
  CloseHandle(hWatchDir_);
}

bool FileWatcher::waitForFileChange(int timeoutMs) {
  unsigned char pBuf[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};

  if (ReadDirectoryChangesW(hWatchDir_, pBuf, sizeof(pBuf), FALSE, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL,
      &overlapped_, NULL) == 0) {
    Logger::logError("ERROR: ReadDirectoryChangesW function failed.\n");
    return false;
  }

  DWORD result = WaitForSingleObject(overlapped_.hEvent, timeoutMs);

  if (result == WAIT_OBJECT_0) {
    DWORD bytesReturned = 0;
    GetOverlappedResult(hWatchDir_, &overlapped_, &bytesReturned, FALSE);

    if (bytesReturned == 0) {
      Logger::logError("ERROR: Cannot get file info.\n");
      return false;
    }

    const FILE_NOTIFY_INFORMATION* pFileInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(pBuf);
    char pMbFileName[_MAX_PATH];

    const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pFileInfo->FileName, -1, pMbFileName,
        sizeof(pMbFileName), NULL, NULL);

    const std::string notifiedFilePath = dirPath_ + pMbFileName;

    if (notifiedFilePath == filePath_) {
      if (pFileInfo->Action == FILE_ACTION_ADDED || pFileInfo->Action == FILE_ACTION_MODIFIED)
        return true;
    }
  }

  return false;
}
