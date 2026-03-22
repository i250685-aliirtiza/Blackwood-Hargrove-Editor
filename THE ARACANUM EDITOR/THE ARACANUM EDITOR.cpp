#include <windows.h>
#include"THE ARACANUM EDITOR.h"
#include<stdio.h>
#include<ctime>

unsigned long long start_time = time(NULL);



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
              //  wprintf(L"screenY: %d\n", screenY);
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
        // Set font
        HFONT font = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Courier New");
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));

        //displaying
        obj.render(hdc, hwnd,start_time);


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

        //ctrl+H for history
        if (wParam == 8) {
            //if already shown then close
            if (obj.getHistoryState()==true) {
                obj.getHistoryState() = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            else
            obj.getHistoryState()=true;
        }
        // ESC pressed, cancel search, clear the text in it
        if (wParam == 27) {
            //clear the marked indices
            obj.clear_highlights();
            obj.getSearchState() = false;
            obj.getSearchTextIndex() = 0;
            obj.getSearchText()[obj.getSearchTextIndex()] = L'\0';
        }
       
        //implementing search text interception and backspace handling. 
        if (obj.getSearchState()) {
            // Printable characters
            if (wParam >= 32 && wParam != 127) {
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
                //if empty array
                if (obj.getSearchTextIndex() == 0) {
                    //clear the marked indices
                    obj.clear_highlights();
                    obj.getSearchState() = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
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
                         //   wprintf(L"%c", pMem[index]);
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
     //   wprintf(L"x: %d ,  y: %d\n", x, y);

        generateCursorInfo(x, y, obj.getCursorIndex(), obj.getcursorX(), obj.getcursorY());
        
       
        return 0;
    }
   //left click,start selection
    case WM_LBUTTONDOWN:{
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
       // wprintf(L"Selection Start at-> x: %d ,y:  %d!\n",x,y);

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