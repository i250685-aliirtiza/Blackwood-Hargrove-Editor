#pragma once
#include "resource.h"
#include<windows.h>
#include <cstdio>
#include<fstream>
#include<ctime>

using namespace std;

//layout
struct Line {
    unsigned long long start = -1;
    int startY=40;
    int startX=40;
    int len = 0;
    int screenY = startY; //storing y-coordinate of each line
    int screenX = startX;//diff for each char
    int offsetX = 1;//character width
};

class column {
private:
    Line* lines;
    int total_lines;
public:
    //getter
    Line* getLine() const {
        return lines;
    }
    int getTotalLines() const {
        return total_lines;
    }
    //setting total lines (user input)
    column(int n=20): total_lines(n) {
        lines = new Line[total_lines];
    }

    // Copy constructor
    column(const column& other): total_lines(other.total_lines){
        lines = new Line[total_lines];
        for (int i = 0; i < total_lines; i++) {
            lines[i] = other.lines[i];
        }

    }

    column& operator=(const column& other) {
        if (this == &other)return *this;
        delete[] this->lines;
        this->total_lines = other.total_lines;
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

class page {
private:
    column* columns;
    int total_columns;
    int total_lines;
public:
    //getter
    column* getColumns() const {
        return columns;
    }
    int getTotalColumns()const {
        return total_columns;
    }

    page(int c = 3, int l = 10) : total_columns(c) ,total_lines(l) {
        columns = new column[total_columns];
        //initialize with user given values
        for (int i = 0; i < total_columns; i++) {
            columns[i] = column(total_lines);
        }

    }

    // Copy constructor
    page(const page& other): total_columns(other.total_columns), total_lines(other.total_lines) {
        columns = new column[total_columns];
        for (int i = 0; i < total_columns; i++)
            columns[i] = other.columns[i];
    }
    //assignment operator
    page& operator=(const page& other) {
        if (this == &other)return *this;

        //delete old 
        delete[] this->columns;

        this->total_columns = other.total_columns;
        this->total_lines = other.total_lines;
        columns = new column[total_columns];
        for (int i = 0; i < total_columns; i++)
            this->columns[i] = other.columns[i];

        return *this;
    }

    ~page() {
        delete[] columns;
    }
};

//search and highlight index
struct Highlight {
    unsigned long long start=-1;
    int len=0;
};

//editor: consists of multiple pages, operations, text buffer, and methods
class Editor {

private:
    unsigned long long capacity;
    wchar_t* text;
    unsigned long long length;
    int cursor_width = 1;
    //pages
    page* pages;
    int page_index;

    //highlighting text
    Highlight* marked_text;
    int marked_text_cap;
    int marked_text_index;


    int line_length, total_lines, total_columns, total_pages;
    int startX, startY;
    int char_width, lineHeight, lineWidth;
    int cursorX, cursorY;
    unsigned long long cursor_to_text_index;

     
    int maxCursorX = startX, maxCursorY = startY;

    int words, withoutSpaces;

    int minLineLength;
    int minTotalLines ;
    int minTotalColumns;

    bool selectionState;
    unsigned long long start_selection;
    unsigned long long end_selection;
    int start_selectionX;
    int start_selectionY;
    int end_selectionX;
    int end_selectionY;
    int max_page;

    bool searchState;
    wchar_t searchText[300];
    int searchIndex;

    int minStartX = 40;
    int minStartY = 40;

    int sentences;

    bool showHistory;

    wchar_t searchHistory[5][100];
    int searchHistoryIndex = 0;
    int searchHistoryCapacity = 5;

    int searchHistoryCounts[5] = { 0 };


    //run time controls 
    wchar_t layoutInfoLines[100] = {'\0'};
    wchar_t layoutInfoLineLength[100] = { '\0' };
    wchar_t layoutInfoColumns[100] = { '\0' };

    int layoutInfoLinesIndex=0;
    int layoutInfoLineLengthIndex=0;
    int layoutInfoColumnsIndex=0;

    bool layoutInfoLinesState = false;
    bool layoutInfoLineLengthState = false;
    bool layoutInfoColumnsState = false;
   


    //max line_length
    const int MAX_CAPACITY=90;

public:
    //constructor
    Editor() {

         layoutInfoLinesIndex = 0;
         layoutInfoLineLengthIndex = 0;
         layoutInfoColumnsIndex = 0;


        layoutInfoLinesState = false;
         layoutInfoLineLengthState = false;
        layoutInfoColumnsState = false;
        layoutInfoLines[0] = '\0' ;
        layoutInfoLineLength[0] =  '\0' ;
        layoutInfoColumns[0] = '\0' ;
        sentences = 0;
        showHistory = false;
        searchState = false;
        searchIndex = 0;
         marked_text_cap = 10;
         marked_text_index = 0;
         marked_text = new Highlight[marked_text_cap];

         searchText[0] = L'\0';
        //initialize search history to \0
         for (int i = 0; i < searchHistoryCapacity; i++) {
             searchHistory[i][0] = '\0';
        }


        minLineLength = 5;
        minTotalLines = 1;
        minTotalColumns = 2;

        capacity = 10000;
        text = new wchar_t[capacity];
        length = 0;
        //default
        line_length = 40;
        total_lines = 20;
        total_columns = 2;
        total_pages = 2; page_index = 0;
        startX = 40;
        startY = 40;
        char_width = 13;
        cursorX = startX; cursorY = startY;
        words = 0; withoutSpaces = 0;
        cursor_to_text_index = length;
        max_page = 0;
        selectionState=false;
        start_selection=-1;
        end_selection=-1;
        start_selectionX=-1;
        start_selectionY=-1;
        end_selectionX=-1;
        end_selectionY=-1;

        //allocate pages
        pages = new page[total_pages];
        for (int i = 0; i < total_pages; i++) {
            pages[i] = page(total_columns, total_lines);
        }
    }

    ~Editor() {
        delete[] text;
        delete[] pages;
        delete[] marked_text;
    }

    //copy constructor
    Editor(const Editor& other) {

        this->capacity = other.capacity;
        this->length = other.length;
        this->cursor_width = other.cursor_width;
        this->page_index = other.page_index;

        this->line_length = other.line_length;
        this->total_lines = other.total_lines;
        this->total_columns = other.total_columns;
        this->total_pages = other.total_pages;
        this->startX = other.startX;
        this->startY = other.startY;
        this->char_width = other.char_width;
        this->lineHeight = other.lineHeight;
        this->lineWidth = other.lineWidth;
        this->cursorX = other.cursorX;
        this->cursorY = other.cursorY;
        this->cursor_to_text_index = other.cursor_to_text_index;
        this->maxCursorX = other.maxCursorX;
        this->maxCursorY = other.maxCursorY;
        this->words = other.words;
        this->withoutSpaces = other.withoutSpaces;
        this->minLineLength = other.minLineLength;
        this->minTotalLines = other.minTotalLines;
        this->minTotalColumns = other.minTotalColumns;
        this->selectionState = other.selectionState;
        this->start_selection = other.start_selection;
        this->end_selection = other.end_selection;
        this->start_selectionX = other.start_selectionX;
        this->start_selectionY = other.start_selectionY;
        this->end_selectionX = other.end_selectionX;
        this->end_selectionY = other.end_selectionY;
        this->max_page = other.max_page;
        this->searchState = other.searchState;
        this->searchIndex = other.searchIndex;
        this->sentences = other.sentences;
        this->showHistory = other.showHistory;
        this->searchHistoryIndex = other.searchHistoryIndex;
        this->searchHistoryCapacity = other.searchHistoryCapacity;

        //char arrays
        const wchar_t* read = other.searchText;
        wchar_t* write = this->searchText;
        while (*read) {
            *write = *read;
            read++;
            write++;
            
        }
        *write = L'\0';

        for (int i = 0; i < 5; i++) {
            this->searchHistoryCounts[i] = other.searchHistoryCounts[i];
            for (int j = 0; j < 100; j++) {
                if (other.searchHistory[i][j] == '\0') {
                    this->searchHistory[i][j] = '\0';
                    break;
                }
                else this->searchHistory[i][j] = other.searchHistory[i][j];
            }
        }

        //copy text array
        this->text = new wchar_t[this->capacity];
        for (unsigned long long i = 0; i < this->length; i++)
            this->text[i] = other.text[i];

        //copy pages
        this->pages = new page[other.total_pages];
        for (int i = 0; i < other.total_pages; i++) {
            this->pages[i] = other.pages[i];
        }
        
  

        this->marked_text_cap = other.marked_text_cap;
        this->marked_text_index = other.marked_text_index;
        this->marked_text = new Highlight[this->marked_text_cap];
        for (int i = 0; i < this->marked_text_index; i++) {
            this->marked_text[i] = other.marked_text[i];
        }
        

    }

    //assignment operator
    Editor& operator=(const Editor& other) {
        //self assignment
        if (this == &other)return *this;

        //delete old
        delete[] text;
        delete[] pages;
        delete[] marked_text;

        this->capacity = other.capacity;
        this->length = other.length;
        this->cursor_width = other.cursor_width;
        this->page_index = other.page_index;

        this->line_length = other.line_length;
        this->total_lines = other.total_lines;
        this->total_columns = other.total_columns;
        this->total_pages = other.total_pages;
        this->startX = other.startX;
        this->startY = other.startY;
        this->char_width = other.char_width;
        this->lineHeight = other.lineHeight;
        this->lineWidth = other.lineWidth;
        this->cursorX = other.cursorX;
        this->cursorY = other.cursorY;
        this->cursor_to_text_index = other.cursor_to_text_index;
        this->maxCursorX = other.maxCursorX;
        this->maxCursorY = other.maxCursorY;
        this->words = other.words;
        this->withoutSpaces = other.withoutSpaces;
        this->minLineLength = other.minLineLength;
        this->minTotalLines = other.minTotalLines;
        this->minTotalColumns = other.minTotalColumns;
        this->selectionState = other.selectionState;
        this->start_selection = other.start_selection;
        this->end_selection = other.end_selection;
        this->start_selectionX = other.start_selectionX;
        this->start_selectionY = other.start_selectionY;
        this->end_selectionX = other.end_selectionX;
        this->end_selectionY = other.end_selectionY;
        this->max_page = other.max_page;
        this->searchState = other.searchState;
        this->searchIndex = other.searchIndex;
        this->sentences = other.sentences;
        this->showHistory = other.showHistory;
        this->searchHistoryIndex = other.searchHistoryIndex;
        this->searchHistoryCapacity = other.searchHistoryCapacity;

        //char arrays
        const wchar_t* read = other.searchText;
        wchar_t* write = this->searchText;
        while (*read) {
            *write = *read;
            read++;
            write++;

        }
        *write = L'\0';

        for (int i = 0; i < 5; i++) {
            this->searchHistoryCounts[i] = other.searchHistoryCounts[i];
            for (int j = 0; j < 100; j++) {
                if (other.searchHistory[i][j] == '\0') {
                    this->searchHistory[i][j] = '\0';
                    break;
                }
                else this->searchHistory[i][j] = other.searchHistory[i][j];
            }
        }

        //copy text array
        this->text = new wchar_t[this->capacity];
        for (unsigned long long i = 0; i < this->length; i++)
            this->text[i] = other.text[i];

        //copy pages
        this->pages = new page[other.total_pages];
        for (int i = 0; i < other.total_pages; i++) {
            this->pages[i] = other.pages[i];
        }



        this->marked_text_cap = other.marked_text_cap;
        this->marked_text_index = other.marked_text_index;
        this->marked_text = new Highlight[this->marked_text_cap];
        for (int i = 0; i < this->marked_text_index; i++) {
            this->marked_text[i] = other.marked_text[i];
        }


        return *this;
    }



    //METHODS
    //getter
    int getTotalLines() const{
        return total_lines;
    }
    int getTotalColumns() const {
        return total_columns;
    }
    int getTotalPages() const {
        return total_pages;
    }
    int& getPageIndex()  {
        return page_index;
    }
    int& getMaxPage() {
        return max_page;
    }
    int& getTotalWords() {
        return words;
    }
    int getLineLength()const {
        return line_length;
    }
    int& getWithoutSp()  {
        return withoutSpaces;
    }
    wchar_t* getText() {
        return text;
    }
    page* getPages() {
        return pages;
    }
    int& getLineHeight() {
        return lineHeight;
    }
    int& getLineWidth() {
        return lineWidth;
    }
    unsigned long long& getLength()  {
        return length;
    }

    unsigned long long& getCursorIndex() {
        return cursor_to_text_index;
    }

    int& getstartX() {
        return startX;
    }
    int& getstartY() {
        return startY;
    }
    int& getcursorX() {
        return cursorX;
    }
    int& getcursorY() {
        return cursorY;
    }
    int& getcharWidth() {
        return char_width;
    }
    int& getmaxCursorX() {
        return maxCursorX;
    }
    int& getmaxCursorY() {
        return maxCursorY;
    }
    bool& getselectionState() {
        return selectionState;
  
    }
    unsigned long long& getselectionStart() {
        return start_selection;

    }
    unsigned long long& getselectionEnd() {
        return end_selection;

    }
    int& getselectionStartX() {
        return start_selectionX;

    }
    int& getselectionStartY() {
        return start_selectionY;

    }
    int& getselectionEndX() {
        return end_selectionX;

    }
    int& getselectionEndY() {
        return end_selectionY;
    }

    bool& getSearchState() {
        return searchState;
    }
    wchar_t* getSearchText() {
        return searchText;
    }
    int& getSearchTextIndex() {
        return searchIndex;
    }

    bool& getHistoryState() {
        return showHistory;
    }
    int& getHistoryCap() {
        return searchHistoryCapacity;
    }

    //get total marked highlight
    int& getMarkedIndex() {
        return marked_text_index;
    }
    Highlight* getMarkedText() {
        return marked_text;
    }

    int& getSentences() {
        return sentences;
    }


    bool& getlayoutInfoLinesState() {
        return layoutInfoLinesState;
    } 
    bool& getlayoutInfoLineLengthState() {
        return layoutInfoLineLengthState;
    }
    bool& getlayoutInfoColumnsState() {
        return layoutInfoColumnsState;
    }

    int& getlayoutInfoLinesIndex() {
        return layoutInfoLinesIndex;
    }
    int& getlayoutInfoLineLengthIndex() {
        return layoutInfoLineLengthIndex;
    }
    int& getlayoutInfoColumnsIndex() {
        return layoutInfoColumnsIndex;
    }


    wchar_t* getlayoutInfoLines() {
        return layoutInfoLines;
    }
    wchar_t* getlayoutInfoLineLength() {
        return layoutInfoLineLength;
    }
    wchar_t* getlayoutInfoColumns() {
        return layoutInfoColumns;
    }


    void clearSearchandLayoutInfo() {
        //clear the marked indices
        getSearchState() = false;
        getSearchTextIndex() = 0;
        getSearchText()[0] = L'\0';

        //clear layout info states
        getlayoutInfoLinesState() = false;
        getlayoutInfoLineLengthState() = false;
        getlayoutInfoColumnsState() = false;

        //reset indices
        getlayoutInfoLinesIndex() = 0;
        getlayoutInfoLineLengthIndex() = 0;
        getlayoutInfoColumnsIndex() = 0;

        //clear layout info text arrays
        getlayoutInfoLines()[0] = L'\0';
        getlayoutInfoLineLength()[0] = L'\0';
        getlayoutInfoColumns()[0] = L'\0';
    }


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


    void render(HDC& hdc, HWND& hwnd,unsigned long long start_time,int current_tab,int total_tabs) {

        // Clear background
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0,0));//gray
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);



        // Rendering function

        // Get line height
        TEXTMETRICW tm;
        GetTextMetrics(hdc, &tm);
        getLineHeight() = tm.tmHeight + tm.tmExternalLeading;
        SIZE size;
        GetTextExtentPoint32W(hdc, getText(), getLength(), &size);
        getLineWidth() = size.cx; // width in pixels of len characters
        // Starting positions
        float columnSpacing = 14; // horizontal gap between columns

        bool filled = false;
        unsigned long long track = 0;
        float x = getstartX(), y = getstartY();



        //SHOW PAGE
        int p = getPageIndex();
        int x_chars = 0;
        // Loop over columns 
        for (int c = 0; c < getTotalColumns(); c++) {
            if (filled) break;
            // Loop over lines
            for (int l = 0; l < getTotalLines(); l++) {


                unsigned long long start = getPages()[p].getColumns()[c].getLine()[l].start;
                int len = getPages()[p].getColumns()[c].getLine()[l].len;
                x_chars = len == 0 ? x_chars : len;
                if (start == -1 || len == 0)continue;


                // Calculate position
                x = getstartX() + c * (getLineLength() * columnSpacing);
                y = getstartY() + l * getLineHeight();

                //printing the line char by char if it is marked
                bool isMarked = false;
                //search for index of current line in highlights
                for (int i = 0; i < getMarkedIndex(); i++) {
                    unsigned long long tempStart = getMarkedText()[i].start;
                    int tempLen = getMarkedText()[i].len;
                    if (tempStart >= start && tempLen < start + len) {
                        isMarked = true;
                        break;
                    }
                }
                if (isMarked) {
                    unsigned long long lineStart = start;
                    unsigned long long lineEnd = start + len;
                    int lineStartX = x;

                    // Loop through each character on this line
                    for (unsigned long long char_index = lineStart; char_index < lineEnd; char_index++) {
                        wchar_t ch = getText()[char_index];

                        // if character is marked in our highlights, print red
                        bool isRed = false;
                        for (int i = 0; i < getMarkedIndex(); i++) {
                            unsigned long long markStart = getMarkedText()[i].start;
                            unsigned long long markLen = (unsigned long long)getMarkedText()[i].len;
                            unsigned long long markEnd = markStart + markLen;

                            if (char_index >= markStart && char_index <= markEnd) {
                                isRed = true;
                                break;
                            }
                        }

                        // Set color and draw
                        if (isRed) {
                            SetTextColor(hdc, RGB(255, 0, 0));  // Red
                        }
                        else {
                            SetTextColor(hdc, RGB(255,255,255));   //white
                        }

                        TextOutW(hdc, x, y, &ch, 1);

                        // Advance x position
                        SIZE charSize;
                        GetTextExtentPoint32W(hdc, &ch, 1, &charSize);
                        x += charSize.cx;
                    }

                    getLineWidth() = x - lineStartX;
                }

                //if not marked
                else {
                    //check if start lies in our selected range
                    if (getselectionStart() >= start && getselectionEnd() <= start + len) {
                        SetTextColor(hdc, RGB(0, 0, 255)); // blue for selected                   
                    }
                    else {
                        SetTextColor(hdc, RGB(255, 255, 255)); // white for normal
                    }

                    TextOutW(hdc, x, y, getText() + start, len);
                }
                size;
                GetTextExtentPoint32W(hdc, getText() + start, len, &size);
                getLineWidth() = size.cx; // width in pixels of len characters
                getcharWidth() = getLineWidth() / len;

                //useful for cursor coordinates
                getPages()[p].getColumns()[c].getLine()[l].screenY = y;
                getPages()[p].getColumns()[c].getLine()[l].screenX = x;

                // update track
                track += len;
                if (track >= getLength()) {
                    filled = true;
                    break;
                }
            }
        }
     
        
        
        //showing search bar at top
        if (searchState) {
            int searchX = getstartX();
            int searchY = 0;

            wchar_t searchTEXT[500];
            int searchLen = merge(L"search(CTRL+F, ESC, ENTER): ", getSearchText(), searchTEXT);
            SetTextColor(hdc, RGB(255, 0, 0)); //red
            TextOutW(hdc, searchX, searchY, searchTEXT, searchLen);

        }
      
        
        //LINE SETTINGS
        //showing layout info lines at top
        else if (layoutInfoLinesState) {
            int layoutX = getstartX();
            int layoutY = 0;
            wchar_t layoutLineTEXT[500];
            int layoutLen = merge(L"lines(CTRL+L, ESC, ENTER): ", getlayoutInfoLines(), layoutLineTEXT);
            SetTextColor(hdc, RGB(0, 255, 0)); //green
            TextOutW(hdc, layoutX, layoutY, layoutLineTEXT, layoutLen);
        }
        //showing layout info line length at top
        else if (layoutInfoLineLengthState) {
            int layoutX = getstartX();
            int layoutY = 0;
            wchar_t layoutLineTEXT[500];

            int layoutLen = merge(L"line length(CTRL+K, ESC, ENTER): ", getlayoutInfoLineLength(),layoutLineTEXT);

            SetTextColor(hdc, RGB(0, 255, 0)); //green
            TextOutW(hdc, layoutX, layoutY, layoutLineTEXT, layoutLen);
        }
        //showing layout info columns at top
        else if (layoutInfoColumnsState) {
            int layoutX = getstartX();
            int layoutY = 0;
            wchar_t layoutLineTEXT[500];

            int layoutLen = merge(L"columns(CTRL+C, ESC, ENTER): ", getlayoutInfoColumns(),layoutLineTEXT);

            SetTextColor(hdc, RGB(0, 255, 0)); //green
            TextOutW(hdc, layoutX, layoutY, layoutLineTEXT, layoutLen);
        }

        SetTextColor(hdc, RGB(128, 128, 128)); // black for normal




        //place at the end
        getmaxCursorX() = x + ((x_chars)*getcharWidth());
        getmaxCursorY() = y;

        //rendering cursor
        unsigned long long current_time = time(NULL);
        unsigned long long elapsed_time = current_time - start_time;

        //showing periodically
        if (elapsed_time % 2 == 0) {
            RECT cursorRect;

            cursorRect = { getcursorX(), getcursorY(),getcursorX() + cursor_width, getcursorY() + getLineHeight() };
            //cursor color
            brush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &cursorRect, brush);
            DeleteObject(brush);
        }





        //SHOWING FOOTER

        int footerY = getLineHeight() * getTotalLines();
        footerY += getstartY();
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
        convertToStr(getTotalWords(), arr);
        len += merge(str2, arr, info1);

        str2 = L" | Total Characters: ";
        convertToStr(getLength(), arr);
        len += merge(str2, arr, info2);

        str2 = L"  | Total Characters without sapaces: ";
        convertToStr(getWithoutSp(), arr);
        len += merge(str2, arr, info3);


        concatenate(info1, info2, info3, final);

        //add sentences count
        str2 = L" | Total Sentences: ";
        convertToStr(getSentences(), arr);
        wchar_t final2[400];

        wchar_t info4[100];
        len += merge(str2, arr, info4);
        len = 0;
        len += merge(final, info4, final2);

        SetTextColor(hdc, RGB(255, 255, 255)); // black for normal

        TextOutW(hdc, getstartX(), footerY, final2, len);

        //also page numbering, showing under middle column.
        int val = getTotalColumns() * getLineLength();
        int temp = getstartX();
        temp = val / 2;
        temp *= getcharWidth();

        int page_numberX = temp;
        footerY -= 20;
        len = 0;
        str2 = L"------";
        convertToStr(getPageIndex() + 1, arr);
        concatenate(str2, arr, str2, final);
        len = getLen(final) - 1;
        TextOutW(hdc, page_numberX, footerY, final, len);

        //show current tab in same line
        str2 = L"DOCUMENT: (";
        convertToStr(current_tab+1, arr);
        len = 0;
        len += merge(str2, arr, info1);
        str2 = L" / ";
        convertToStr(total_tabs, arr);
        len = 0;
        len += merge(str2, arr, info2);
        len += merge(info1, info2, info3);
        str2 = L")";
        len = 0;
        len += merge(info3, str2, info4);
        SetTextColor(hdc, RGB(0, 200, 200));
        int document_x = page_numberX <= 200 ? page_numberX + 200 : page_numberX - 200;
        TextOutW(hdc, document_x, footerY, info4, len);

        //show search history
        if (getHistoryState()) {

            footerY += 30;
            SetTextColor(hdc, RGB(255, 0, 0));
            TextOutW(hdc, getstartX(), footerY, L"====SEARCH HISTORY==== ", 24);
            footerY += 10;
            for (int i = 0; i < getHistoryCap(); i++) {
                const wchar_t* str2 = getHistoryAt(i);
                const wchar_t* str3 = L"  :  ";
                int len = merge(str2, str3, final);
                //skipping empty entries
                if (getHistoryCountAt(i) == 0)continue;

                convertToStr(getHistoryCountAt(i), arr);
                len = merge(final, arr, final2);

                //final string
                SetTextColor(hdc, RGB(0,255,0));
                TextOutW(hdc, getstartX(), footerY, final2, len);

                footerY += 10;
            }
        }


        // Restore original font
        SelectObject(hdc, oldFooterFont);

        // Delete footer font
        DeleteObject(footerFont);

   }

    

    const wchar_t* getHistoryAt(int i) {
        if (i < 0)i = 0;
        if (i >= searchHistoryCapacity)i = searchHistoryCapacity - 1;
        return searchHistory[i];
    }
    int getHistoryCountAt(int i) {
        if (i < 0)i = 0;
        if (i >= searchHistoryCapacity)i = searchHistoryCapacity - 1;

        return searchHistoryCounts[i];
    }



    void pushback_highlight(unsigned long long start, int len) {
          //resize if required
        if (marked_text_index >= marked_text_cap) {
            Highlight* copy = new Highlight[marked_text_cap * 2];

            for (int i = 0; i < marked_text_index;i++) {
                copy[i] = marked_text[i];
            }
            delete[] marked_text;
            marked_text = copy;
            marked_text_cap *= 2;
        }
        marked_text[marked_text_index].start = start;
        marked_text[marked_text_index].len = len;
         
        marked_text_index++;
    }

    void clear_highlights() {
        if (marked_text_index == 0)return;

        delete[] marked_text;
        marked_text_cap = 5;
        marked_text = new Highlight[marked_text_cap];
        marked_text_index = 0;
    }

    //reading/writing files
    //save file
    bool saveFile(const char* filename=nullptr) {
        //if user hasn't passed name
        if (filename == nullptr) {
            filename = "file.txt";
        }

        wofstream writeFile;
        writeFile.open(filename);
        if (!writeFile)return false;
        writeFile<<text;
        writeFile.close();
        return true;
    }

    //loading file
    bool loadFile(const char* filename) {
        if (filename == nullptr) {
            return false;
        }

        wifstream readFile(filename,ios::binary);
        if (!readFile)return false;

        //check if there's any data in editor already, clear it
        delete[] text;
        cursor_to_text_index = 0;
        length = 0;
        text = new wchar_t[capacity];

        wchar_t c;
        while (readFile.get(c)) {
            append(c);
        }
        text[length] = L'\0';
        readFile.close();
        return true;
    }


    //clamping to max possible if user selects large values
    void verifyLayout() {
        //checking if smaller than minimum
        if (total_columns < minTotalColumns) total_columns = minTotalColumns;
        if (total_lines < minTotalLines) total_lines = minTotalLines;
        if (line_length < minLineLength) line_length = minLineLength;

        //more than 22 lines go outside window
        if (total_lines > 22) total_lines = 22;


        //reducing columns
        while (total_columns > 1 && line_length * total_columns > MAX_CAPACITY) {
            total_columns--;
        }

        //if after reducing columns line length is large, reducing line length
        while (line_length > minLineLength && line_length * total_columns > MAX_CAPACITY) {
            line_length--;
        }
    }

    //setters for run time layout changes
    void setTotalLines(int n) {
        int temp = total_lines;
        total_lines = n;
        verifyLayout();
        //remake the layout
        if (temp != total_lines) {
            //clear old
            delete[] pages;
            
            //allocate pages
            pages = new page[total_pages];
            for (int i = 0; i < total_pages; i++) {
                pages[i] = page(total_columns, total_lines);
            }
            recalculateLayout();
            page_index = findCurrentPage();
        }
    }
    void setTotalColumns(int n) {
        int temp = total_columns;
        total_columns = n;
        verifyLayout();
        if (temp != total_columns) {
            //clear old
            delete[] pages;

            //allocate pages
            pages = new page[total_pages];
            for (int i = 0; i < total_pages; i++) {
                pages[i] = page(total_columns, total_lines);
            }
            recalculateLayout();
            page_index = findCurrentPage();
        }
    }
    void setLineLength(int n) {
        int temp = line_length;
        line_length = n;
        verifyLayout();
        if (temp != line_length) {
            //clear old
            delete[] pages;

            //allocate pages
            pages = new page[total_pages];
            for (int i = 0; i < total_pages; i++) {
                pages[i] = page(total_columns, total_lines);
            }

            recalculateLayout();
            page_index = findCurrentPage();
        }
    }



    //helper for finding current column and line for cursor calculation
    void getColumnAndLine(unsigned long long index, int* arr) {
          for (int p = 0; p <= max_page; p++) {
            for (int c = 0; c < total_columns; c++) {
                for (int l = 0; l < total_lines; l++) {
                    unsigned long long start = pages[p].getColumns()[c].getLine()[l].start;
                    unsigned long long len = pages[p].getColumns()[c].getLine()[l].len;
                    if (start == -1) continue; // skip empty lines
              
                    if (index >= start && index < start + len) {
                        arr[0] = c;
                        arr[1] = l;
                        return;
                    }
                }
            }
        }
          //else
          arr[0] = total_columns - 1;
          arr[1] = total_lines - 1;

    }


    //text pushback
    void append(wchar_t c) {
       if (length >= capacity - 2) {
            wchar_t* copy = new wchar_t[capacity * 2];
            for (unsigned long long j = 0; j < length; j++) {
                copy[j] = text[j];
            }
            capacity *= 2;
            delete[] text;
            text = copy;
        }

        text[length] = c;
        length++;

        text[length] = L'\0';
        cursor_to_text_index = length;
       
    }

    //pages array resizing
    void resizePages() {
        page* copy = new page[total_pages * 2];

        for (int i = 0; i < total_pages; i++) {
            copy[i] = pages[i];
        }

        for (int i = total_pages; i < total_pages * 2; i++) {
            copy[i] = page(total_columns, total_lines);
        }

        delete[] pages;
        pages = copy;
        total_pages *= 2;
    }

    //helper for recalculateLayout
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


    //finding current page to display
    int findCurrentPage() {
        //loop till cursor index
        for (int p = 0; p <= max_page; p++) {
            for (int c = 0; c < total_columns; c++) {
                for (int l = 0; l < total_lines; l++) {
                    int start = pages[p].getColumns()[c].getLine()[l].start;
                    int len = pages[p].getColumns()[c].getLine()[l].len;

                    if (cursor_to_text_index >= start && cursor_to_text_index < start + len) {
                        return p;
                    }
                }
            }
        }
        //nothing found
        return max_page;
    }
    
    //BACKBONE OF EDITOR
    //filling our layout
    void recalculateLayout() {
        // Read through `text`, break it into lines/columns/pages
        words = 0; withoutSpaces = 0; sentences = 0;
        // Clear previous layout
        for (int p = 0; p <= max_page; p++) {
            for (int c = 0; c < total_columns; c++) {
                for (int l = 0; l < total_lines; l++) {
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;
                    pages[p].getColumns()[c].getLine()[l].screenX = startX;
                    pages[p].getColumns()[c].getLine()[l].screenY = startY;
                }
            }
        }

        unsigned long long track = 0; //iterating over text
        int p = 0;//represents current page
        int c = 0;//represents current column
        int l = 0; //represents current line
        int total = 0;//current chars on line 
        while (true) {
           //text buffer filled
            if (track >= length) {
                max_page = p;
                //calculate without spaces chars
                for (unsigned long long j = 0; j < length; j++) {
                    if (text[j] != L' ' && text[j] != L'\n')withoutSpaces++;

                    if (text[j] == '.' || text[j] == '?' || text[j] == '!')sentences++;
                }
             

                return;
            }

            if (p >= total_pages) {
                resizePages();
            }

            //get len of word
            int len = getLen(&text[track]);


            //if -1 go to next line/column/page whatever is in place
            if (len == -1) {
                words++;
                pages[p].getColumns()[c].getLine()[l].len = total;
                //move to next line
                if (l < total_lines - 1) {
                    l++; total = 0;
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    track++;
                    continue;
                }
                else if (c < total_columns - 1) {
                    c++; l = 0; total = 0;
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    track++;
                    continue;
                }
                //page break
                else {
                    p++;
                    //if pages full
                    if (p >= total_pages)resizePages();
                    c = 0;
                    l = 0;
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;
                    total = 0;

                    track++;
                    continue;
                }

                continue;
            }


            total += len;
            //if it fits
            if (total <= line_length) {
                words++;
                if (pages[p].getColumns()[c].getLine()[l].start == -1) {
                    pages[p].getColumns()[c].getLine()[l].start = track;
                }
                pages[p].getColumns()[c].getLine()[l].len = total;
                //move index
                track += len;
                continue;
            }
            //else wrap
            else {
                words++; 
                // Save current line without this word
                if (pages[p].getColumns()[c].getLine()[l].start != -1) {
                    pages[p].getColumns()[c].getLine()[l].len = total - len;
                }
                else {
                    // Line is empty,so truncate the word. fit it here and move the track
                    pages[p].getColumns()[c].getLine()[l].start = track;
                    pages[p].getColumns()[c].getLine()[l].len = line_length;

                    len -= line_length;
                    track += line_length;
                    total = 0;
                }

                //MOVEMENT OF LINE/COLUMN/PAGE(whatever is required)
                if (l < total_lines - 1) {
                    l++;
                    //set starting index for new line, where old world wrapped
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;

                }
                //else move to next column top
                else if (c < total_columns - 1) {
                    c++;
                    l = 0;
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;

                }
                //page break
                else {
                    p++;
                    //if pages full
                    if (p >= total_pages)resizePages();
                    c = 0;
                    l = 0;

                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;

                }

            }

            //SPECIAL CASE
            //if a word is longer than line
            while (len > line_length) {
               
                if (pages[p].getColumns()[c].getLine()[l].start == -1) {
                    pages[p].getColumns()[c].getLine()[l].start = track;
                }
                pages[p].getColumns()[c].getLine()[l].len = line_length;
                len -= line_length;
                track += line_length;//remaining word in next line
                total = 0;

                //move line
                if (l < total_lines - 1) {
                    l++;
                    //set starting index for new line, where old world wrapped
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;
                }
                else if (c < total_columns - 1) {
                    c++;
                    l = 0;
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;
                }
                else {
                    p++;
                    //if pages full
                    if (p >= total_pages)resizePages();
                    c = 0;
                    l = 0;

                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;

                    pages[p].getColumns()[c].getLine()[l].start = track;
                    total = 0;
                }


            }


        }
        if (length == 0)words = 0;
    }
  

    //function for inserting in our text buffer
    void insertAt(unsigned long long& i, wchar_t c) {

     

        if (i > length)i = length;
        if (i == length) {
            append(c);
            cursorX = getmaxCursorX();
            cursorY = getmaxCursorY();
            return;
        }
               
        //safety for index and capacity
        if (length >= capacity - 3) {
            wchar_t* copy = new wchar_t[capacity * 2];
            for (unsigned long long j = 0; j < length; j++) {
                copy[j] = text[j];
            }
            delete[] text;
            text = copy;
            capacity *= 2;
        }

        //starting column,line
        int CL_old[2];
        getColumnAndLine(i, CL_old);
     
        //shift text array from right
        //as text[index] is \0
        unsigned long long len = length + 1;
        text[len] = '\0';
        for (unsigned long long j = len - 1; j > i; j--) {
            text[j] = text[j - 1];
        }
        text[i] = c;
        length++;
        i++;

        recalculateLayout();
        page_index = findCurrentPage();
        //increment cursor
        int CL_new[2];
        getColumnAndLine(i, CL_new);

        //wprintf(L"=====OLD i: %d=====\nColumn: %d , Line: %d\n", i-1,CL_old[0], CL_old[1]);

        //wprintf(L"=====NEW i: %d=====\nColumn: %d , Line: %d\n",i, CL_new[0], CL_new[1]);
       
        //if line and column both same
        if (CL_old[0] == CL_new[0] && CL_old[1] == CL_new[1]) {
            cursorX += char_width;
    
        }
        //if change of page
        else if (CL_old[0] == total_columns - 1 && CL_new[0] == 0) {
            page_index++;
            cursorX = startX;
            cursorY = startY; 
        }
        //if change of column/line
        else{
            cursorX = pages[page_index].getColumns()[CL_new[0]].getLine()[CL_new[1]].screenX;
            cursorY = pages[page_index].getColumns()[CL_new[0]].getLine()[CL_new[1]].screenY;
        }


    }

    //function for deletion in our text buffer
    void deleteBack(unsigned long long& i) {
        if (i <= 0) {
            i = 0;
            return;
        };

        int arr[2]={0};
        getColumnAndLine(i, arr);
        //normal deletion from the end
        if (i == length) {
            length--;
            text[length] = L'\0';
            i = length;
        }

        //shifting array
        else {
            for (unsigned long long j = i; j < length -1; j++) {
                text[j] = text[j + 1];
            }
            
            //update position
            i--;
            //reduce length
            length--;
            text[length] = L'\0';
        }



        //moving cursor accordingly
        recalculateLayout();
        page_index = findCurrentPage();

        int arr2[2] = { 0 };
        getColumnAndLine(i, arr2);


        //move cursor
        
        if (arr[0] == arr2[0] && arr[1] == arr2[1]) {
            cursorX -= char_width;
        }
        //previous line same column
        else if (arr[0] == arr2[0] && arr[1] > arr2[1]) {
            cursorY = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].screenY;

            int len = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].len;

            int temp = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].screenX + len * char_width;
            cursorX = temp;
        }
        //previous column
        else if (arr[0] > arr2[0]) {
            cursorY = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].screenY;

            int len = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].len;

            int temp = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].screenX + len * char_width;
            cursorX = temp;
        }
        //page difference
        else {
            if (page_index > 0) page_index--;

            cursorY = lineHeight * total_lines;

            int len = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].len;

            int temp = pages[page_index].getColumns()[arr2[0]].getLine()[arr2[1]].screenX + len * char_width;
            cursorX = temp;
        }
       
    }
    //delete key
    void deleteForward() {
        //do nothing if no text on right
        if (cursor_to_text_index >= length) {
            return;
        }
      
        //cursor index remains same. just shift the array
        for (unsigned long long j = cursor_to_text_index; j < length - 1; j++) {
            text[j] = text[j + 1];
        }
        length--;
        text[length] = L'\0';
        recalculateLayout();
        page_index = findCurrentPage();
    }


    //deleting selected text, always deletes from start to end of selected portion
    void deletePortion() {
        
        unsigned long long start = start_selection < end_selection ? start_selection : end_selection;
        unsigned long long end= start_selection > end_selection ? start_selection : end_selection;


        if (start == end || start <0 || end <0 || start>length)return;

        unsigned long long index = start;
        for (unsigned long long j = end; j < length; j++) {
            text[index] = text[j];
            index++;
        }
        length = index;
        cursor_to_text_index = length;
        text[length] = L'\0';
        recalculateLayout();
        page_index = findCurrentPage();
    }
    

    //select all
    void selectAll() {
        recalculateLayout();
        page_index = findCurrentPage();
        end_selection = pages[page_index].getColumns()[0].getLine()[0].start;
        int c = total_columns - 1;
        int l = total_lines - 1;
        //find max line written
        while (pages[page_index].getColumns()[c].getLine()[l].start== -1) {
            if (l > 0)l--;

            else {
                l = total_lines - 1;
                c--;
            }
        }
        start_selection = pages[page_index].getColumns()[c].getLine()[l].start+ pages[page_index].getColumns()[c].getLine()[l].len;
        start_selection-=2;
        selectionState = true;
    }


    bool compareStr(const wchar_t* ptr1, const wchar_t* ptr2) {
        while (*ptr1 && *ptr2) {
            if (*ptr1 != *ptr2 && *ptr1 + 32 != *ptr2 && *ptr1 - 32 != *ptr2)return false;

            ptr1++; ptr2++;

         }

        if (*ptr1 || *ptr2)return false;

        return true;

    }

    void writeToStr(const wchar_t* read, wchar_t* write) {
        while (*read) {
            *write = *read;
            read++; write++;
        }
        *write = '\0';
    }

    //save search to history
    void saveSearch(const wchar_t* str) {

        //check if it exists, if it does, increment the count and return
        for (int i = 0; i < searchHistoryCapacity; i++) {
            if (compareStr(str, searchHistory[i])) {
                searchHistoryCounts[i]++;
                return;
            }
        }
        

        //shifting both arrays to right
        for (int i = searchHistoryCapacity-1; i > 0; i--) {
            writeToStr(searchHistory[i - 1], searchHistory[i]);
            searchHistoryCounts[i] = searchHistoryCounts[i - 1];
        }

        //adding on top
        writeToStr(str, searchHistory[0]);
        searchHistoryCounts[0] = 1;


        for (int i = 0; i < 5; i++) {
         //   wprintf(L"string: %s , count: %d\n", searchHistory[i],searchHistoryCounts[i]);
        }


    }

    //implement search
    void executeSearch() {
        int p = page_index;

        //add to history
        saveSearch(searchText);

        //clear old highlights
        clear_highlights();


        //get max and min index of this page
        unsigned long long max = 0, min = pages[p].getColumns()[0].getLine()[0].start;
        for (int c = 0; c < total_columns; c++) {
            for (int l = 0; l < total_lines; l++) {
                unsigned long long start = pages[p].getColumns()[c].getLine()[l].start;
                unsigned long long index = start+pages[p].getColumns()[c].getLine()[l].len;
                
                if (start == -1)continue;
                if (index > max)max = index-2;
                if (min == -1) {
                    min = start;
                }

            }
        }


        int index = 0;
        unsigned long long j = min;
        while (j <= max) {           
            //if char found
            if (text[j] == searchText[index]  || text[j]+32 == searchText[index] || text[j]-32 == searchText[index]) {
                index++;
            }
            //if substring matched
            else if(searchText[index]==L'\0') {
                pushback_highlight(j - searchIndex-1, searchIndex);
             //   wprintf(L"%c\n", text[j - searchIndex-1]);
                

                //reset for next search
                index = 0;
            }
            //reset index
            else {
                index = 0;
            }

            j++;
        }
        //last word check
        if (searchText[index] == L'\0') {
            pushback_highlight(j - searchIndex-1, searchIndex);
        }


    }

};

//multiple tabs
class Tabs {
private:
    Editor* tab;
    int size;
    int current_tab;
    const int MAX_TABS = 10;
public:
    //constructor
    Tabs() {
        size = 1;
        current_tab = 0;
        tab = new Editor[size];
    }
    //destructor
    ~Tabs() {
        delete[] tab;
    }
    //if user wants to add new tab
    void appendTabs() {
        if (size >= MAX_TABS)return;

        //wprintf(L"appendTabs called\n");
        //resize
            Editor* copy = new Editor[size +1];
            for (int i = 0; i < size; i++) {
                copy[i] = tab[i];
            }
            
            delete[] tab;
            tab = copy;
           // wprintf(L"successfully resized\n");
  
        size++;
    }

    //get current working tab
    Editor& getActiveEditor() {
        return tab[current_tab];
    }
    int getTotalTabs() {
        return size;
    }

    int getCurrentTabIndex() {
        return current_tab;
    }
    //switching view to next tab    
    void incrementTab() {
        if(current_tab+1<size)
        current_tab++;
        getActiveEditor().recalculateLayout();
    }
    //previous
    void decrementTab() {
        if (current_tab - 1 >= 0) {
            current_tab--;
            getActiveEditor().recalculateLayout();
        }
    }

    //switching to nth tab
    void switchTo(int i=0) {
        wprintf(L"size(1 index based): %d    , current tab(0 index): %d \n", size, i);
        if (i < 0)i = 0;
        else if (i >=size)i = size-1;        
        current_tab = i;
        getActiveEditor().recalculateLayout();
    }

    //closing current tab, only if there are more than 1 tabs
    void closeTab() {
        if (size < 2)return;

        //make copy tabs missing this one
        Editor* copy = new Editor[size - 1];
        int j = 0;
        for (int i = 0; i < size; i++) {
            if (i == current_tab)continue;

            copy[j] = tab[i];
            j++;
        }
        size = j;
        delete[] tab;
        tab = copy;     

        //show previous
        if (current_tab - 1 >= 0) {
            current_tab--;
        }        
    }

};








