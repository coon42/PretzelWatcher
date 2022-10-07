#include <chrono>
#include <format>

#include "stringhelpers.h"
#include "logger.h"

//--------------------------------------------------------------------------------------------------------------
// Logger
//--------------------------------------------------------------------------------------------------------------

Logger::Logger() {
  hPrintMutex_ = CreateMutex(NULL, FALSE, NULL);
}

Logger::~Logger() {
  CloseHandle(hPrintMutex_);
}

void Logger::printColored(const char* pFormat, va_list args, uint16_t colorAttributes) {
  Logger* pThis = Logger::getInstance();

  if (pThis) {
    WaitForSingleObject(pThis->hPrintMutex_, INFINITE);

    const std::string text = __V(pFormat, args);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    GetConsoleScreenBufferInfo(hConsole, &consoleInfo); // Save current font color
    saved_attributes = consoleInfo.wAttributes;
    SetConsoleTextAttribute(hConsole, colorAttributes); // Change font color

    std::chrono::zoned_time time{std::chrono::current_zone(), std::chrono::system_clock::now()};

    printf(std::format("[{:%Y-%m-%d %X}] ", time).c_str());
    printf(text.c_str());

    SetConsoleTextAttribute(hConsole, saved_attributes); // Restore original font color

    ReleaseMutex(pThis->hPrintMutex_);
  }
}

void Logger::log(const char* pFormat, ...) {
  va_list args;
  va_start(args, pFormat);
  printColored(pFormat, args, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  va_end(args);
}

void Logger::logSuccess(const char* pFormat, ...) {
  va_list args;
  va_start(args, pFormat);
  printColored(pFormat, args, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
  va_end(args);
}

void Logger::logError(const char* pFormat, ...) {
  va_list args;
  va_start(args, pFormat);
  printColored(pFormat, args, FOREGROUND_RED | FOREGROUND_INTENSITY);
  va_end(args);
}

void Logger::logWarning(const char* pFormat, ...) {
  va_list args;
  va_start(args, pFormat);
  printColored(pFormat, args, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Yellow
  va_end(args);
}
