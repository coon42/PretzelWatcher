#include <string>
#include <windows.h>

#include "logger.h"
#include "stringhelpers.h"
#include "filewatcher.h"

//--------------------------------------------------------------------------------------------------------------
// FileWatcher
//--------------------------------------------------------------------------------------------------------------

bool FileWatcher::waitForFileChange() {
  char pDrive[4];
  char pDir[_MAX_DIR];
  char pFile[_MAX_FNAME];

  if (errno_t err = _splitpath_s(filePath_.c_str(), pDrive, 4, pDir, _MAX_DIR, pFile, _MAX_FNAME, NULL, NULL))
    return false;

  const std::string dirPath = __("%s%s", pDrive, pDir);

  while (true) {
    const HANDLE hDir = CreateFileA(dirPath.c_str(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS,
      NULL
    );

    unsigned char pBuf[sizeof(FILE_NOTIFY_INFORMATION) + _MAX_PATH]{0};

    DWORD bytesReturned = 0;

    if (ReadDirectoryChangesW(hDir, pBuf, sizeof(pBuf), FALSE, FILE_NOTIFY_CHANGE_ATTRIBUTES, &bytesReturned, NULL, NULL) == 0) {
      Logger::logError("ERROR: ReadDirectoryChangesW function failed.\n");
      return false;
    }

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

    if (notifiedFilePath == filePath_)
      return true;
  }
}