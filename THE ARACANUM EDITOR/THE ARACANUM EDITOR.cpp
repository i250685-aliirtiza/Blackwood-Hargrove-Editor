#include <windows.h>
#include"THE ARACANUM EDITOR.h"
#include<stdio.h>
#include<ctime>

unsigned long long start_time = time(NULL);
int cursor_width = 1;

//helper int to char, for words, total chars etc
void convertToStr(int num, wchar_t arr[]) {
    if (num == 0) {
        arr[0] = L'0';
        arr[1] = L'\0';
        return;
    }


    int i = 0;
    while (num) {
        arr[i] = num % 10 + 48;
        num /= 10;
        i++;
    }
    arr[i] = L'\0';

    //reverse
    for (int j = 0; j < i / 2; j++) {
        wchar_t temp = arr[j];
        arr[j] = arr[i - 1 - j];
        arr[i - 1 - j] = temp;
    }

}

int merge(const wchar_t* read, const wchar_t* read2, wchar_t* write) {
    int len = 0;
    while (*read) {
        *write = *read; len++;
        write++; read++;
    }
    while (*read2) {
        *write = *read2; len++;
        write++; read2++;
    }
    *write = '\0';


    return len;

}

void concatenate(const wchar_t* read1, const wchar_t* read2, const wchar_t* read3, wchar_t* write) {
    while (*read1) {
        *write = *read1;
        write++; read1++;
    }
    while (*read2) {
        *write = *read2;
        write++; read2++;
    }
    while (*read3) {
        *write = *read3;
        write++; read3++;
    }
    *write = L'\0';
}


Editor obj;

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
        obj.getLineHeight() = tm.tmHeight + tm.tmExternalLeading;
        SIZE size;
        GetTextExtentPoint32W(hdc, obj.getText(), obj.getLength(), &size);
        obj.getLineWidth() = size.cx; // width in pixels of len characters
        // Starting positions
        float columnSpacing = 14; // horizontal gap between columns

        bool filled = false;
        unsigned long long track = 0;
        float x = obj.getstartX(), y = obj.getstartY();
        int current_column = 0;
        float offset = 0;
        int p = obj.getPageIndex();

        // Loop over columns 
        for (int c = 0; c < obj.getTotalColumns(); c++) {
            if (filled) break;
            // Loop over lines
            for (int l = 0; l < obj.getTotalLines(); l++) {

                unsigned long long start = obj.getPages()[p].getColumns()[c].getLine()[l].start;
                int len = obj.getPages()[p].getColumns()[c].getLine()[l].len;

                if (start == -1 || len == 0)continue;
                offset = 0;

                // Calculate position
                x = obj.getstartX() + c * (obj.getLineLength() * columnSpacing);
                y = obj.getstartY() + l * obj.getLineHeight();


                TextOutW(hdc, x, y, obj.getText() + start, len);
                size;
                GetTextExtentPoint32W(hdc, obj.getText() + start, len, &size);
                obj.getLineWidth() = size.cx; // width in pixels of len characters
                obj.getcharWidth() = obj.getLineWidth() / len;

                //useful for cursor coordinates
                obj.getPages()[p].getColumns()[c].getLine()[l].screenY = y;
                obj.getPages()[p].getColumns()[c].getLine()[l].screenX = x;
                obj.getPages()[p].getColumns()[c].getLine()[l].offsetX = obj.getcharWidth();

                offset += (x + obj.getLineWidth());
                offset -= 10;//padding

                // update track
                track += len;
                if (track >= obj.getLength()) {
                    filled = true;
                    break;
                }
            }
        }



        //rendering cursor
        unsigned long long current_time = time(NULL);
        unsigned long long elapsed_time = current_time - start_time;

        offset = offset == 0 ? obj.getstartX() : offset;
        obj.getmaxX() = offset > obj.getmaxX() ? offset : obj.getmaxX();
        obj.getmaxY() = y > obj.getmaxY() ? y : obj.getmaxY();
        obj.getminY() = y < obj.getminY() ? y : obj.getminY();
        //showing periodically
        if (elapsed_time % 2 == 0) {

            RECT cursorRect = { obj.getcursorX(), obj.getcursorY(),obj.getcursorX() + cursor_width, obj.getcursorY() + obj.getLineHeight() };
            // RECT cursorRect = { obj.getmaxX(), obj.getmaxY(),obj.getmaxX() + cursor_width,obj.getmaxY() + obj.getLineHeight() };
            brush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &cursorRect, brush);
            DeleteObject(brush);
        }

        //SHOWING FOOTER

        int footerY = obj.getLineHeight() * obj.getTotalLines();
        footerY += obj.getstartY();
        //offset
        footerY += 30;

        // Create smaller font for footer
        HFONT footerFont = CreateFontW(
            14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Courier New"
        );

        // Switch to footer font
        HFONT oldFooterFont = (HFONT)SelectObject(hdc, footerFont);

        // Draw footer for total words, characters and characters without spaces
        wchar_t final[300];
        wchar_t info1[100];
        wchar_t info2[100];
        wchar_t info3[100];

        int len = 0;
        const wchar_t* str2 = L"Total Words: ";
        wchar_t arr[20];
        convertToStr(obj.getTotalWords(), arr);
        len += merge(str2, arr, info1);

        str2 = L"  Total Characters: ";
        convertToStr(obj.getLength(), arr);
        len += merge(str2, arr, info2);

        str2 = L"  Total Characters without sapaces: ";
        convertToStr(obj.getWithoutSp(), arr);
        len += merge(str2, arr, info3);


        concatenate(info1, info2, info3, final);

        TextOutW(hdc, obj.getstartX(), footerY, final, len);

        //also page numbering, showing under middle column.
        int val = obj.getTotalColumns() * obj.getLineLength();
        int temp = obj.getstartX();
        temp = val / 2;
        temp *= obj.getcharWidth();

        int page_numberX = temp;
        footerY -= 20;
        len = 0;
        str2 = L"------";
        convertToStr(obj.getPageIndex() + 1, arr);
        concatenate(str2, arr, str2, final);
        len = obj.getLen(final) - 1;
        TextOutW(hdc, page_numberX, footerY, final, len);


        // Restore original font
        SelectObject(hdc, oldFooterFont);

        // Delete footer font
        DeleteObject(footerFont);



        // Restore and cleanup
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:

        PostQuitMessage(0);
        return 0;

    case WM_CHAR: {
        // Backspace
        if (wParam == '\b') {

            //no need of deletion
            if (obj.getLength() <= 1) {
                obj.getLength() = 0;
                obj.getWithoutSp() = 0;
                obj.getText()[0] = '\0';
                obj.getTotalWords() = 0;
                obj.recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            //deletion required 
            else {
                //left shift logic
                if (obj.getInsert()) {
                    //IMPLEMENTATION NEEDED
                    //deleteAt(obj.getText(), length, cursor_to_obj.getText()_index);

                    obj.recalculateLayout();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                //other wise simple deletion from end
                else {
                    obj.getWithoutSp()--;
                    obj.getText()[obj.getLength() - 1] = '\0';
                    obj.getLength()--;
                    obj.recalculateLayout();
                    InvalidateRect(hwnd, NULL, FALSE);


                }

            }

        }
        // Enter key
        else if (wParam == '\r') {
            if (obj.getInsert()) {
                obj.insertAt(obj.getCursorIndex(), '\n');
            }
            else
                obj.append('\n');


            obj.recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);

        }
        // Printable characters
        else if (wParam >= 32 && wParam != 127) {
            if (wParam != ' ')obj.getWithoutSp()++;
            if (obj.getInsert()) {
                obj.insertAt(obj.getCursorIndex(), (wchar_t)wParam);
            }
            else
                obj.append((wchar_t)wParam);

            obj.recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);

        }
        return 0;
    }

    case WM_KEYDOWN: {
        if (wParam == VK_DELETE) {
            // Delete key pressed, same as backspace logic
            if (obj.getLength() > 0) {
                obj.getText()[obj.getLength() - 1] = '\0';
                obj.getLength()--;
                obj.recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
    case WM_TIMER: {
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
                 //right click(cursor placement)
    case WM_RBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        wprintf(L"x: %d ,  y: %d\n", x, y);
        //check if user is clicking apart from the allocated text place
        if (x > obj.getmaxX() || y > obj.getmaxY() || x < obj.getminX() || y < obj.getminY())return 0;

        //calculate cursor info from line data

        //find the x and y coordinates and fix them according to offset+ScreenX+screenY
        int p = obj.getPageIndex();
        bool foundX = false;
        bool foundY = false;
        for (int c = 0; c < obj.getTotalColumns(); c++) {
            for (int l = 0; l < obj.getTotalLines(); l++) {
                int screenY = obj.getPages()[p].getColumns()[c].getLine()[l].screenY;
                //check for y first
                if (screenY >= y - 10) {
                    obj.getcursorY() = screenY;
                    foundY = true;
                }
                //if y found approximate X
                if (foundY) {
                    int offset = obj.getPages()[p].getColumns()[c].getLine()[l].offsetX;
                    int screenX = obj.getPages()[p].getColumns()[c].getLine()[l].screenX;
                    for (int i = 0; i < obj.getPages()[p].getColumns()[c].getLine()[l].len; i++) {
                        //approximation wrt start of line
                        if (screenX + (obj.getcharWidth() * (i + 1)) <= x - 10) {
                            obj.getcursorX() = x;
                            if (obj.getcursorX() == obj.getmaxX())obj.getInsert() = false;
                            else {
                                obj.getInsert() = true;
                                //calculate index
                                obj.getCursorIndex() = obj.getPages()[p].getColumns()[c].getLine()[l].start;//start of line
                                //move from start of line wrt character width
                                int jump = 0;
                                int temp = obj.getPages()[p].getColumns()[c].getLine()[l].screenX;
                                while (temp <= obj.getcursorX()) {
                                    temp += obj.getcharWidth();
                                    jump += 1;
                                }
                                obj.getCursorIndex() += jump - 1;
                                                       
                                //   wprintf(L"index: %d\n", obj.getCursorIndex());

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