#include <windows.h>
#include <stdio.h>
#include <ctime>

unsigned long long start_time = time(NULL);

unsigned long long capacity = 100000;
wchar_t* text = new wchar_t[capacity];
unsigned long long length = 0;

// Layout defaults
int line_length = 10;
int total_lines = 5;
int total_columns = 4;
int total_pages = 1;
int page_index = 0;

int startX = 40;
int startY = 40;

// Append character safely
void append(wchar_t*& text, unsigned long long& index, unsigned long long& cap, wchar_t c) {
    if (index >= cap - 1) {
        wchar_t* copy = new wchar_t[cap * 2];
        for (unsigned long long j = 0; j < index; j++)
            copy[j] = text[j];
        cap *= 2;
        delete[] text;
        text = copy;
    }
    text[index++] = c;
    text[index] = L'\0';
}

// Layout structures
struct Line {
    long long start = -1;
    int len = 0;
};

struct Column {
    Line* lines;
    Column() { lines = new Line[total_lines]; }
    Column(const Column& other) {
        lines = new Line[total_lines];
        for (int i = 0; i < total_lines; i++) lines[i] = other.lines[i];
    }
    ~Column() { delete[] lines; }
};

struct Page {
    Column* columns;
    Page() { columns = new Column[total_columns]; }
    Page(const Page& other) {
        columns = new Column[total_columns];
        for (int i = 0; i < total_columns; i++) columns[i] = other.columns[i];
    }
    ~Page() { delete[] columns; }
};

Page* pages = new Page[total_pages];

// Resize pages safely
void resizePages() {
    Page* copy = new Page[total_pages * 2];
    for (int i = 0; i < total_pages; i++) copy[i] = pages[i];
    delete[] pages;
    pages = copy;
    total_pages *= 2;
}

// Get length of word or -1 for newline
int getLen(wchar_t* ptr) {
    if (*ptr == '\n') return -1;
    int i = 0;
    while (*ptr && *ptr != ' ' && *ptr != '\n') { i++; ptr++; }
    return i + 1;
}

// Debug layout
void debugLayout() {
    for (int p = 0; p <= page_index; p++) {
        wprintf(L"PAGE %d\n", p);
        for (int c = 0; c < total_columns; c++) {
            wprintf(L"  COLUMN %d\n", c);
            for (int l = 0; l < total_lines; l++) {
                Line& line = pages[p].columns[c].lines[l];
                if (line.start != -1 && line.len > 0) {
                    wchar_t buf[256];
                    wcsncpy_s(buf, text + line.start, line.len);
                    buf[line.len] = L'\0';
                    wprintf(L"    LINE %d: \"%s\"\n", l, buf);
                }
            }
        }
    }
}

// Recalculate layout safely
void recalculateLayout() {
    unsigned long long track = 0;
    int p = 0, c = 0, l = 0, total = 0;

    while (track < length) {
        if (p >= total_pages) resizePages();

        int len = getLen(&text[track]);

        if (len == -1) { // newline
            pages[p].columns[c].lines[l].len = total;
            track++;
            total = 0; l++;
            if (l >= total_lines) { l = 0; c++; }
            if (c >= total_columns) { c = 0; p++; }
            continue;
        }

        if (len > line_length) { // word too long
            if (pages[p].columns[c].lines[l].start == -1)
                pages[p].columns[c].lines[l].start = track;
            pages[p].columns[c].lines[l].len = line_length;
            track += line_length;
            total = 0; l++;
            if (l >= total_lines) { l = 0; c++; }
            if (c >= total_columns) { c = 0; p++; }
            continue;
        }

        total += len;
        if (total <= line_length) {
            if (pages[p].columns[c].lines[l].start == -1)
                pages[p].columns[c].lines[l].start = track;
            pages[p].columns[c].lines[l].len = total;
            track += len;
        }
        else { // wrap line
            pages[p].columns[c].lines[l].len = total - len;
            total = 0; l++;
            if (l >= total_lines) { l = 0; c++; }
            if (c >= total_columns) { c = 0; p++; }
        }
    }

    page_index = p;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        HFONT font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Courier New");
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        TEXTMETRICW tm;
        GetTextMetrics(hdc, &tm);
        int lineHeight = tm.tmHeight + tm.tmExternalLeading;
        float columnSpacing = 14;

        float x = startX, y = startY;
        int p = page_index;

        for (int c = 0; c < total_columns; c++) {
            for (int l = 0; l < total_lines; l++) {
                if (p >= total_pages) break;
                Line& line = pages[p].columns[c].lines[l];
                if (line.start == -1 || line.len == 0) continue;

                x = startX + c * (line_length * columnSpacing);
                y = startY + l * lineHeight;
                TextOutW(hdc, x, y, text + line.start, line.len);
            }
        }

        // cursor blink
        unsigned long long elapsed_time = time(NULL) - start_time;
        if (elapsed_time % 2 == 0) {
            RECT cursorRect = { (int)x, (int)y, (int)x + 1, (int)y + lineHeight };
            brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &cursorRect, brush);
            DeleteObject(brush);
        }

        SelectObject(hdc, oldFont);
        DeleteObject(font);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        delete[] text;
        PostQuitMessage(0);
        return 0;

    case WM_CHAR:
        if (wParam == '\b') {
            if (length > 0) text[--length] = L'\0';
            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (wParam == '\r') {
            append(text, length, capacity, L'\n');
            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        else if (wParam >= 32 && wParam != 127) {
            append(text, length, capacity, (wchar_t)wParam);
            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_DELETE && length > 0) {
            text[--length] = L'\0';
            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    const wchar_t CLASS_NAME[] = L"AracanumEditor";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Aracanum Editor",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1440, 720,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 200, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}