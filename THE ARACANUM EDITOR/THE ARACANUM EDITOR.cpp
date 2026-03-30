#include <windows.h>
#include"THE ARACANUM EDITOR.h"
#include<stdio.h>
#include<ctime>


unsigned long long start_time = time(NULL);

Tabs tab; //array of editors


//HELPERS
//converting screen x y click to valid buffer index
void generateCursorInfo(int x, int y,unsigned long long& index, int& cursorX, int& cursorY) {

    //find line 
    int activeLine = 0, activePage = 0, activeColumn = 0;
    bool filled = false;
    for (int p = 0; p <=tab.getActiveEditor().getPageIndex(); p++) {
        for (int c = 0; c < tab.getActiveEditor().getTotalColumns(); c++) {
            for (int l = 0; l < tab.getActiveEditor().getTotalLines(); l++) {
                int screenY = tab.getActiveEditor().getPages()[p].getColumns()[c].getLine()[l].screenY;
                int screenX = tab.getActiveEditor().getPages()[p].getColumns()[c].getLine()[l].screenX;
              //  wprintf(L"screenY: %d\n", screenY);
                if (y>=screenY && y<=screenY+tab.getActiveEditor().getLineHeight() && !filled){
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
    for (int c = 0; c < tab.getActiveEditor().getTotalColumns(); c++) {
        int min = tab.getActiveEditor().getPages()[activePage].getColumns()[c].getLine()[activeLine].screenX;
        int max = min + tab.getActiveEditor().getLineLength() * tab.getActiveEditor().getcharWidth();
        if (x>=min && x<=max) {
            activeColumn = c;
            break;
        }
    }

    //approximate char index
    int screenX = tab.getActiveEditor().getPages()[activePage].getColumns()[activeColumn].getLine()[activeLine].screenX;
    int temp = screenX;
    int jump = 0;
    int len = tab.getActiveEditor().getPages()[activePage].getColumns()[activeColumn].getLine()[activeLine].len;

    while (jump < len && temp<x) {
        temp += tab.getActiveEditor().getcharWidth();
        jump++;
    }
    //we have line, column, page and offset. generate index wrt text buffer
    int start = tab.getActiveEditor().getPages()[tab.getActiveEditor().getPageIndex()].getColumns()[activeColumn].getLine()[activeLine].start;
    index = start + jump;
    cursorX = temp;
  //  wprintf(L"x: %d , y: %d , index: %d , cursorX: %d , cursorY= %d\n",x,y,index,cursorX,cursorY);
}


int convertToInt(wchar_t* read) {
    //array always contains digits
    int num = 0;
    while (*read) {
        num *= 10;
        num += *read - 48;
        read++;
    }

    return num;
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Set font
        HFONT font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Courier New");
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        //displaying
        tab.getActiveEditor().render(hdc, hwnd, start_time,tab.getCurrentTabIndex(),tab.getTotalTabs());


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
            bool val=tab.getActiveEditor().saveFile("dummy.txt");
            break;
        }
        //opening file
        else if (wParam == 15) {
            bool val = tab.getActiveEditor().loadFile("sample-text_for_editor.txt");
            tab.getActiveEditor().recalculateLayout();
            //display latest page
            tab.getActiveEditor().getPageIndex()=tab.getActiveEditor().getMaxPage();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        
        //ctrl+N pressed, add new tab
        else if (wParam == 14) {
            wprintf(L"shift+N called\n");
            tab.appendTabs();
            //go to that tab
            tab.switchTo(tab.getTotalTabs());
            tab.getActiveEditor().recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }

        //closing current tab, ctrl+Q
        else if (wParam == 17) {
            tab.closeTab();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        //ctrl+L  for line settings on top
        else if (wParam == 12) {
            tab.getActiveEditor().clearSearchandLayoutInfo();
            tab.getActiveEditor().getlayoutInfoLinesState() = true;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        //ctrl+W for line length settings on top
        else if (wParam == 23) {
            tab.getActiveEditor().clearSearchandLayoutInfo();
            tab.getActiveEditor().getlayoutInfoLineLengthState() = true;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        //ctrl+K for column settings on top
        else if (wParam == 11) {
            tab.getActiveEditor().clearSearchandLayoutInfo();
            tab.getActiveEditor().getlayoutInfoColumnsState() = true;
            InvalidateRect(hwnd, NULL, FALSE);
        }

        //if ctrl+F pressed
        else if (wParam == 6) {
            tab.getActiveEditor().clearSearchandLayoutInfo();
            tab.getActiveEditor().getSearchState() = true;
            InvalidateRect(hwnd, NULL, FALSE);
        }


        // ESC pressed
        else if (wParam == 27) {
            tab.getActiveEditor().clear_highlights();
            tab.getActiveEditor().clearSearchandLayoutInfo();
            InvalidateRect(hwnd, NULL, FALSE);
        }



        //implementing search text interception and backspace handling. 
        if (tab.getActiveEditor().getSearchState()) {
            // Printable characters
            if (wParam >= 32 && wParam != 127) {
                //don't take a word larger than 20 letters
                if (tab.getActiveEditor().getSearchTextIndex() >= 20)break;

                tab.getActiveEditor().getSearchText()[tab.getActiveEditor().getSearchTextIndex()] = wParam;
                tab.getActiveEditor().getSearchTextIndex()++;
                tab.getActiveEditor().getSearchText()[tab.getActiveEditor().getSearchTextIndex()] = L'\0';

            }
            //backspace
            else if (wParam == '\b') {
                tab.getActiveEditor().getSearchText()[tab.getActiveEditor().getSearchTextIndex()] = L'\0';

                if (tab.getActiveEditor().getSearchTextIndex() != 0)
                    tab.getActiveEditor().getSearchTextIndex()--;
            }
            //enter pressed execute search
            else if (wParam == '\r') {
                //if empty array
                if (tab.getActiveEditor().getSearchTextIndex() == 0) {
                    //clear the marked indices
                    tab.getActiveEditor().clear_highlights();
                    tab.getActiveEditor().getSearchState() = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                tab.getActiveEditor().executeSearch();
                InvalidateRect(hwnd, NULL, FALSE);

            }

        }


        //if layoutline setting active
        else if (tab.getActiveEditor().getlayoutInfoLinesState()) {
            //input only numbers.
            if (wParam >= 48 && wParam <59) {
                //don't take a number with digits>4
                if (tab.getActiveEditor().getlayoutInfoLinesIndex() > 4) {

                    int value = convertToInt(tab.getActiveEditor().getlayoutInfoLines());
                    tab.getActiveEditor().setTotalLines(value);

                    tab.getActiveEditor().getlayoutInfoLinesState() = false;
                    tab.getActiveEditor().getlayoutInfoLinesIndex() = 0;
                    tab.getActiveEditor().getlayoutInfoLines()[0] = L'\0';

                    tab.getActiveEditor().recalculateLayout();
                    InvalidateRect(hwnd, NULL, FALSE);

                    break;

                }

                tab.getActiveEditor().getlayoutInfoLines()[tab.getActiveEditor().getlayoutInfoLinesIndex()] = wParam;
                tab.getActiveEditor().getlayoutInfoLinesIndex()++;
                tab.getActiveEditor().getlayoutInfoLines()[tab.getActiveEditor().getlayoutInfoLinesIndex()] = L'\0';

            }
            //backspace
            else if (wParam == '\b') {
                tab.getActiveEditor().getlayoutInfoLines()[tab.getActiveEditor().getlayoutInfoLinesIndex()] = L'\0';

                if (tab.getActiveEditor().getlayoutInfoLinesIndex() != 0)
                    tab.getActiveEditor().getlayoutInfoLinesIndex()--;
            }
            //enter pressed pass values to verify new layout
            else if (wParam == '\r') {
                //if empty array
                if (tab.getActiveEditor().getlayoutInfoLinesIndex() == 0) {
                    //clear
                    tab.getActiveEditor().getlayoutInfoLinesState() = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                

                //else convert to int and pass to setLines
                int value = convertToInt(tab.getActiveEditor().getlayoutInfoLines());
                tab.getActiveEditor().setTotalLines(value);

                tab.getActiveEditor().getlayoutInfoLinesState() = false;
                tab.getActiveEditor().getlayoutInfoLinesIndex() = 0;
                tab.getActiveEditor().getlayoutInfoLines()[0] = L'\0';

                tab.getActiveEditor().recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);

                break;

            }
        }

        //if LineLength setting active
        else if (tab.getActiveEditor().getlayoutInfoLineLengthState()) {
            //input only numbers
            if (wParam >= 48 && wParam < 59) {

                if (tab.getActiveEditor().getlayoutInfoLineLengthIndex() > 4) {

                    int value = convertToInt(tab.getActiveEditor().getlayoutInfoLineLength());
                    tab.getActiveEditor().setLineLength(value);

                    tab.getActiveEditor().getlayoutInfoLineLengthState() = false;
                    tab.getActiveEditor().getlayoutInfoLineLengthIndex() = 0;
                    tab.getActiveEditor().getlayoutInfoLineLength()[0] = L'\0';

                    tab.getActiveEditor().recalculateLayout();
                    InvalidateRect(hwnd, NULL, FALSE);

                    break;
                }

                tab.getActiveEditor().getlayoutInfoLineLength()
                    [tab.getActiveEditor().getlayoutInfoLineLengthIndex()] = wParam;

                tab.getActiveEditor().getlayoutInfoLineLengthIndex()++;

                tab.getActiveEditor().getlayoutInfoLineLength()
                    [tab.getActiveEditor().getlayoutInfoLineLengthIndex()] = L'\0';
            }
            //backspace
            else if (wParam == '\b') {
                tab.getActiveEditor().getlayoutInfoLineLength()
                    [tab.getActiveEditor().getlayoutInfoLineLengthIndex()] = L'\0';

                if (tab.getActiveEditor().getlayoutInfoLineLengthIndex() != 0)
                    tab.getActiveEditor().getlayoutInfoLineLengthIndex()--;
            }
            //enter
            else if (wParam == '\r') {

                if (tab.getActiveEditor().getlayoutInfoLineLengthIndex() == 0) {
                    tab.getActiveEditor().getlayoutInfoLineLengthState() = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }

                int value = convertToInt(tab.getActiveEditor().getlayoutInfoLineLength());
                tab.getActiveEditor().setLineLength(value);

                tab.getActiveEditor().getlayoutInfoLineLengthState() = false;
                tab.getActiveEditor().getlayoutInfoLineLengthIndex() = 0;
                tab.getActiveEditor().getlayoutInfoLineLength()[0] = L'\0';

                tab.getActiveEditor().recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);

                break;
            }
        }

        //if Columns setting active
        else if (tab.getActiveEditor().getlayoutInfoColumnsState()) {
            //input only numbers
            if (wParam >= 48 && wParam < 59) {

                if (tab.getActiveEditor().getlayoutInfoColumnsIndex() > 4) {

                    int value = convertToInt(tab.getActiveEditor().getlayoutInfoColumns());
                    tab.getActiveEditor().setTotalColumns(value);

                    tab.getActiveEditor().getlayoutInfoColumnsState() = false;
                    tab.getActiveEditor().getlayoutInfoColumnsIndex() = 0;
                    tab.getActiveEditor().getlayoutInfoColumns()[0] = L'\0';

                    tab.getActiveEditor().recalculateLayout();
                    InvalidateRect(hwnd, NULL, FALSE);

                    break;
                }

                tab.getActiveEditor().getlayoutInfoColumns()
                    [tab.getActiveEditor().getlayoutInfoColumnsIndex()] = wParam;

                tab.getActiveEditor().getlayoutInfoColumnsIndex()++;

                tab.getActiveEditor().getlayoutInfoColumns()
                    [tab.getActiveEditor().getlayoutInfoColumnsIndex()] = L'\0';
            }
            //backspace
            else if (wParam == '\b') {
                tab.getActiveEditor().getlayoutInfoColumns()
                    [tab.getActiveEditor().getlayoutInfoColumnsIndex()] = L'\0';

                if (tab.getActiveEditor().getlayoutInfoColumnsIndex() != 0)
                    tab.getActiveEditor().getlayoutInfoColumnsIndex()--;
            }
            //enter
            else if (wParam == '\r') {

                if (tab.getActiveEditor().getlayoutInfoColumnsIndex() == 0) {
                    tab.getActiveEditor().getlayoutInfoColumnsState() = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }

                int value = convertToInt(tab.getActiveEditor().getlayoutInfoColumns());
                tab.getActiveEditor().setTotalColumns(value);

                tab.getActiveEditor().getlayoutInfoColumnsState() = false;
                tab.getActiveEditor().getlayoutInfoColumnsIndex() = 0;
                tab.getActiveEditor().getlayoutInfoColumns()[0] = L'\0';

                tab.getActiveEditor().recalculateLayout();
                InvalidateRect(hwnd, NULL, FALSE);

                break;
            }
        }


        //ONLY RUN THESE OPERATIONS IF NOT IN SETTINGS BOX 
        else {
            //1 is ascii for ctrl+A
            if (wParam == 1) {
                tab.getActiveEditor().selectAll();
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
                            if (tab.getActiveEditor().getselectionState()) {
                                //delete selected text
                                tab.getActiveEditor().deletePortion();
                                InvalidateRect(hwnd, NULL, FALSE);
                            }
                            //insert to buffer
                            for (int i = 0; i < len; i++) {
                                tab.getActiveEditor().insertAt(tab.getActiveEditor().getCursorIndex(), temp[i]);
                            }
                            tab.getActiveEditor().recalculateLayout();
                            tab.getActiveEditor().getPageIndex() = tab.getActiveEditor().findCurrentPage();
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
                //check selection state
                if (tab.getActiveEditor().getselectionState() == 0)break;



                if (OpenClipboard(hwnd))
                {
                    EmptyClipboard();

                    int len=0;

                    if (tab.getActiveEditor().getselectionStart() <= tab.getActiveEditor().getselectionEnd()) {
                        len = tab.getActiveEditor().getselectionEnd() - tab.getActiveEditor().getselectionStart() + 1;
                    }
                    else {
                        len = tab.getActiveEditor().getselectionStart() - tab.getActiveEditor().getselectionEnd() + 1;
                    }

                    // optional safety check
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
                        unsigned long long start=0, end=0;
                        if (tab.getActiveEditor().getselectionStart() <= tab.getActiveEditor().getselectionEnd()) {
                            start = tab.getActiveEditor().getselectionStart();
                            end = tab.getActiveEditor().getselectionEnd();
                        }
                        else {
                            start = tab.getActiveEditor().getselectionEnd();
                            end = tab.getActiveEditor().getselectionStart();
                        }
                        for (unsigned long long i = start; i <= end; i++) {
                            pMem[index] = tab.getActiveEditor().getText()[i];
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
                if (tab.getActiveEditor().getLength() <= 1) {
                    tab.getActiveEditor().getCursorIndex() = 0;
                    tab.getActiveEditor().getLength() = 0;
                    tab.getActiveEditor().getWithoutSp() = 0;
                    tab.getActiveEditor().getText()[0] = '\0';
                    tab.getActiveEditor().getTotalWords() = 0;
                    tab.getActiveEditor().recalculateLayout();

                }
                else {
                    //deletion required 

                     //clear selected area
                    if (tab.getActiveEditor().getselectionState()) {
                        tab.getActiveEditor().deletePortion();
                        tab.getActiveEditor().getselectionState() = false;
                    }
                    InvalidateRect(hwnd, NULL, FALSE);

                    tab.getActiveEditor().deleteBack(tab.getActiveEditor().getCursorIndex());
                    tab.getActiveEditor().recalculateLayout();
                    tab.getActiveEditor().getPageIndex() = tab.getActiveEditor().findCurrentPage();
                }

                InvalidateRect(hwnd, NULL, FALSE);
            }
            // Enter key
            else if (wParam == '\r') {
                //clear selected area
                if (tab.getActiveEditor().getselectionState()) {
                    tab.getActiveEditor().deletePortion();
                    tab.getActiveEditor().getselectionState() = false;
                }
                InvalidateRect(hwnd, NULL, FALSE);

                tab.getActiveEditor().insertAt(tab.getActiveEditor().getCursorIndex(), '\n');
                tab.getActiveEditor().recalculateLayout();
                tab.getActiveEditor().getPageIndex() = tab.getActiveEditor().findCurrentPage();

                InvalidateRect(hwnd, NULL, FALSE);

            }
            // Printable characters
            else if (wParam >= 32 && wParam != 127) {
                //clear selected area
                if (tab.getActiveEditor().getselectionState()) {
                    tab.getActiveEditor().deletePortion();
                    tab.getActiveEditor().getselectionState() = false;
                }
                InvalidateRect(hwnd, NULL, FALSE);

                tab.getActiveEditor().insertAt(tab.getActiveEditor().getCursorIndex(), (wchar_t)wParam);

                tab.getActiveEditor().recalculateLayout();
                tab.getActiveEditor().getPageIndex() = tab.getActiveEditor().findCurrentPage();

                InvalidateRect(hwnd, NULL, FALSE);

            }
        }
        
        
        return 0;
    
}


    case WM_KEYDOWN: {

        //ctrl+shift+H for history
        if (wParam == 'H' && (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000)) {
            //turn off if already on
            if (tab.getActiveEditor().getHistoryState() == true) {
                tab.getActiveEditor().getHistoryState() = false;
            }
            else {
                tab.getActiveEditor().getHistoryState() = true;
            }

            InvalidateRect(hwnd, NULL, FALSE);
        }


        //moving in between tabs

        //shift tab move backwards
        if (wParam == VK_TAB && (GetKeyState(VK_CONTROL) & 0x8000) & (GetKeyState(VK_SHIFT) & 0x8000)) {
            tab.decrementTab();
            //display length
            wprintf(L"Length: %d\n", tab.getActiveEditor().getLength());
            tab.getActiveEditor().recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }


        //tab pressed move forward
        else if ((wParam == VK_TAB) && (GetKeyState(VK_CONTROL) & 0x8000)){
            tab.incrementTab();
            tab.getActiveEditor().recalculateLayout();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        


        if(!tab.getActiveEditor().getSearchState())
            if (wParam == VK_DELETE) {
            //clear selected area
            if (tab.getActiveEditor().getselectionState()) {
                tab.getActiveEditor().deletePortion();
                tab.getActiveEditor().getselectionState() = false;
            }
            InvalidateRect(hwnd, NULL, FALSE);

            //no need of deletion
            if (tab.getActiveEditor().getLength() <= 1) {
                tab.getActiveEditor().getCursorIndex() = 0;
                tab.getActiveEditor().getLength() = 0;
                tab.getActiveEditor().getWithoutSp() = 0;
                tab.getActiveEditor().getText()[0] = '\0';
                tab.getActiveEditor().getTotalWords() = 0;
                tab.getActiveEditor().recalculateLayout();

            }
            else {
                tab.getActiveEditor().deleteForward();
            }

            InvalidateRect(hwnd, NULL, FALSE);
        }
      
        // Up arrow pressed
        else if (wParam == VK_UP) {
            if (tab.getActiveEditor().getPageIndex() > 0) {
                tab.getActiveEditor().getPageIndex()--;

                //move cursor to the start of page
                generateCursorInfo(tab.getActiveEditor().getstartX(), tab.getActiveEditor().getstartY(), tab.getActiveEditor().getCursorIndex(), tab.getActiveEditor().getcursorX(), tab.getActiveEditor().getcursorY());
            }
        }
        
        //down arrow key
        else if (wParam==VK_DOWN) {
            if (tab.getActiveEditor().getPageIndex() + 1 <= tab.getActiveEditor().getMaxPage()) {
                tab.getActiveEditor().getPageIndex()++;
                //move cursor
                generateCursorInfo(tab.getActiveEditor().getstartX(), tab.getActiveEditor().getstartY(), tab.getActiveEditor().getCursorIndex(), tab.getActiveEditor().getcursorX(), tab.getActiveEditor().getcursorY());
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
        tab.getActiveEditor().getselectionState() = false;
        tab.getActiveEditor().getselectionStart() = -1;
        tab.getActiveEditor().getselectionEnd() = -1;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
     //   wprintf(L"x: %d ,  y: %d\n", x, y);

        generateCursorInfo(x, y, tab.getActiveEditor().getCursorIndex(), tab.getActiveEditor().getcursorX(), tab.getActiveEditor().getcursorY());
        
       
        return 0;
    }
   //left click,start selection
    case WM_LBUTTONDOWN:{
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
       // wprintf(L"Selection Start at-> x: %d ,y:  %d!\n",x,y);

        //turn on selection
        tab.getActiveEditor().getselectionState() = true;

        //save coordinates of starting position and map coordinates->index 
        generateCursorInfo(x, y, tab.getActiveEditor().getselectionStart(), tab.getActiveEditor().getselectionStartX(), tab.getActiveEditor().getselectionStartY());

        //dragging
        SetCapture(hwnd);

        return 0;
    }
  
    //selection
    case WM_MOUSEMOVE: {
        //check if user is selecting
        if (tab.getActiveEditor().getselectionState()) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            //keep storing latest positions and alternate index mapping
            unsigned long long tempIndex;
            int tempX, tempY;
            generateCursorInfo(x, y, tempIndex, tempX, tempY);

            tab.getActiveEditor().getselectionEnd() = tempIndex;
            tab.getActiveEditor().getselectionEndX() = tempX;
            tab.getActiveEditor().getselectionEndY() = tempY;

            //new cursor position
            tab.getActiveEditor().getCursorIndex() = tempIndex;
            tab.getActiveEditor().getcursorX() = tempX;
            tab.getActiveEditor().getcursorY() = tempY;

            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }


    //release selection
    case WM_LBUTTONUP: {
        tab.getActiveEditor().getselectionState() = true;
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