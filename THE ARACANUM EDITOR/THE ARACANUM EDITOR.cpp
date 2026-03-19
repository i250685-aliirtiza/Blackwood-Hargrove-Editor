#include <windows.h>
#include"THE ARACANUM EDITOR.h"
#include<stdio.h>
#include<ctime>

unsigned long long start_time = time(NULL);

unsigned long long capacity = 10000;
wchar_t* text = new wchar_t[capacity];
unsigned long long length = 0;


int lineHeight=0, lineWidth=0;

//checking if we have to append or insert
bool insert = false;


int char_width = 1;


//tracking words
unsigned int words = 0;


//default
int line_length = 10;
int total_lines = 5;
int total_columns = 3;
int total_pages = 5;
int startX = 40;       // left margin
int startY = 40;       // top margin
int cursor_width = 1;

int cursorX = startX, cursorY = startY;//cursor coordinates
unsigned long long cursor_to_text_index = 0;
//represent the max valid | placement in text
int maxX = startX;
int maxY = startY;
int minX = startX;
int minY = startY;

void append(wchar_t*& text, unsigned long long& index, unsigned long long& cap, wchar_t c) {
    if (index >= cap - 1) {
        wchar_t* copy = new wchar_t[cap * 2];
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
    int screenY = startY; //storing y-coordinate of each line
    int screenX = startX;//diff for each char
    int offsetX = 1;//character width
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

    column& operator=(const column& other) {
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


    ~page() {
        delete[] columns;
    }
};


page* pages = new page[total_pages];
int page_index = 0; //current page setter

void resize(page*& pages, int& total_pages) {
    page* copy = new page[total_pages * 2];


    for (int i = 0; i < total_pages; i++) {
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
    while (*ptr && *ptr != ' ' && *ptr != '\n') {
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

    // Clear previous layout
    for (int p = 0; p <= page_index; p++) {
        for (int c = 0; c < total_columns; c++) {
            for (int l = 0; l < total_lines; l++) {
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;
            }
        }
    }
    unsigned long long track = 0; //iterating over text
    int p = 0;//represents current page
    int c = 0;//represents current column
    int l = 0; //represents current line
    int total = 0;//current chars on line 
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


        total += len;
        //if it fits
        if (total <= line_length) {
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
                pages[p].columns[c].lines[l].len = total - len;
            }
            else {
                // Line is empty,so truncate the word. fit it here and move the track
                pages[p].columns[c].lines[l].start = track;
                pages[p].columns[c].lines[l].len = line_length;

                len -= line_length;
                track += line_length;
                total = 0;
            }
            
            //MOVEMENT OF LINE/COLUMN/PAGE(whatever is required)
            if (l < total_lines - 1) {
                l++;
                //set starting index for new line, where old world wrapped
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;

            }
            //else move to next column top
            else if (c < total_columns - 1) {
                c++;
                l = 0;
                pages[p].columns[c].lines[l].start = -1;
                pages[p].columns[c].lines[l].len = 0;

                pages[p].columns[c].lines[l].start = track;
                total = 0;

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

            }

        }

        //SPECIAL CASE
        //if a word is longer than line
        while (len > line_length) {

            if (pages[p].columns[c].lines[l].start == -1) {
                pages[p].columns[c].lines[l].start = track;
            }
            pages[p].columns[c].lines[l].len = line_length;
            len -= line_length;
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


        }


    }

}




//getters
void getColumnAndLine(unsigned long long index,int* arr) {
    for (int p = 0; p <= page_index; p++) {
        for (int c = 0; c < total_columns; c++) {
            for (int l = 0; l < total_lines; l++) {
                if (index >= pages[p].columns[c].lines[l].start && index < pages[p].columns[c].lines[l].start + pages[p].columns[c].lines[l].len) {
                    arr[0] = c;
                    arr[1] = l;
                    break;
                }
            }
        }
    }
    
  }



//function for inserting inbetween our text buffer
void insertAt(wchar_t*& text, unsigned long long& index, unsigned long long& capacity, unsigned long long& i,wchar_t c) {
    //safety for index and capacity
    if (index >= capacity - 3) {
        wchar_t* copy = new wchar_t[capacity * 2];
        for (unsigned long long j = 0; j < index; j++) {
            copy[j] = text[j];
        }
        delete[] text;
        text = copy;
        capacity *= 2;
    }

   //shift text array from right
   //as text[index] is \0
    unsigned long long len = index + 1;
    text[len] = '\0';
    for (unsigned long long j = len-1; j >i; j--) {
        text[j] = text[j-1];
    }
    text[i] = c;
    index++;

    int arr[2];
    getColumnAndLine(i, arr);
    i++;
    int range = pages[page_index].columns[arr[0]].lines[arr[1]].start + pages[page_index].columns[arr[0]].lines[arr[1]].len-1;

    if(i<range && c!='\n')
    cursorX += char_width;


    else {
        if (arr[1] < total_lines - 1) {
            arr[1] += 1;
            cursorY = pages[page_index].columns[arr[0]].lines[arr[1]].screenY;
            cursorX = pages[page_index].columns[arr[0]].lines[arr[1]].screenX;
        }
        else if (arr[0] < total_columns - 1) {
            arr[0] += 1;
            arr[1] = 0;
            cursorY = pages[page_index].columns[arr[0]].lines[arr[1]].screenY;
            cursorX = pages[page_index].columns[arr[0]].lines[arr[1]].screenX;
        }
        else {
            cursorY = startY;
            cursorX = startX;
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
        lineHeight = tm.tmHeight + tm.tmExternalLeading;
        SIZE size;
        GetTextExtentPoint32W(hdc, text, length, &size);
        lineWidth = size.cx; // width in pixels of len characters
        // Starting positions
        float columnSpacing = 14; // horizontal gap between columns

        bool filled = false;
        unsigned long long track = 0;
        float x = startX, y = startY;
        int current_column = 0;
        float offset = 0;
        int p = page_index;

        // Loop over columns 
        for (int c = 0; c < total_columns; c++) {
            if (filled) break;
            // Loop over lines
            for (int l = 0; l < total_lines; l++) {

                unsigned long long start = pages[p].columns[c].lines[l].start;
                int len = pages[p].columns[c].lines[l].len;

                if (start == -1 || len == 0)continue;
                offset = 0;

                // Calculate position
                x = startX + c * (line_length * columnSpacing);
                y = startY + l * lineHeight;
    



                TextOutW(hdc, x, y, text + start, len);
                size;
                GetTextExtentPoint32W(hdc, text + start, len, &size);
                int lineWidth = size.cx; // width in pixels of len characters
                char_width = lineWidth / len;

                //useful for cursor coordinates
                pages[p].columns[c].lines[l].screenY = y;
                pages[p].columns[c].lines[l].screenX = x;
                pages[p].columns[c].lines[l].offsetX = char_width;

                offset += (x + lineWidth);
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
        maxX = offset > maxX ? offset : maxX;
        maxY = y > maxY ? y : maxY;
        minY = y < minY ? y : minY;
        //showing periodically
        if (elapsed_time % 2 == 0) {
            
            RECT cursorRect = { cursorX, cursorY,cursorX + cursor_width, cursorY + lineHeight };
           // RECT cursorRect = { maxX, maxY,maxX + cursor_width,maxY + lineHeight };
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

            if (length > 1) {
                if (text[length - 1] != ' ' && text[length - 2] == ' ')words--;

                text[length - 1] = '\0';
                length--;
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else {
                text[0] = '\0';
                words = 0;
                recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        // Enter key
        else if (wParam == '\r') {
            if (insert) {
                insertAt(text, length, capacity, cursor_to_text_index, '\n');
            }
            else
            append(text, length, capacity, '\n');


            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);

        }
        // Printable characters
        else if (wParam >= 32 && wParam != 127) {
            if (wParam == ' ' && length >= 2 && text[length - 1] != ' ')words++;

            if (insert) {
                insertAt(text, length, capacity,cursor_to_text_index ,(wchar_t)wParam);
            }
            else
            append(text, length, capacity, (wchar_t)wParam);

            recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);

        }
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_DELETE) {
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
    case WM_TIMER:{
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    //right click(cursor placement)
    case WM_RBUTTONDOWN:{
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        wprintf(L"x: %d ,  y: %d\n", x, y);
        //check if user is clicking apart from the allocated text place
        if (x > maxX || y > maxY || x<minX || y<minY)return 0;
        
        //calculate cursor info from line data

        //find the x and y coordinates and fix them according to offset+ScreenX+screenY
        int p = page_index;
        bool foundX = false;
        bool foundY = false;
        for (int c = 0; c < total_columns; c++) {
            for (int l = 0; l < total_lines; l++) {
                int screenY = pages[p].columns[c].lines[l].screenY;
                //check for y first
                if (screenY>= y-10) {
                    cursorY = screenY;
                    foundY = true;
                }
                //if y found approximate X
                if (foundY) {             
                    int offset =pages[p].columns[c].lines[l].offsetX;
                    int screenX = pages[p].columns[c].lines[l].screenX;
                    for (int i = 0; i < pages[p].columns[c].lines[l].len; i++) {
                   
                        if (screenX + (offset * (i + 1)) <= x-10) {
                            cursorX =x;
                            if (cursorX >= maxX - 2)insert = false;
                            else {
                                insert = true;
                                //calculate index
                                cursor_to_text_index=pages[p].columns[c].lines[l].start;//start of line
                                //move wrt character
                                int jump = 0;
                                int temp = pages[p].columns[c].lines[l].screenX;
                                while (temp <= cursorX) {
                                    temp += char_width;
                                    jump += 1;
                                }
                                cursor_to_text_index += jump-1;
                             //   wprintf(L"index: %d\n", cursor_to_text_index);
                                
                            }
                            foundX = true; break;
                        }
                    }
                }
                if (foundY && foundX)break;
            }
            if (foundY && foundX)break;
        }

        return 0;
    }
    //left click
    case WM_LBUTTONDOWN:
    {
        wprintf(L"LEFT CLICK!\n");
    }
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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