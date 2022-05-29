/*
    *Gloomy.cc
    *https://github.com/Chase1803

    Copyright (c) 2022 Chase1803
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:
    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
 */

#include "singed.hpp"

void system_no_output(std::string command)
{
    command.insert(0, "/C ");

    SHELLEXECUTEINFOA ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = "cmd.exe";
    ShExecInfo.lpParameters = command.c_str();
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;

    if (ShellExecuteExA(&ShExecInfo) == FALSE)

        WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

    DWORD rv;
    GetExitCodeProcess(ShExecInfo.hProcess, &rv);
    CloseHandle(ShExecInfo.hProcess);
}

bool chase1803magic()
{
    HKEY hKey{};
    LONG lReg{};

    DWORD disable = 0x3;

    lReg = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        XorStr(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management").c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL
    );

    if (lReg != ERROR_SUCCESS) return false;

    lReg = RegSetValueEx(
        hKey,
        XorStr(L"FeatureSettingsOverride").c_str(),
        NULL,
        REG_DWORD,
        (LPBYTE)&disable,
        sizeof(disable)
    );

    if (lReg != ERROR_SUCCESS) return false;

    lReg = RegSetValueEx(
        hKey,
        XorStr(L"FeatureSettingsOverrideMask").c_str(),
        NULL,
        REG_DWORD,
        (LPBYTE)&disable,
        sizeof(disable)
    );

    if (lReg != ERROR_SUCCESS) return false;

    RegCloseKey(hKey);
    return true;
}

bool VulnerableDriver::Init()
{
    chase1803magic();

    const std::string placement_path = XorStr("C:\\Windows\\System32\\drivers\\vmbusraid.sys").c_str();

   //const std::string placement_path = XorStr("C:\\Windows\\System32\\drivers\\vmbus.sys").c_str(); 

    /* If vmbusraid.sys dosent work use vmbus.sys*/

    if (std::filesystem::exists(placement_path))
    {
        std::remove(placement_path.c_str());
    }

    if (!file_utils::create_file_from_buffer(
        placement_path,
        (void*)resource::raw_driver,
        sizeof(resource::raw_driver)
    ))
    {
        return false;
    }

    detail::service_handle = service_utils::create_service(placement_path);

    if (!CHECK_HANDLE(detail::service_handle))
    {
        return false;
    }


    if (!service_utils::start_service(detail::service_handle))
    {
        system_no_output(XorStr("sc start SandyNuts").c_str());
        return true;
    }

    return true;
}

void VulnerableDriver::Unload()
{
    system_no_output(XorStr("sc stop SandyNuts").c_str());
    system_no_output(XorStr("cls").c_str());
}
