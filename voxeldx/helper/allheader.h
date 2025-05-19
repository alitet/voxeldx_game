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

#define PRINT(text) std::cout << (char*)text << "\n" << std::flush; 
#define VERIFY_SUCCEEDED(hr) \
{ \
  HRESULT hrLocal = hr; if(FAILED(hrLocal)) { \
    PRINT("Error at: " << __FILE__ << ", line: " << __LINE__ << ", HRESULT: 0x" << std::hex << hrLocal); throw E_FAIL; \
  } \
}

#define CATCH_PRINT_ERROR(extraCatchCode) \
    catch(const std::exception& ex) \
    { \
        fwprintf(stderr, L"ERROR: %hs\n", ex.what()); \
        extraCatchCode \
    } \
    catch(...) \
    { \
        fwprintf(stderr, L"UNKNOWN ERROR.\n"); \
        extraCatchCode \
    }

constexpr const wchar_t* const CLASS_NAME = L"VOXELENGINO";
constexpr const wchar_t* const WINDOW_TITLE = L"Voxel Engine Discrete";