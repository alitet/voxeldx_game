#pragma once
#include <iostream>
#include <Windows.h>

enum class ExitCode : int
{
  GPUList = 2,
  Help = 1,
  Success = 0,
  RuntimeError = -1,
  CommandLineError = -2,
};

constexpr const wchar_t* const CLASS_NAME = L"VOXELENGINO";
constexpr const wchar_t* const WINDOW_TITLE = L"Voxel Engine Discrete";

#define PRINT(text) std::cout << (char*)text << "\n" << std::flush; 
#define VERIFY_SUCCEEDED(hr) \
{ \
  HRESULT hrLocal = hr; if(FAILED(hrLocal)) { \
    PRINT("Error at: " << __FILE__ << ", line: " << __LINE__ << ", HRESULT: 0x" << std::hex << hrLocal); throw E_FAIL; \
  } \
}

//#define CATCH_PRINT_ERROR(extraCatchCode) \
//    catch(const std::exception& ex) \
//    { \
//        fwprintf(stderr, L"ERROR: %hs\n", ex.what()); \
//        extraCatchCode \
//    } \
//    catch(...) \
//    { \
//        fwprintf(stderr, L"UNKNOWN ERROR.\n"); \
//        extraCatchCode \
//    }

#define CATCH_PRINT_ERROR(extraCatchCode) \
    catch(const std::exception& ex) \
    { \
        PRINT("ERROR" << ex.what()) \
        extraCatchCode \
    } \
    catch(...) \
    { \
        PRINT("UNKN ERROR") \
        extraCatchCode \
    }


template <typename T>
inline constexpr T AlignUp(T val, T align)
{
  return (val + align - 1) / align * align;
}

static std::string utf8_encode(const std::wstring& wstr)
{
  if (wstr.empty()) return std::string();
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
  return strTo;
}