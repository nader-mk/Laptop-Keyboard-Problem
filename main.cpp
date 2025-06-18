#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <cctype>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_TOGGLE 1001
#define ID_TRAY_ABOUT 1002
#define ID_TRAY_RELOAD 1003
#define ID_TRAY_EXIT 1004

// Global variables
HHOOK g_hKeyboardHook = NULL;
HWND g_hWnd = NULL;
NOTIFYICONDATA g_nid = {0};
bool g_bBlockingEnabled = true;
HINSTANCE g_hInstance = NULL;
std::vector<DWORD> g_blockedKeys;
std::string g_configPath;
std::vector<std::string> g_blockedKeyNames;

// Virtual key code mapping
std::map<std::string, DWORD> g_keyMap = {
    // Numbers
    {"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
    {"5", 0x35}, {"6", 0x36}, {"7", 0x37}, {"8", 0x38}, {"9", 0x39},
    
    // Letters
    {"A", 0x41}, {"B", 0x42}, {"C", 0x43}, {"D", 0x44}, {"E", 0x45},
    {"F", 0x46}, {"G", 0x47}, {"H", 0x48}, {"I", 0x49}, {"J", 0x4A},
    {"K", 0x4B}, {"L", 0x4C}, {"M", 0x4D}, {"N", 0x4E}, {"O", 0x4F},
    {"P", 0x50}, {"Q", 0x51}, {"R", 0x52}, {"S", 0x53}, {"T", 0x54},
    {"U", 0x55}, {"V", 0x56}, {"W", 0x57}, {"X", 0x58}, {"Y", 0x59},
    {"Z", 0x5A},
    
    // Special characters
    {"-", VK_OEM_MINUS}, {"=", VK_OEM_PLUS}, {"[", VK_OEM_4}, {"]", VK_OEM_6},
    {"\\", VK_OEM_5}, {";", VK_OEM_1}, {"'", VK_OEM_7}, {",", VK_OEM_COMMA},
    {".", VK_OEM_PERIOD}, {"/", VK_OEM_2}, {"`", VK_OEM_3},
    
    // Numpad
    {"NUMPAD0", VK_NUMPAD0}, {"NUMPAD1", VK_NUMPAD1}, {"NUMPAD2", VK_NUMPAD2},
    {"NUMPAD3", VK_NUMPAD3}, {"NUMPAD4", VK_NUMPAD4}, {"NUMPAD5", VK_NUMPAD5},
    {"NUMPAD6", VK_NUMPAD6}, {"NUMPAD7", VK_NUMPAD7}, {"NUMPAD8", VK_NUMPAD8},
    {"NUMPAD9", VK_NUMPAD9}, {"*", VK_MULTIPLY}, {"+", VK_ADD},
    {"NUMPAD-", VK_SUBTRACT}, {"NUMPAD.", VK_DECIMAL}, {"NUMPAD/", VK_DIVIDE},
    
    // Function keys
    {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
    {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
    {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
    
    // Control keys
    {"SPACE", VK_SPACE}, {"ENTER", VK_RETURN}, {"TAB", VK_TAB},
    {"BACKSPACE", VK_BACK}, {"DELETE", VK_DELETE}, {"INSERT", VK_INSERT},
    {"HOME", VK_HOME}, {"END", VK_END}, {"PAGEUP", VK_PRIOR}, {"PAGEDOWN", VK_NEXT},
    {"UP", VK_UP}, {"DOWN", VK_DOWN}, {"LEFT", VK_LEFT}, {"RIGHT", VK_RIGHT},
    {"ESCAPE", VK_ESCAPE}, {"CAPSLOCK", VK_CAPITAL}, {"SHIFT", VK_SHIFT},
    {"CTRL", VK_CONTROL}, {"ALT", VK_MENU}, {"LWIN", VK_LWIN}, {"RWIN", VK_RWIN}
};

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void CreateTrayIcon(HWND hwnd);
void ShowContextMenu(HWND hwnd);
void ShowAboutDialog(HWND hwnd);
void ToggleBlocking();
void UpdateTrayIcon();
bool LoadConfiguration();
void CreateDefaultConfig();
std::string GetDesktopPath();
std::string ToUpper(const std::string& str);

// Get desktop path
std::string GetDesktopPath() {
    char path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, path) == S_OK) {
        return std::string(path);
    }
    return "";
}

// Convert string to uppercase
std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c){ return std::toupper(c); });
    return result;
}

// Create default configuration file
void CreateDefaultConfig() {
    std::ofstream file(g_configPath);
    if (file.is_open()) {
        file << "# Keyboard Blocker Configuration File\n";
        file << "# Add one key per line\n";
        file << "# Available keys:\n";
        file << "# - Numbers: 0-9\n";
        file << "# - Letters: A-Z\n";
        file << "# - Special: - = [ ] \\ ; ' , . / ` SPACE ENTER TAB BACKSPACE\n";
        file << "# - Numpad: NUMPAD0-9, *, +, NUMPAD-, NUMPAD., NUMPAD/\n";
        file << "# - Function: F1-F12\n";
        file << "# - Control: DELETE INSERT HOME END PAGEUP PAGEDOWN UP DOWN LEFT RIGHT\n";
        file << "# - Modifiers: ESCAPE CAPSLOCK SHIFT CTRL ALT LWIN RWIN\n";
        file << "\n";
        file << "# Default blocked keys:\n";
        file << "7\n";
        file << "8\n";
        file << "-\n";
        file.close();
    }
}

// Load configuration from file
bool LoadConfiguration() {
    g_blockedKeys.clear();
    g_blockedKeyNames.clear();
    
    std::ifstream file(g_configPath);
    if (!file.is_open()) {
        CreateDefaultConfig();
        file.open(g_configPath);
        if (!file.is_open()) {
            return false;
        }
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Convert to uppercase for lookup
        std::string upperKey = ToUpper(line);
        
        // Find the virtual key code
        auto it = g_keyMap.find(upperKey);
        if (it != g_keyMap.end()) {
            g_blockedKeys.push_back(it->second);
            g_blockedKeyNames.push_back(line);
        }
    }
    
    file.close();
    return true;
}

// Keyboard hook procedure
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && g_bBlockingEnabled) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        
        // Check if it's a keydown event
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            // Check if this key is in our blocked list
            for (DWORD blockedKey : g_blockedKeys) {
                if (pKeyboard->vkCode == blockedKey) {
                    return 1; // Block the key
                }
            }
        }
    }
    
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateTrayIcon(hwnd);
            break;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                ShowContextMenu(hwnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                ShowAboutDialog(hwnd);
            }
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_TOGGLE:
                    ToggleBlocking();
                    break;
                case ID_TRAY_ABOUT:
                    ShowAboutDialog(hwnd);
                    break;
                case ID_TRAY_RELOAD:
                    if (LoadConfiguration()) {
                        MessageBoxA(hwnd, "Configuration reloaded successfully!", 
                                   "Keyboard Blocker", MB_OK | MB_ICONINFORMATION);
                    } else {
                        MessageBoxA(hwnd, "Failed to reload configuration!", 
                                   "Error", MB_OK | MB_ICONERROR);
                    }
                    break;
                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            break;
            
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &g_nid);
            if (g_hKeyboardHook) {
                UnhookWindowsHookEx(g_hKeyboardHook);
                g_hKeyboardHook = NULL;
            }
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Create system tray icon
void CreateTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(g_nid.szTip, "Keyboard Blocker - Active");
    
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

// Update tray icon tooltip
void UpdateTrayIcon() {
    if (g_bBlockingEnabled) {
        strcpy(g_nid.szTip, "Keyboard Blocker - Active");
    } else {
        strcpy(g_nid.szTip, "Keyboard Blocker - Inactive");
    }
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

// Show context menu
void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    
    HMENU hMenu = CreatePopupMenu();
    if (hMenu) {
        AppendMenuA(hMenu, MF_STRING, ID_TRAY_TOGGLE, 
                   g_bBlockingEnabled ? "Disable Blocking" : "Enable Blocking");
        AppendMenuA(hMenu, MF_STRING, ID_TRAY_RELOAD, "Reload Config");
        AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(hMenu, MF_STRING, ID_TRAY_ABOUT, "About");
        AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");
        
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    }
}

// Show about dialog
void ShowAboutDialog(HWND hwnd) {
    std::stringstream ss;
    ss << "Keyboard Blocker v1.0\n\n";
   ss << "Developed by Nader Mahbub Khan (c) 2025\nBachelor of Arts (English)(Hons.), National University, Bangladesh\n";
    ss << "Configuration file: " << g_configPath << "\n\n";
    ss << "Currently blocked keys:\n";
    
    if (g_blockedKeyNames.empty()) {
        ss << "(No keys configured)\n";
    } else {
        for (const auto& key : g_blockedKeyNames) {
            ss << "• " << key << "\n";
        }
    }
    
    ss << "\nRight-click the tray icon for options.\n";
    ss << "Double-click the tray icon to show this dialog.";
    
    MessageBoxA(hwnd, ss.str().c_str(), "About Keyboard Blocker", MB_OK | MB_ICONINFORMATION);
}

// Toggle blocking on/off
void ToggleBlocking() {
    g_bBlockingEnabled = !g_bBlockingEnabled;
    UpdateTrayIcon();
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Check if already running
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "KeyboardBlockerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(NULL, "Keyboard Blocker is already running!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }
    
    g_hInstance = hInstance;
    
    // Set config path
    g_configPath = GetDesktopPath() + "\\Nader.cfg";
    
    // looad configuration
    if (!LoadConfiguration()) {
        MessageBoxA(NULL, "Failed to load configuration file!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
        // Register window class :0
    const char* CLASS_NAME = "KeyboardBlockerWindow";
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hMutex);
        return 1;
    }
    
    // Create hidden window
    g_hWnd = CreateWindowExA(
        WS_EX_TOOLWINDOW,  // This helps keep it hidden from taskbar
        CLASS_NAME,
        "Keyboard Blocker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        100, 100,  // Small size since it's hidden
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (!g_hWnd) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hMutex);
        return 1;
    }
    
    // Don't show the window - keep it hidden
    ShowWindow(g_hWnd, SW_HIDE);
    UpdateWindow(g_hWnd);
    
    // Install keyboard hook
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    if (!g_hKeyboardHook) {
        MessageBoxA(NULL, "Failed to install keyboard hook", "Error", MB_OK | MB_ICONERROR);
        DestroyWindow(g_hWnd);
        CloseHandle(hMutex);
        return 1;
    }
    
    // Show initial notification
    MessageBoxA(NULL, "Keyboard Blocker started successfully!\nCheck the system tray icon.", 
                "Keyboard Blocker", MB_OK | MB_ICONINFORMATION);
    
    // Message loop
    MSG msg;
    BOOL bRet;
    
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            // Handle error
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // Cleanup
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
    }
    
    CloseHandle(hMutex);
    
    return (int)msg.wParam;
} 
