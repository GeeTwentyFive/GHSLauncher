#include <string>
#include <stdexcept>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>


const char TARGET_PROCESS_NAME[] = "hl2.exe";
const char HIDDEN_SOURCE_WINDOW_NAME[] = "HIDDEN: SOURCE - Enhanced Edition";

const double TIMEOUT = 10.0;


std::string _GetWin32LastErrorMessage() { const size_t BUF_SIZE = 64 * 1024;
        std::string error_msg; error_msg.reserve(BUF_SIZE);
        DWORD error_code = GetLastError();
        if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0,
                error_code, 0, error_msg.data(), BUF_SIZE, NULL
        ) == 0) return std::string("");
        SetLastError(error_code);
        return error_msg;
}
#define ERROR_WIN32(msg) throw std::runtime_error(std::string("[Win32 ERROR] ") + "@" + std::to_string(__LINE__) + " (" + __func__ + "): " + std::string((msg)) + "\n\n" + "GetLastError(): " + std::to_string(GetLastError()) + "\n\n" + "FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM): " + _GetWin32LastErrorMessage())


int main() { try { LSTATUS _status;
        // Kill "hl2.exe" if already running

        [](){
                PERFORMACE_INFORMATION performance_info;
                if (GetPerformanceInfo(&performance_info, sizeof(PERFORMACE_INFORMATION)) == FALSE) return;

                const DWORD process_ids_size = (performance_info.ProcessCount > 1024) ? performance_info.ProcessCount*2 : 1024*2;
                DWORD process_ids[process_ids_size];
                DWORD process_ids_count;
                if (EnumProcesses(process_ids, process_ids_size, &process_ids_count) == 0) return;
                process_ids_count = process_ids_count / sizeof(DWORD);

                for (DWORD i = 0; i < process_ids_count; i++) { if (process_ids[i] == 0) continue;
                        HANDLE process_handle = OpenProcess((PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE), FALSE,
                                process_ids[i]
                        ); if (process_handle == NULL) continue;

                        char process_name[sizeof(TARGET_PROCESS_NAME)];
                        if (GetModuleBaseNameA(process_handle, NULL, process_name, sizeof(TARGET_PROCESS_NAME)) == sizeof(TARGET_PROCESS_NAME)-1) {
                                if (strcmp(process_name, TARGET_PROCESS_NAME) == 0) {
                                        if (TerminateProcess(process_handle, 0) != 0) WaitForSingleObject(process_handle, INFINITE);
                                }
                        }

                        CloseHandle(process_handle);
                }
        }();


        // Launch Hidden: Source

        DWORD steam_exe_path_len = 1024 + sizeof("\\steam.exe");
        char steam_exe_path[steam_exe_path_len];
        _status = RegGetValueA(HKEY_CURRENT_USER,
                "Software\\Valve\\Steam",
                "SteamPath", RRF_RT_REG_SZ, NULL, steam_exe_path, &steam_exe_path_len
        ); if (_status != ERROR_SUCCESS) { SetLastError(_status); ERROR_WIN32("RegGetValueA(HKEY_CURRENT_USER\\Software\\Valve\\Steam, SteamPath) failed"); }
        strcat(steam_exe_path, "\\steam.exe");

        DWORD hidden_source_path_len = 1024 + sizeof("\\hidden");
        char hidden_source_path[hidden_source_path_len];
        _status = RegGetValueA(HKEY_CURRENT_USER,
                "Software\\Valve\\Steam",
                "SourceModInstallPath", RRF_RT_REG_SZ, NULL, hidden_source_path, &hidden_source_path_len
        ); if (_status != ERROR_SUCCESS) { SetLastError(_status); ERROR_WIN32("RegGetValueA(HKEY_CURRENT_USER\\Software\\Valve\\Steam, SourceModInstallPath) failed"); }
        strcat(hidden_source_path, "\\hidden");

        int screen_width = GetSystemMetrics(SM_CXSCREEN); if (screen_width == 0) ERROR_WIN32("GetSystemMetrics(SM_CXSCREEN) failed");
        int screen_height = GetSystemMetrics(SM_CYSCREEN); if (screen_height == 0) ERROR_WIN32("GetSystemMetrics(SM_CYSCREEN) failed");

        std::string steam_exe_cli_args; steam_exe_cli_args.reserve(32767);
        steam_exe_cli_args = std::string("-applaunch 215 -game \"") + hidden_source_path + "\" -width " + std::to_string(screen_width) + " -height " + std::to_string(screen_height) + " -windowed -console";
        STARTUPINFOA _hs_startup_info = {.cb = sizeof(STARTUPINFOA)}; PROCESS_INFORMATION _hs_proc_info;
        if (CreateProcessA(
                steam_exe_path,
                steam_exe_cli_args.data(), NULL, NULL, FALSE, 0, NULL, NULL, &_hs_startup_info, &_hs_proc_info
        ) == 0) ERROR_WIN32(std::string("CreateProcessA(steam.exe ") + steam_exe_cli_args + ") failed");


        // Set window to borderless window mode

        HWND hidden_source_window = NULL; std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
        while (hidden_source_window == NULL) { Sleep(100); if (std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count() >= TIMEOUT) return 1;
                hidden_source_window = FindWindowA(NULL, HIDDEN_SOURCE_WINDOW_NAME);
        }

        SetWindowLongA(hidden_source_window, GWL_STYLE,
                GetWindowLongA(hidden_source_window, GWL_STYLE) & ~(  // removes any following flags:
                        WS_BORDER | WS_DLGFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME
                )
        );

        SetWindowLongA(hidden_source_window, GWL_EXSTYLE,
                GetWindowLongA(hidden_source_window, GWL_EXSTYLE) & ~(  // removes any following flags:
                        WS_EX_DLGMODALFRAME | WS_EX_COMPOSITED | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_LAYERED | WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_APPWINDOW
                )
        );

        SetWindowPos(hidden_source_window, 0, 0, 0, 0, 0, (
                SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING
        ));

        ShowWindow(hidden_source_window, SW_SHOWMAXIMIZED);
return 0; } catch (const std::exception& e) { MessageBoxA(NULL, e.what(), "GHSLauncher", MB_OK); return 1; } }