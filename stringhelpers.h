#ifndef STRINGHELPERS_H
#define STRINGHELPERS_H

#include <cstdarg>
#include <string>

//--------------------------------------------------------------------------------------------------------------
// __V
//--------------------------------------------------------------------------------------------------------------

class __V {
public:
  __V(const char* pFormat, const va_list& args) {
    const int reqLen = vsnprintf(nullptr, 0, pFormat, args) + 1;
    char* pTmp = new char[reqLen];
    vsnprintf(pTmp, reqLen, pFormat, args);

    str_ = pTmp;
    delete[] pTmp;
  }

  operator const std::string() const { return str_; }
  operator const char* () { return str_.c_str(); }

private:
  std::string str_;
};

//--------------------------------------------------------------------------------------------------------------
// __
//--------------------------------------------------------------------------------------------------------------

class __ {
public:
  __(const char* pFormat, ...) {
    va_list args;
    va_start(args, pFormat);
    str_ = static_cast<const std::string>(__V(pFormat, args));
    va_end(args);
  }

  operator const std::string() const { return str_; }
  operator const char* () { return str_.c_str(); }

private:
  std::string str_;
};

#endif // STRINGHELPERS_H
