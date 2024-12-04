#include "Logger.h"
#include <dxgidebug.h>
#include "StringUtility.h"

using namespace StringUtility;

namespace Logger {
	void Log(const std::string& message) { 
		OutputDebugStringA(message.c_str());
	}
    void Log(const std::wstring& message) {
		OutputDebugStringA(ConvertString(message).c_str());
	}
}