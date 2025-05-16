#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#pragma comment(lib, "comctl32.lib")

// Character sets for password generation
const char LOWERCASE[] = "abcdefghijklmnopqrstuvwxyz";
const char UPPERCASE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char DIGITS[]    = "0123456789";
const char SYMBOLS[]   = "!@#$%^&*()-_=+[]{};:,.<>?/";

#define MAX_PASSWORDS 20
#define MAX_LEN 255
#define MIN_LEN 4

// GUI control IDs
#define ID_EDIT_LENGTH   101
#define ID_EDIT_COUNT    102
#define ID_CHECK_LOWER   103
#define ID_CHECK_UPPER   104
#define ID_CHECK_DIGIT   105
#define ID_CHECK_SYMBOL  106
#define ID_BUTTON_GEN    107
#define ID_EDIT_OUTPUT   108
#define ID_BUTTON_COPY   109
#define ID_GROUP_OPTIONS 110
#define ID_GROUP_OUTPUT  111
#define ID_STATIC_STRENGTH 112
#define ID_STATUSBAR     113
#define ID_ABOUT         114
#define CONFIG_FILE      "settings.cfg"

// Helper: shuffle
void swap(char *a, char *b) { char t = *a; *a = *b; *b = t; }
void shuffle(char *array, int n) {
    for (int i = n - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(&array[i], &array[j]);
    }
}

// Generate a password using only selected sets
int generate_password(char *out, int length, int use_lower, int use_upper, int use_digit, int use_symbol) {
    const char *sets[4];
    int set_sizes[4];
    int num_sets = 0;
    if (use_lower)  { sets[num_sets] = LOWERCASE; set_sizes[num_sets++] = 26; }
    if (use_upper)  { sets[num_sets] = UPPERCASE; set_sizes[num_sets++] = 26; }
    if (use_digit)  { sets[num_sets] = DIGITS;    set_sizes[num_sets++] = 10; }
    if (use_symbol) { sets[num_sets] = SYMBOLS;   set_sizes[num_sets++] = 24; }
    if (num_sets == 0 || length < num_sets) return 0;
    int pos = 0;
    // Guarantee one char from each selected set
    for (int s = 0; s < num_sets; ++s) {
        int idx = rand() % set_sizes[s];
        out[pos++] = sets[s][idx];
    }
    // Fill the rest randomly
    for (; pos < length; ++pos) {
        int set = rand() % num_sets;
        int idx = rand() % set_sizes[set];
        out[pos] = sets[set][idx];
    }
    shuffle(out, length);
    out[length] = '\0';
    return 1;
}

// Password strength: weak/medium/strong
const char* password_strength(int length, int num_sets) {
    if (length < 8 || num_sets < 2) return "Weak";
    if (length < 12 || num_sets < 3) return "Medium";
    return "Strong";
}

COLORREF strength_color(const char* strength) {
    if (strcmp(strength, "Weak") == 0) return RGB(220, 50, 47);      // Red
    if (strcmp(strength, "Medium") == 0) return RGB(203, 154, 0);    // Yellow
    return RGB(0, 128, 0);                                            // Green
}

// Set Segoe UI font for all controls
HFONT create_font() {
    return CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        FF_DONTCARE, "Segoe UI");
}

void set_font_all(HWND parent, HFONT font) {
    HWND h = GetWindow(parent, GW_CHILD);
    while (h) {
        SendMessage(h, WM_SETFONT, (WPARAM)font, TRUE);
        h = GetWindow(h, GW_HWNDNEXT);
    }
}

void center_window(HWND hwnd) {
    RECT rc = {0};
    GetWindowRect(hwnd, &rc);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hwnd, NULL, (sw-w)/2, (sh-h)/2, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

void save_settings(int len, int cnt, int lower, int upper, int digit, int symbol) {
    FILE *f = fopen(CONFIG_FILE, "w");
    if (!f) return;
    fprintf(f, "%d %d %d %d %d %d\n", len, cnt, lower, upper, digit, symbol);
    fclose(f);
}

int load_settings(int *len, int *cnt, int *lower, int *upper, int *digit, int *symbol) {
    FILE *f = fopen(CONFIG_FILE, "r");
    if (!f) return 0;
    int ok = fscanf(f, "%d %d %d %d %d %d", len, cnt, lower, upper, digit, symbol) == 6;
    fclose(f);
    return ok;
}

void show_about(HWND hwnd) {
    MessageBox(hwnd, "Password Generator\n\nWin32 GUI demo\n\nCreated by TMHSDigital\n2025", "About", MB_OK|MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditLen, hEditCnt, hCheckLower, hCheckUpper, hCheckDigit, hCheckSymbol, hButton, hEditOut, hButtonCopy, hGroupOpt, hGroupOut, hStaticStrength, hStatus;
    static HFONT hFont = NULL;
    static HWND hTooltip = NULL;
    switch (msg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_BAR_CLASSES};
        InitCommonControlsEx(&icc);
        hFont = create_font();
        hGroupOpt = CreateWindow("BUTTON", "Options", WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 10, 5, 320, 110, hwnd, (HMENU)ID_GROUP_OPTIONS, NULL, NULL);
        CreateWindow("STATIC", "Length:", WS_CHILD|WS_VISIBLE, 30, 30, 50, 20, hwnd, NULL, NULL, NULL);
        hEditLen = CreateWindow("EDIT", "8", WS_CHILD|WS_VISIBLE|ES_NUMBER, 85, 30, 40, 22, hwnd, (HMENU)ID_EDIT_LENGTH, NULL, NULL);
        CreateWindow("STATIC", "Count:", WS_CHILD|WS_VISIBLE, 140, 30, 45, 20, hwnd, NULL, NULL, NULL);
        hEditCnt = CreateWindow("EDIT", "1", WS_CHILD|WS_VISIBLE|ES_NUMBER, 190, 30, 30, 22, hwnd, (HMENU)ID_EDIT_COUNT, NULL, NULL);
        hButton = CreateWindow("BUTTON", "Generate", WS_CHILD|WS_VISIBLE, 240, 30, 75, 25, hwnd, (HMENU)ID_BUTTON_GEN, NULL, NULL);
        hCheckLower  = CreateWindow("BUTTON", "Lowercase", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 30, 60, 90, 20, hwnd, (HMENU)ID_CHECK_LOWER, NULL, NULL);
        hCheckUpper  = CreateWindow("BUTTON", "Uppercase", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 130, 60, 90, 20, hwnd, (HMENU)ID_CHECK_UPPER, NULL, NULL);
        hCheckDigit  = CreateWindow("BUTTON", "Digits",    WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 30, 85, 90, 20, hwnd, (HMENU)ID_CHECK_DIGIT, NULL, NULL);
        hCheckSymbol = CreateWindow("BUTTON", "Symbols",   WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 130, 85, 90, 20, hwnd, (HMENU)ID_CHECK_SYMBOL, NULL, NULL);
        SendMessage(hCheckLower, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(hCheckUpper, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(hCheckDigit, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(hCheckSymbol, BM_SETCHECK, BST_CHECKED, 0);
        hButtonCopy = CreateWindow("BUTTON", "Copy", WS_CHILD|WS_VISIBLE, 240, 85, 75, 25, hwnd, (HMENU)ID_BUTTON_COPY, NULL, NULL);
        hGroupOut = CreateWindow("BUTTON", "Output", WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 10, 120, 320, 110, hwnd, (HMENU)ID_GROUP_OUTPUT, NULL, NULL);
        hEditOut = CreateWindow("EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_BORDER, 30, 145, 280, 55, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
        hStaticStrength = CreateWindow("STATIC", "", WS_CHILD|WS_VISIBLE, 30, 205, 200, 20, hwnd, (HMENU)ID_STATIC_STRENGTH, NULL, NULL);
        hStatus = CreateWindowEx(0, STATUSCLASSNAME, "Ready", WS_CHILD|WS_VISIBLE|SBARS_SIZEGRIP, 0, 0, 0, 0, hwnd, (HMENU)ID_STATUSBAR, NULL, NULL);
        set_font_all(hwnd, hFont);
        // Tooltips
        hTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_ALWAYSTIP, 0,0,0,0, hwnd, NULL, NULL, NULL);
        TOOLINFO ti;
        ti.cbSize = sizeof(TOOLINFO);
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = hwnd;
        ti.uId = 0;
        ti.rect.left = ti.rect.top = ti.rect.right = ti.rect.bottom = 0;
        ti.hinst = NULL;
        ti.lParam = 0;
        ti.lpszText = NULL;
        #define ADDTIP(ctrl, txt) ti.hwnd = ctrl; ti.lpszText = (LPSTR)txt; SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
        ADDTIP(hEditLen,    "Password length (min: number of sets, max: 255)");
        ADDTIP(hEditCnt,    "Number of passwords to generate (max 20)");
        ADDTIP(hCheckLower, "Include lowercase letters");
        ADDTIP(hCheckUpper, "Include uppercase letters");
        ADDTIP(hCheckDigit, "Include digits");
        ADDTIP(hCheckSymbol,"Include symbols");
        ADDTIP(hButton,     "Generate password(s)");
        ADDTIP(hButtonCopy, "Copy passwords to clipboard");
        ADDTIP(hEditOut,    "Generated password(s)");
        #undef ADDTIP
        // Load settings
        int len=8, cnt=1, lower=1, upper=1, digit=1, symbol=1;
        if (load_settings(&len, &cnt, &lower, &upper, &digit, &symbol)) {
            char buf[16];
            sprintf(buf, "%d", len); SetWindowText(hEditLen, buf);
            sprintf(buf, "%d", cnt); SetWindowText(hEditCnt, buf);
            SendMessage(hCheckLower,  BM_SETCHECK, lower  ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckUpper,  BM_SETCHECK, upper  ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckDigit,  BM_SETCHECK, digit  ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hCheckSymbol, BM_SETCHECK, symbol ? BST_CHECKED : BST_UNCHECKED, 0);
        }
        break; }
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_GEN || (HIWORD(wParam)==BN_CLICKED && LOWORD(wParam)==ID_BUTTON_GEN)) {
            char bufLen[16], bufCnt[16], out[4096] = "", password[MAX_LEN+1];
            int len = 0, cnt = 0, ok = 1, nsets = 0;
            int use_lower = SendMessage(hCheckLower, BM_GETCHECK, 0, 0) == BST_CHECKED;
            int use_upper = SendMessage(hCheckUpper, BM_GETCHECK, 0, 0) == BST_CHECKED;
            int use_digit = SendMessage(hCheckDigit, BM_GETCHECK, 0, 0) == BST_CHECKED;
            int use_symbol = SendMessage(hCheckSymbol, BM_GETCHECK, 0, 0) == BST_CHECKED;
            nsets = use_lower + use_upper + use_digit + use_symbol;
            GetWindowText(hEditLen, bufLen, sizeof(bufLen));
            GetWindowText(hEditCnt, bufCnt, sizeof(bufCnt));
            len = atoi(bufLen);
            cnt = atoi(bufCnt);
            if (len < nsets || len > MAX_LEN || cnt < 1 || cnt > MAX_PASSWORDS || nsets == 0) {
                SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Invalid input. Check length, count, and at least one set.");
                break;
            }
            for (int i = 0; i < cnt; ++i) {
                if (!generate_password(password, len, use_lower, use_upper, use_digit, use_symbol)) {
                    ok = 0; break;
                }
                strcat(out, password);
                strcat(out, "\r\n");
            }
            SetWindowText(hEditOut, ok ? out : "Generation error");
            const char* strength = ok ? password_strength(len, nsets) : "";
            SetWindowText(hStaticStrength, strength);
            InvalidateRect(hStaticStrength, NULL, TRUE);
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)(ok ? "Generated!" : "Error generating password(s)"));
            // Save settings
            save_settings(len, cnt, use_lower, use_upper, use_digit, use_symbol);
        } else if (LOWORD(wParam) == ID_BUTTON_COPY) {
            char buf[4096];
            GetWindowText(hEditOut, buf, sizeof(buf));
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(buf)+1);
                memcpy(GlobalLock(hMem), buf, strlen(buf)+1);
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
                CloseClipboard();
            }
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Copied to clipboard!");
        } else if (LOWORD(wParam) == ID_ABOUT) {
            show_about(hwnd);
        }
        break;
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;
        if (hCtrl == hStaticStrength) {
            char buf[32];
            GetWindowText(hStaticStrength, buf, sizeof(buf));
            SetTextColor(hdc, strength_color(buf));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        break;
    }
    case WM_DESTROY:
        if (hFont) DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    case WM_SHOWWINDOW:
        if (wParam) center_window(hwnd);
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_CLOSE) {
            // Save settings on close
            char bufLen[16], bufCnt[16];
            int len, cnt, lower, upper, digit, symbol;
            GetWindowText(hEditLen, bufLen, sizeof(bufLen));
            GetWindowText(hEditCnt, bufCnt, sizeof(bufCnt));
            len = atoi(bufLen); cnt = atoi(bufCnt);
            lower = SendMessage(hCheckLower, BM_GETCHECK, 0, 0) == BST_CHECKED;
            upper = SendMessage(hCheckUpper, BM_GETCHECK, 0, 0) == BST_CHECKED;
            digit = SendMessage(hCheckDigit, BM_GETCHECK, 0, 0) == BST_CHECKED;
            symbol = SendMessage(hCheckSymbol, BM_GETCHECK, 0, 0) == BST_CHECKED;
            save_settings(len, cnt, lower, upper, digit, symbol);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            SendMessage(hButton, BM_CLICK, 0, 0);
        } else if (wParam == 'C' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            SendMessage(hwnd, WM_COMMAND, ID_BUTTON_COPY, 0);
        } else if (wParam == VK_F1) {
            show_about(hwnd);
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    srand((unsigned int)time(NULL));
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "PwdGenWin";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = (HICON)LoadImage(NULL, "C-icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("PwdGenWin", "Password Generator", WS_OVERLAPPEDWINDOW^WS_THICKFRAME, 100, 100, 350, 300, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) SendMessage(hwnd, WM_KEYDOWN, msg.wParam, msg.lParam);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

/*
 * Ultimate Win32 API GUI password generator:
 * - Modern font, group boxes, colored strength, centered window
 * - Tooltips, keyboard shortcuts, status bar, remembers last settings
 * - App icon (C-icon.ico)
 * - About dialog (F1 or menu)
 * - User can select char sets, generate multiple passwords, copy output
 * - Modular and easy to extend
 */ 