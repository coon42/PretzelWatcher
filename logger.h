#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <windows.h>

#include "singleton.h"

//--------------------------------------------------------------------------------------------------------------
// Logger
//--------------------------------------------------------------------------------------------------------------

class Logger : public Singleton<Logger> {
public:
  Logger();
  ~Logger();

  static void log(const char* pFormat, ...);
  static void logSuccess(const char* pFormat, ...);
  static void logError(const char* pFormat, ...);
  static void logWarning(const char* pFormat, ...);

private:
  static void printColored(const char* pFormat, va_list args, uint16_t colorAttributes);
  HANDLE hPrintMutex_{INVALID_HANDLE_VALUE};
};

#endif // LOGGER_H
