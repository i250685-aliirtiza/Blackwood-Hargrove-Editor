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

//converting screen x y click to valid buffer index
void generateCursorInfo(int x, int y,unsigned long long& index, int& cursorX, int& cursorY) {

    //find line 
    int activeLine = 0, activePage = 0, activeColumn = 0;
    bool filled = false;
    for (int p = 0; p <=obj.getPageIndex(); p++) {
        for (int c = 0; c < obj.getTotalColumns(); c++) {
            for (int l = 0; l < obj.getTotalLines(); l++) {
                int screenY = obj.getPages()[p].getColumns()[c].getLine()[l].screenY;
                int screenX = obj.getPages()[p].getColumns()[c].getLine()[l].screenX;
                wprintf(L"screenY: %d\n", screenY);
                if (y>=screenY && y<=screenY+obj.getLineHeight() && !filled){
                    activeLine = l;
                    cursorY = screenY;
                    activePage = p;
                    filled = true;
                    break;
                }

            }
            if (filled)break;

        }
        if (filled)break;
    }

    //find active column
    for (int c = 0; c < obj.getTotalColumns(); c++) {
        int min = obj.getPages()[activePage].getColumns()[c].getLine()[activeLine].screenX;
        int max = min + obj.getLineLength() * obj.getcharWidth();
        if (x>=min && x<=max) {
            activeColumn = c;
            break;
        }
    }

    //approximate char index
    int screenX = obj.getPages()[activePage].getColumns()[activeColumn].getLine()[activeLine].screenX;
    int temp = screenX;
    int jump = 0;
    int len = obj.getPages()[activePage].getColumns()[activeColumn].getLine()[activeLine].len;

    while (jump < len && temp<x) {
        temp += obj.getcharWidth();
        jump++;
    }
    //we have line, column, page and offset. generate index wrt text buffer
    int start = obj.getPages()[obj.getPageIndex()].getColumns()[activeColumn].getLine()[activeLine].start;
    index = start + jump;
    cursorX = temp;
  //  wprintf(L"x: %d , y: %d , index: %d , cursorX: %d , cursorY= %d\n",x,y,index,cursorX,cursorY);
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
        obj.getLineHeight() = tm.tmHeight + tm.tmExternalLeading;
        SIZE size;
        GetTextExtentPoint32W(hdc, obj.getText(), obj.getLength(), &size);
        obj.getLineWidth() = size.cx; // width in pixels of len characters
        // Starting positions
        float columnSpacing = 14; // horizontal gap between columns

        bool filled = false;
        unsigned long long track = 0;
        float x = obj.getstartX(), y = obj.getstartY();
       


        //SHOW PAGE
        int p = obj.getPageIndex();
        int x_chars = 0;
        // Loop over columns 
        for (int c = 0; c < obj.getTotalColumns(); c++) {
            if (filled) break;
            // Loop over lines
            for (int l = 0; l < obj.getTotalLines(); l++) {


                unsigned long long start = obj.getPages()[p].getColumns()[c].getLine()[l].start;
                int len = obj.getPages()[p].getColumns()[c].getLine()[l].len;
                x_chars = len == 0 ? x_chars : len;
                if (start == -1 || len == 0)continue;


                // Calculate position
                x = obj.getstartX() + c * (obj.getLineLength() * columnSpacing);
                y = obj.getstartY() + l * obj.getLineHeight();
                
                //printing the line char by char if it is marked
                bool isMarked = false;
                //search for index of current line in highlights
                for (int i = 0; i < obj.getMarkedIndex(); i++) {
                    unsigned long long tempStart = obj.getMarkedText()[i].start;
                    int tempLen = obj.getMarkedText()[i].len;
                    if (tempStart >= start && tempLen < start + len) {
                        isMarked = true;
                        break;
                    }
                }

                if (isMarked) {
                    unsigned long long lineStart = start;
                    unsigned long long lineEnd = start + len;

                    unsigned long long current = lineStart;

                    //check all marked_text indices
                    for (int i = 0; i < obj.getMarkedIndex(); i++) {
                        unsigned long long markStart = obj.getMarkedText()[i].start;
                        int markLen = obj.getMarkedText()[i].len;
                        unsigned long long markEnd = markStart + markLen;

                        // skip if highligh is on another line
                        if (markEnd <= lineStart || markStart >= lineEnd) continue;

                        //printing normal before marked
                        if (markStart > current) {
                            int blackLen = markStart - current;
                            SetTextColor(hdc, RGB(0, 0, 0));
                            TextOutW(hdc, x, y, obj.getText() + current, blackLen);
                            SIZE size;
                            GetTextExtentPoint32W(hdc, obj.getText() + current, blackLen, &size);
                            x += size.cx;
                        }

                        // print the marked part
                        unsigned long long highlightedStart = current > markStart ? current : markStart;
                        int highlightedEnd = lineEnd < markEnd ? lineEnd : markEnd;
                        int highlightLen = highlightedEnd - highlightedStart;

                        SetTextColor(hdc, RGB(255, 0, 0)); // red for marked
                        TextOutW(hdc, x, y, obj.getText() + highlightedStart, highlightLen);

                        // update x-position
                        SIZE size;
                        GetTextExtentPoint32W(hdc, obj.getText() + highlightedStart, highlightLen, &size);
                        x += size.cx;

                        current = highlightedEnd;
                    }

                    // print remaining unmarked part after last marked region
                    if (current < lineEnd) {
                        int remainingLen = lineEnd - current;
                        SetTextColor(hdc, RGB(0, 0, 0)); // black
                        TextOutW(hdc, x, y, obj.getText() + current, remainingLen);
                        SIZE size;
                        GetTextExtentPoint32W(hdc, obj.getText() + current, remainingLen, &size);
                        x += size.cx;
                    }

                    obj.getLineWidth() = x - size.cx;
                }
                //if not marked
                else {
                    //check if start lies in our selected range
                    if (obj.getselectionStart() >= start && obj.getselectionEnd() <= start + len) {
                        SetTextColor(hdc, RGB(0, 0, 255)); // blue for selected                   
                    }
                    else {
                        SetTextColor(hdc, RGB(0, 0, 0)); // black for normal
                    }

                    TextOutW(hdc, x, y, obj.getText() + start, len);
                }
                size;
                GetTextExtentPoint32W(hdc, obj.getText() + start, len, &size);
                obj.getLineWidth() = size.cx; // width in pixels of len characters
                obj.getcharWidth() = obj.getLineWidth() / len;

                //useful for cursor coordinates
                obj.getPages()[p].getColumns()[c].getLine()[l].screenY = y;
                obj.getPages()[p].getColumns()[c].getLine()[l].screenX = x;

                // update track
                track += len;
                if (track >= obj.getLength()) {
                    filled = true;
                    break;
                }
            }
        }
        //showing search bar at top
        int searchX = obj.getstartX();
        int searchY = 0;

        wchar_t searchTEXT[500];
        int searchLen = merge(L"search(CTRL+F, ESC, ENTER): ", obj.getSearchText(), searchTEXT);
        SetTextColor(hdc, RGB(255, 0, 0)); //red
        TextOutW(hdc, searchX, searchY, searchTEXT, searchLen);

        SetTextColor(hdc, RGB(0, 0, 0)); // black for normal



        //place at the end
        obj.getmaxCursorX() = x + ((x_chars)*obj.getcharWidth());
        obj.getmaxCursorY() = y;

        //rendering cursor
        unsigned long long current_time = time(NULL);
        unsigned long long elapsed_time = current_time - start_time;

          //showing periodically
        if (elapsed_time % 2 == 0) {
            RECT cursorRect;
      
            cursorRect = { obj.getcursorX(), obj.getcursorY(),obj.getcursorX() + cursor_width, obj.getcursorY() + obj.getLineHeight() };

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

        str2 = L" | Total Characters: ";
        convertToStr(obj.getLength(), arr);
        len += merge(str2, arr, info2);

        str2 = L"  | Total Characters without sapaces: ";
        convertToStr(obj.getWithoutSp(), arr);
        len += merge(str2, arr, info3);


        concatenate(info1, info2, info3, final);

        //add sentences count
        str2 = L" | Total Sentences: ";
        convertToStr(obj.getSentences(), arr);
        wchar_t final2[400];

        wchar_t info4[100];
        len += merge(str2, arr, info4);
        len = 0;
        len += merge(final, info4, final2);

        SetTextColor(hdc, RGB(0, 0, 0)); // black for normal

        TextOutW(hdc, obj.getstartX(), footerY, final2, len);

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

   

        //saving file
        if (wParam == 19) {
            bool val=obj.saveFile("dummy.txt");
            break;
        }
        //opening file
        if (wParam == 15) {
            bool val = obj.loadFile("sample-text_for_editor.txt");
            obj.recalculateLayout();
            //display latest page
            obj.getPageIndex()=obj.getMaxPage();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        
        //if ctrl+F pressed
        if (wParam == 6) {
            obj.getSearchState() = true;
        }

       
        //implementing search text interception and backspace handling. 
        if (obj.getSearchState()) {
            // ESC pressed, cancel search, clear the text in it
            if (wParam == 27) {
                //clear the marked indices
                obj.clear_highlights();
                obj.getSearchState() = false;
                obj.getSearchTextIndex() = 0;
                obj.getSearchText()[obj.getSearchTextIndex()] = L'\0';
             }
            // Printable characters
            else if (wParam >= 32 && wParam != 127) {
                //don't take a word larger than 20 letters
                if (obj.getSearchTextIndex() >= 20)break;

                obj.getSearchText()[obj.getSearchTextIndex()] = wParam;
                obj.getSearchTextIndex()++;
                obj.getSearchText()[obj.getSearchTextIndex()] = L'\0';

            }
            //backspace
            else if (wParam == '\b') {
                obj.getSearchText()[obj.getSearchTextIndex()] = L'\0';

                if(obj.getSearchTextIndex()!=0)
                     obj.getSearchTextIndex()--;
            }
            //enter pressed execute search
            else if (wParam == '\r') {
                wprintf(L"Searching.....\n");
                obj.executeSearch();
                InvalidateRect(hwnd, NULL, FALSE);

            }
 
        }
        //ONLY RUN THESE OPERATIONS IF NOT IN SEARCH BOX    
        else {
            //1 is ascii for ctrl+A
            if (wParam == 1) {
                obj.selectAll();
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            //PASTE TEXT
            //22 is ascii for ctrl+V
            if (wParam == 22) {
                // Access clipboard and get text into wchar_t* temp
                if (OpenClipboard(hwnd))
                {
                    HANDLE hData = GetClipboardData(CF_UNICODETEXT); // get clipboard data
                    if (hData)
                    {
                        wchar_t* pText = (wchar_t*)GlobalLock(hData); // lock the memory to get pointer
                        if (pText)
                        {
                            //find length
                            int  len = 0;
                            wchar_t* read = pText;
                            while (*read) {
                                read++; len++;
                            }
                            // Allocate array to store clipboard content
                            wchar_t* temp = new wchar_t[len + 1];

                            // Copy clipboard content
                            for (size_t i = 0; i <= len; i++)
                            {
                                temp[i] = pText[i];
                            }

                            //if text selected
                            if (obj.getselectionState()) {
                                //delete selected text
                                obj.deletePortion();
                                InvalidateRect(hwnd, NULL, FALSE);
                            }
                            //insert to buffer
                            for (int i = 0; i < len; i++) {
                                obj.insertAt(obj.getCursorIndex(), temp[i]);
                            }
                            obj.recalculateLayout();
                            obj.getPageIndex() = obj.findCurrentPage();
                            InvalidateRect(hwnd, NULL, FALSE);
                            delete[] temp;
                        }
                    }
                    CloseClipboard();
                }
                break;
            }

            //3 i ascii for ctrl +c
            if (wParam == 3) {
                if (OpenClipboard(hwnd))
                {
                    EmptyClipboard();


                    int len = obj.getselectionStart() - obj.getselectionEnd() + 1;// number of characters (without null)
                    if (len <= 0) {
                        break;
                    }


                    // 2. Allocate memory (include null terminator)
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));

                    if (hMem)
                    {
                        // 3. Copy data into allocated memory
                        wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
                        unsigned long long index = 0;
                        for (unsigned long long i = obj.getselectionEnd(); i <= obj.getselectionStart(); i++) {
                            pMem[index] = obj.getText()[i];
                            wprintf(L"%c", pMem[index]);
                            index++;
                        }
                        pMem[index] = L'\0';

                        GlobalUnlock(hMem);

                        // 4. Send to clipboard
                        SetClipboardData(CF_UNICODETEXT, hMem);
                    }

                    CloseClipboard();
                }
            }
            // Backspace
            else if (wParam == '\b') {

                //no need of deletion
                if (obj.getLength() <= 1) {
                    obj.getCursorIndex() = 0;
                    obj.getLength() = 0;
                    obj.getWithoutSp() = 0;
                    obj.getText()[0] = '\0';
                    obj.getTotalWords() = 0;
                    obj.recalculateLayout();

                }
                else {
                    //deletion required 

                     //clear selected area
                    if (obj.getselectionState()) {
                        obj.deletePortion();
                        obj.getselectionState() = false;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);

                    obj.deleteBack(obj.getCursorIndex());
                    obj.recalculateLayout();
                    obj.getPageIndex() = obj.findCurrentPage();
                }

                InvalidateRect(hwnd, NULL, FALSE);
            }
            // Enter key
            else if (wParam == '\r') {
                //clear selected area
                if (obj.getselectionState()) {
                    obj.deletePortion();
                    obj.getselectionState() = false;
                }
                InvalidateRect(hwnd, NULL, FALSE);

                obj.insertAt(obj.getCursorIndex(), '\n');
                obj.recalculateLayout();
                obj.getPageIndex() = obj.findCurrentPage();

                InvalidateRect(hwnd, NULL, FALSE);

            }
            // Printable characters
            else if (wParam >= 32 && wParam != 127) {
                //clear selected area
                if (obj.getselectionState()) {
                    obj.deletePortion();
                    obj.getselectionState() = false;
                }
                InvalidateRect(hwnd, NULL, FALSE);

                obj.insertAt(obj.getCursorIndex(), (wchar_t)wParam);

                obj.recalculateLayout();
                obj.getPageIndex() = obj.findCurrentPage();

                InvalidateRect(hwnd, NULL, FALSE);

            }
        }
        
        
        return 0;
    
}


    case WM_KEYDOWN: {

        if(!obj.getSearchState())
            if (wParam == VK_DELETE) {
            //clear selected area
            if (obj.getselectionState()) {
                obj.deletePortion();
                obj.getselectionState() = false;
            }
            InvalidateRect(hwnd, NULL, FALSE);

            //no need of deletion
            if (obj.getLength() <= 1) {
                obj.getCursorIndex() = 0;
                obj.getLength() = 0;
                obj.getWithoutSp() = 0;
                obj.getText()[0] = '\0';
                obj.getTotalWords() = 0;
                obj.recalculateLayout();

            }
            else {
                obj.deleteForward();
            }

            InvalidateRect(hwnd, NULL, FALSE);
        }
      
        // Up arrow pressed
        else if (wParam == VK_UP) {
            if (obj.getPageIndex() > 0) {
                obj.getPageIndex()--;

                //move cursor to the start of page
                generateCursorInfo(obj.getstartX(), obj.getstartY(), obj.getCursorIndex(), obj.getcursorX(), obj.getcursorY());
            }
        }
        
        //down arrow key
        else if (wParam==VK_DOWN) {
            if (obj.getPageIndex() + 1 <= obj.getMaxPage()) {
                obj.getPageIndex()++;
                //move cursor
                generateCursorInfo(obj.getstartX(), obj.getstartY(), obj.getCursorIndex(), obj.getcursorX(), obj.getcursorY());
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
        //stop selection
        obj.getselectionState() = false;
        obj.getselectionStart() = -1;
        obj.getselectionEnd() = -1;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        wprintf(L"x: %d ,  y: %d\n", x, y);

        generateCursorInfo(x, y, obj.getCursorIndex(), obj.getcursorX(), obj.getcursorY());
        
       
        return 0;
    }
   //left click,start selection
    case WM_LBUTTONDOWN:{
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        wprintf(L"Selection Start at-> x: %d ,y:  %d!\n",x,y);

        //turn on selection
        obj.getselectionState() = true;

        //save coordinates of starting position and map coordinates->index 
        generateCursorInfo(x, y, obj.getselectionStart(), obj.getselectionStartX(), obj.getselectionStartY());

        //dragging
        SetCapture(hwnd);

        return 0;
    }
  
    //selection
    case WM_MOUSEMOVE: {
        //check if user is selecting
        if (obj.getselectionState()) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            //keep storing latest positions and alternate index mapping
            unsigned long long tempIndex;
            int tempX, tempY;
            generateCursorInfo(x, y, tempIndex, tempX, tempY);

            obj.getselectionEnd() = tempIndex;
            obj.getselectionEndX() = tempX;
            obj.getselectionEndY() = tempY;

            //new cursor position
            obj.getCursorIndex() = tempIndex;
            obj.getcursorX() = tempX;
            obj.getcursorY() = tempY;

            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }


    //release selection
    case WM_LBUTTONUP: {
        obj.getselectionState() = true;
        ReleaseCapture();
        return 0;
    }
   
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