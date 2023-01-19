#include <string>
#include <windows.h>

#include "logger.h"
#include "stringhelpers.h"
#include "filewatcher.h"

//--------------------------------------------------------------------------------------------------------------
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

bool FileWatcher::waitForFileChange(int timeoutMs) {
  char pDrive[4];
  char pDir[_MAX_DIR];
  char pFile[_MAX_FNAME];

  if (errno_t err = _splitpath_s(filePath_.c_str(), pDrive, 4, pDir, _MAX_DIR, pFile, _MAX_FNAME, NULL, NULL))
    return false;

  const std::string dirPath = __("%s%s", pDrive, pDir);

  const HANDLE hDir = CreateFileA(dirPath.c_str(),
    FILE_LIST_DIRECTORY,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    OPEN_EXISTING,
    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
    NULL);

  OVERLAPPED overlapped;
  overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

  unsigned char pBuf[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};


  if (ReadDirectoryChangesW(hDir, pBuf, sizeof(pBuf), FALSE, FILE_NOTIFY_CHANGE_ATTRIBUTES, NULL, &overlapped, NULL) == 0) {
    Logger::logError("ERROR: ReadDirectoryChangesW function failed.\n");
    return false;
  }

  DWORD result = WaitForSingleObject(overlapped.hEvent, timeoutMs);

  if (result == WAIT_OBJECT_0) {
    DWORD bytesReturned = 0;
    GetOverlappedResult(hDir, &overlapped, &bytesReturned, FALSE);

    if (bytesReturned == 0) {
      Logger::logError("ERROR: Cannot get file info.\n");
      return false;
    }

    CloseHandle(hDir);

    const FILE_NOTIFY_INFORMATION* pFileInfo = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(pBuf);
    char pMbFileName[_MAX_PATH];

    const int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, pFileInfo->FileName, -1, pMbFileName,
        sizeof(pMbFileName), NULL, NULL);

    const std::string notifiedFilePath = dirPath + pMbFileName;

    if (notifiedFilePath == filePath_) {
      if (pFileInfo->Action == FILE_ACTION_ADDED || pFileInfo->Action == FILE_ACTION_MODIFIED)
        return true;
    }
  }

  CloseHandle(overlapped.hEvent);

  return false;
}
