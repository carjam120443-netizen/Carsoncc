#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDC_EDITOR 1001
#define IDC_OUTPUT 1002
#define IDC_OPEN 1003
#define IDC_SAVE 1004
#define IDC_BUILD 1005
#define IDC_RUN 1006
#define IDC_BUILD_RUN 1007
#define IDC_TARGET 1008
#define IDC_STATUS 1009

static HINSTANCE g_instance;
static HWND g_editor, g_output, g_target, g_status;
static char g_file[MAX_PATH] = "untitled.car";
static char g_compiler[MAX_PATH] = "carsoncc.exe";

static void set_text(HWND h, const char *s) {
    SetWindowTextA(h, s ? s : "");
}

static void append_output(const char *s) {
    int n = GetWindowTextLengthA(g_output);
    SendMessageA(g_output, EM_SETSEL, (WPARAM)n, (LPARAM)n);
    SendMessageA(g_output, EM_REPLACESEL, FALSE, (LPARAM)(s ? s : ""));
}

static void status(const char *s) {
    set_text(g_status, s);
}

static char *editor_text(void) {
    int n = GetWindowTextLengthA(g_editor);
    char *s = (char *)malloc((size_t)n + 1);
    if (!s) return NULL;
    GetWindowTextA(g_editor, s, n + 1);
    return s;
}

static int write_text_file(const char *path, const char *text) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    size_t n = strlen(text);
    int ok = fwrite(text, 1, n, f) == n;
    fclose(f);
    return ok;
}

static int read_text_file(const char *path, char **out) {
    FILE *f = fopen(path, "rb");
    long n;
    char *buf;
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    n = ftell(f);
    if (n < 0) { fclose(f); return 0; }
    rewind(f);
    buf = (char *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return 0; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf); fclose(f); return 0;
    }
    buf[n] = 0;
    fclose(f);
    *out = buf;
    return 1;
}

static void save_as_dialog(void) {
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetParent(g_editor);
    ofn.lpstrFilter = "Carson source (*.car)\0*.car\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = g_file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (GetSaveFileNameA(&ofn)) {
        char *text = editor_text();
        if (text && write_text_file(g_file, text)) {
            status("Saved.");
        } else {
            MessageBoxA(NULL, "Could not save the source file.", "CarsonCC", MB_ICONERROR);
        }
        free(text);
    }
}

static void open_dialog(void) {
    OPENFILENAMEA ofn;
    char path[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "Carson source (*.car)\0*.car\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        char *text = NULL;
        if (read_text_file(path, &text)) {
            SetWindowTextA(g_editor, text);
            strncpy(g_file, path, MAX_PATH - 1);
            g_file[MAX_PATH - 1] = 0;
            status("Source opened.");
            free(text);
        } else {
            MessageBoxA(NULL, "Could not open the source file.", "CarsonCC", MB_ICONERROR);
        }
    }
}

static void output_name(char *out, size_t size) {
    char tmp[MAX_PATH];
    strncpy(tmp, g_file, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = 0;
    char *dot = strrchr(tmp, '.');
    if (dot) *dot = 0;
    snprintf(out, size, "%s.exe", tmp);
}

static int run_process(const char *command, DWORD *exit_code) {
    SECURITY_ATTRIBUTES sa;
    HANDLE read_pipe = NULL, write_pipe = NULL;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char *cmdline;
    char buffer[4096];
    DWORD read_n;
    size_t len = strlen(command);

    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) return 0;
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = write_pipe;
    si.hStdError = write_pipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    ZeroMemory(&pi, sizeof(pi));
    cmdline = (char *)malloc(len + 1);
    if (!cmdline) {
        CloseHandle(read_pipe); CloseHandle(write_pipe); return 0;
    }
    memcpy(cmdline, command, len + 1);

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW,
                        NULL, NULL, &si, &pi)) {
        free(cmdline);
        CloseHandle(read_pipe); CloseHandle(write_pipe);
        return 0;
    }
    free(cmdline);
    CloseHandle(write_pipe);

    while (ReadFile(read_pipe, buffer, sizeof(buffer) - 1, &read_n, NULL) && read_n) {
        buffer[read_n] = 0;
        append_output(buffer);
    }
    CloseHandle(read_pipe);
    WaitForSingleObject(pi.hProcess, INFINITE);
    if (exit_code) GetExitCodeProcess(pi.hProcess, exit_code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 1;
}

static int save_current(void) {
    char *text = editor_text();
    int ok = text && write_text_file(g_file, text);
    free(text);
    if (!ok) MessageBoxA(NULL, "Save failed.", "CarsonCC", MB_ICONERROR);
    return ok;
}

static int build_project(void) {
    char exe[MAX_PATH], command[3 * MAX_PATH + 128];
    DWORD code = 1;

    if (!save_current()) return 0;
    output_name(exe, sizeof(exe));
    append_output("\r\n> Building for Windows x64...\r\n");
    snprintf(command, sizeof(command),
             "\"%s\" \"%s\" -target windows -o \"%s\"",
             g_compiler, g_file, exe);

    if (!run_process(command, &code)) {
        append_output("Could not start carsoncc.exe. Put it beside CarsonCC.exe or on PATH.\r\n");
        status("Compiler not found.");
        return 0;
    }
    if (code == 0) {
        append_output("Build succeeded.\r\n");
        status("Build succeeded.");
        return 1;
    }
    append_output("Build failed.\r\n");
    status("Build failed.");
    return 0;
}

static void run_project(void) {
    char exe[MAX_PATH], command[2 * MAX_PATH + 32];
    DWORD code = 1;
    output_name(exe, sizeof(exe));

    append_output("\r\n> Running program...\r\n");
    snprintf(command, sizeof(command), "\"%s\"", exe);
    if (!run_process(command, &code)) {
        append_output("Could not start the generated executable. Build it first.\r\n");
        status("Run failed.");
        return;
    }
    {
        char msg[128];
        snprintf(msg, sizeof(msg), "Program exited with code %lu.\r\n", (unsigned long)code);
        append_output(msg);
    }
    status("Program finished.");
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HFONT font = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, FF_MODERN, "Consolas");
        g_editor = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT",
            "fn main() {\r\n    let answer = 10 + 20 * 2;\r\n    return answer;\r\n}\r\n",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
            ES_AUTOVSCROLL | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_EDITOR,
            g_instance, NULL);
        g_output = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "CarsonCC Terminal\r\n",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY |
            ES_AUTOVSCROLL, 0, 0, 0, 0, hwnd, (HMENU)IDC_OUTPUT, g_instance, NULL);
        g_target = CreateWindowExA(0, "STATIC", "Target: Windows x64",
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_TARGET, g_instance, NULL);
        g_status = CreateWindowExA(0, "STATIC", "Ready.",
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS, g_instance, NULL);

        CreateWindowA("BUTTON", "Open", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      8, 8, 80, 28, hwnd, (HMENU)IDC_OPEN, g_instance, NULL);
        CreateWindowA("BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      94, 8, 80, 28, hwnd, (HMENU)IDC_SAVE, g_instance, NULL);
        CreateWindowA("BUTTON", "Build", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      180, 8, 80, 28, hwnd, (HMENU)IDC_BUILD, g_instance, NULL);
        CreateWindowA("BUTTON", "Run", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      266, 8, 80, 28, hwnd, (HMENU)IDC_RUN, g_instance, NULL);
        CreateWindowA("BUTTON", "Build & Run", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                      352, 8, 105, 28, hwnd, (HMENU)IDC_BUILD_RUN, g_instance, NULL);

        SendMessageA(g_editor, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageA(g_output, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageA(g_status, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageA(g_target, WM_SETFONT, (WPARAM)font, TRUE);
        return 0;
    }
    case WM_SIZE: {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        int top = 48, bottom = 28, gap = 8;
        int left_w = (w - gap) * 3 / 5;
        MoveWindow(g_editor, 8, top, left_w - 12, h - top - bottom, TRUE);
        MoveWindow(g_output, left_w + 4, top, w - left_w - 12, h - top - bottom, TRUE);
        MoveWindow(g_target, 470, 12, 150, 24, TRUE);
        MoveWindow(g_status, 8, h - 24, w - 16, 20, TRUE);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OPEN: open_dialog(); break;
        case IDC_SAVE:
            if (save_current()) status("Saved.");
            break;
        case IDC_BUILD: build_project(); break;
        case IDC_RUN: run_project(); break;
        case IDC_BUILD_RUN:
            if (build_project()) run_project();
            break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc;
    HWND hwnd;
    MSG msg;
    (void)hPrev; (void)lpCmdLine;

    g_instance = hInstance;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CarsonCCWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassA(&wc)) return 1;

    hwnd = CreateWindowExA(0, "CarsonCCWindow", "CarsonCC - Carson Compiler",
                           WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           1100, 700, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
