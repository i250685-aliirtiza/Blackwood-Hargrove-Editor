#include <windows.h>
#include"THE ARACANUM EDITOR.h"
#include<stdio.h>
#include<ctime>

unsigned long long start_time = time(NULL);

unsigned long long capacity = 100000;
wchar_t* text = new wchar_t[capacity];
unsigned long long length = 0;

int current_page = 0;//for displaying current page to user

//default
int line_length = 10;
int total_lines = 5;
int total_columns =4;
int total_pages = 1;
int startX = 40;       // left margin
int startY = 40;       // top margin

void append(wchar_t*& text, unsigned long long& index, unsigned long long& cap,wchar_t c) {
    if (index >= cap-1) {
        wchar_t* copy = new wchar_t[cap* 2];
        for (unsigned long long j = 0; j < index; j++) {
            copy[j] = text[j];
        }
        cap *= 2;
        delete[] text;
        text = copy;
    }

    text[index] = c;
    index++;
    text[index] = L'\0';
}

//layout
struct Line {
    unsigned long long start = -1;
    int len = 0;
};
struct column {
    Line* lines;

    column() { 
        lines = new Line[total_lines]; 
    }

    // Copy constructor
    column(const column& other) {
        lines = new Line[total_lines];
        for (int i = 0; i < total_lines; i++) {
            lines[i] = other.lines[i];
        }
    
    }

    column& operator=(const column& other){
        if (this == &other)return *this;
        delete[] this->lines;
        this->lines = new Line[total_lines];
        for (int i = 0; i < total_lines; i++) {
            lines[i] = other.lines[i];
        }

        return *this;
    
    }

    ~column() {
        delete[] lines;
    }
};

struct page {
    column* columns;

    page() { 
        columns = new column[total_columns];
    }

    // Copy constructor
    page(const page& other) {
        columns = new column[total_columns];
        for (int i = 0; i < total_columns; i++)
            columns[i] = other.columns[i];
        }
    //assignment operator
    page& operator=(const page& other) {
        if (this == &other)return *this;

        //delete old 
        delete[] this->columns;

        columns = new column[total_columns];
        for (int i = 0; i < total_columns; i++)
            this->columns[i] = other.columns[i];

        return *this;
    }


    ~page(){ 
        delete[] columns; 
    }
}; 


page* pages = new page[total_pages];
int page_index = 0; //valid pages

void resize(page*& pages, int& total_pages) {
    page* copy = new page[total_pages * 2];

    
    for (int i = 0; i <total_pages; i++) {
        copy[i] = pages[i];
    }
    delete[] pages;
    pages = copy;
    total_pages *= 2;
}

int getLen(wchar_t* ptr) {
    //if enter jumping to next line
    if (*ptr == '\n')return -1;

    int i = 0;
    while (*ptr && *ptr != ' ' && *ptr!='\n') {
        i++;
        ptr++;
    }

    i += 1;//for last space after word
    return i;
}



void debugLayout() {
    for (int p = 0; p <= page_index; p++) {
        wprintf(L"PAGE %d\n", p);
        for (int c = 0; c < total_columns; c++) {
            wprintf(L"  COLUMN %d\n", c);
            for (int l = 0; l < total_lines; l++) {
                Line& line = pages[p].columns[c].lines[l];
                if (line.start != -1) {
                    wchar_t buf[256];
                    wcsncpy_s(buf, text + line.start, line.len);
                    buf[line.len] = L'\0';
                    wprintf(L"    LINE %d: \"%s\"\n", l, buf);
                }
            }
        }
    }
}


//filling our layout
void recalculateLayout() {
    // Read through `text`, break it into lines/columns/pages
    unsigned long long track = 0; //iterating over text
    int p = 0;//represents current page
    int c = 0;//represents current column
    int l = 0; //represents current line
    int total = 0;
    while (true) {
        //DEBUGGING 
        
        //wprintf(L"\n=== LAYOUT DEBUG ===\n");
        //wprintf(L"Text: \"%s\"\n", text);
        //wprintf(L"Length: %llu\n\n", length);
        //wprintf(L"index: %d\n", track);
        //wprintf(L"index: %c\n", text[track]);
        //debugLayout();
        


       //text buffer filled
        if (track >= length) {
            page_index = p;
            return;
        }

        if (p >= total_pages) {
            resize(pages, total_pages);
        }
        //LINE WRAPPING
        //get len of word
        int len = getLen(&text[track]);

        //if -1 go to next line/column/page whatever is in place
        if (len == -1) {
            
            pages[p].columns[c].lines[l].len = total;
            //move to next line
            if (l < total_lines - 1) {
                l++; total = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                track++;
                continue;
            }
            else if (c < total_columns - 1) {
                c++; l = 0; total = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                track++;
                continue;
            }
            //page break
            else {
                p++;
                //if pages full
                if (p >= total_pages)resize(pages, total_pages);
                c = 0;
                l = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;
                total = 0;

                track++;
                continue;
            }
            
            continue;
        }
        
        //if a word is longer than line
        if (len > line_length) {


            if (pages[p].columns[c].lines[l].start == -1) {
                pages[p].columns[c].lines[l].start = track;
            }
            pages[p].columns[c].lines[l].len =line_length;
            track += line_length;//remaining word in next line
            total = 0;

            //move line
            if (l < total_lines - 1) {
                l++;
                //set starting index for new line, where old world wrapped
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;
            }
            else if (c < total_columns - 1) {
                c++;
                l = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;
            }
            else {
                p++;
                //if pages full
                if (p >= total_pages)resize(pages, total_pages);
                c = 0;
                l = 0;

                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;
            }

            continue;
        }

        total += len;
        //if it fits
        if (total<=line_length) {
            if (pages[p].columns[c].lines[l].start == -1) {
                pages[p].columns[c].lines[l].start = track;
            }
            pages[p].columns[c].lines[l].len = total;
            //move index
            track += len;
            continue;
        }
        //else wrap
        else {
            // Save current line without this word
            if (pages[p].columns[c].lines[l].start != -1) {
                pages[p].columns[c].lines[l].len = total-len;
            }

            
            if (l < total_lines - 1) {
                l++;
                //set starting index for new line, where old world wrapped
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;
     
                pages[p].columns[c].lines[l].start = track;
                total = 0;
                continue;
            }
            //else move to next column top
            else if (c<total_columns-1) {
                c++;
                l = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;
                continue;
            }
            //page break
            else {
                p++;
                //if pages full
                if (p >= total_pages)resize(pages, total_pages);
                c = 0;
                l = 0;

                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;
                continue;
            }

        }

    }

}



LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Clear background
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);

        // Set font
        HFONT font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Courier New");
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        // Rendering function

        // Get line height
        TEXTMETRICW tm;
        GetTextMetrics(hdc, &tm);
        int lineHeight = tm.tmHeight + tm.tmExternalLeading;
        SIZE size;
        GetTextExtentPoint32W(hdc, text, length, &size);
        int lineWidth = size.cx; // width in pixels of len characters
        // Starting positions
        float columnSpacing = 14; // horizontal gap between columns

        bool filled = false;
        unsigned long long track = 0;
        float x=startX, y=startY;
        int current_column = 0;
        float offset = 0;
        int p = page_index;

            // Loop over columns (0..total_columns-1)
            for (int c = 0; c < total_columns; c++) {
                if (filled) break;
                // Loop over lines
                for (int l = 0; l < total_lines; l++) {
                    
                    unsigned long long start = pages[p].columns[c].lines[l].start;
                    int len = pages[p].columns[c].lines[l].len;

                    if (start==-1 || len == 0)continue;
                    offset = 0;

                    // Calculate position
                    x = startX + c * (line_length*columnSpacing); 
                    y = startY + l * lineHeight;
                    current_column = c+1;

                    TextOutW(hdc, x, y, text + start, len);
                    size;
                    GetTextExtentPoint32W(hdc, text+start,len, &size);
                    int lineWidth = size.cx; // width in pixels of len characters
                    offset += (x+lineWidth);
                    offset -= 10;//padding

                    // update track
                    track += len;
                    if (track >= length) {
                        filled = true;
                        break;
                    }
                }
            }
        
        

        //rendering cursor
        unsigned long long current_time = time(NULL);
        unsigned long long elapsed_time = current_time - start_time;

        offset = offset == 0 ? startX : offset;
        //showing periodically
        if (elapsed_time % 2==0) {
            RECT cursorRect = { offset, y,offset +1, y + lineHeight };
            brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &cursorRect, brush);
            DeleteObject(brush);
        }
        // Restore and cleanup
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (text != nullptr) {
            delete[] text;
            text = nullptr;
        }
        PostQuitMessage(0);
        return 0;

    case WM_CHAR: {
        // Backspace
        if (wParam == '\b') {
            if (length > 0) {
                text[length - 1] = '\0';
                length--;
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE); 
            }
            else {
                text[0] = '\0';
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        // Enter key
        else if (wParam == '\r') {
            if (length < capacity) {
                append(text, length, capacity, '\n');
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        // Printable characters
        else if (wParam >= 32 && wParam != 127) {
            wprintf(L"Typed: %c\n", (wchar_t)wParam);  // ADD THIS LINE
            if (length < capacity) {
                append(text, length, capacity, (wchar_t)wParam);
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam==VK_DELETE) {
            // Delete key pressed, same as backspace logic
            if (length > 0) {
                text[length - 1] = '\0';
                length--;
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);        
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    //debugging helpers
    AllocConsole();                                 
    FILE* f;                                         
    freopen_s(&f, "CONOUT$", "w", stdout);
    ///////////////////////////////

    const wchar_t CLASS_NAME[] = L"FastianWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1 );
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Blackwood & Hargrove",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1440, 720,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, 1, 200, NULL); //for cursor tracking,it calls WM_TIMER

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}