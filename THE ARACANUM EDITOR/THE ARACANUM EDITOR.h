#pragma once
#include "resource.h"
#include<windows.h>
#include <cstdio>
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



//editor: consists of multiple pages, operations, text buffer, and methods
class Editor {
private:
    unsigned long long capacity;
    wchar_t* text;
    unsigned long long length;

    //pages
    page* pages;
    int page_index;

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

    //max line_length
    const int MAX_CAPACITY=90;

public:
    //constructor
    Editor() {
        minLineLength = 5;
        minTotalLines = 1;
        minTotalColumns = 1;

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

        selectionState=false;
        start_selection-1;
        end_selection=-1;
        start_selectionX=-1;
        start_selectionY-1;
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
    int getPageIndex() const {
        return page_index;
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
        }
    }



    //helper for finding current column and line for cursor calculation
    void getColumnAndLine(unsigned long long index, int* arr) {
          for (int p = 0; p <= page_index; p++) {
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

    //filling our layout
    void recalculateLayout() {
        // Read through `text`, break it into lines/columns/pages
        words = 0; withoutSpaces = 0;
        // Clear previous layout
        for (int p = 0; p <= page_index; p++) {
            for (int c = 0; c < total_columns; c++) {
                for (int l = 0; l < total_lines; l++) {
                    pages[p].getColumns()[c].getLine()[l].start = -1;
                    pages[p].getColumns()[c].getLine()[l].len = 0;
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
                page_index = p;
                //calculate without spaces chars
                for (unsigned long long j = 0; j < length; j++) {
                    if (text[j] != L' ' && text[j] != L'\n')withoutSpaces++;
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

        wprintf(L"index: %d\n", i);
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
    }
    

    //select all
    void selectAll() {
        recalculateLayout();
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

        wprintf(L"start:  %d , end: %d\n", end_selection, start_selection);
    }

};






