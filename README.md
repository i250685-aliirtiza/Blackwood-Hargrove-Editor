Blackwood & Hargrove
THE ARCANUM EDITOR
Technical Reference & Developer Documentation

Author: i250685-aliirtiza  |  Platform: Win32 API (C++)  |  Started: March 17, 2026
 
1. Project Overview
Blackwood & Hargrove is a from-scratch, native Windows text editor built in C++ using the Win32 API. It implements every major editor system — text buffer management, word-wrap layout, multi-column/multi-page rendering, cursor tracking, text selection, search and highlight, clipboard integration, file I/O, and multi-tab editing — without any third-party UI framework.

The project was built incrementally over one week (March 17–23, 2026), with each commit adding a discrete, working feature on top of the last. The architecture is fully object-oriented, centered on three core classes: Line/column/page for layout, Editor for all buffer and rendering logic, and Tabs for multi-document management.

2. Architecture
2.1 Data Structures
The layout system is a three-tier hierarchy:

struct Line	One visual line of text. Stores start index into the text buffer, character length, and rendered screen coordinates (screenX, screenY).
class column	An array of Lines. Represents one column on a page. Owns its Line array with RAII (copy constructor + assignment operator + destructor).
class page	An array of columns. Represents one full page. Also RAII-managed. Default layout is 3 columns × 10 lines.
struct Highlight	A start index + length pair marking a search result region in the text buffer.

2.2 Class: Editor
The central class. It owns the raw wchar_t* text buffer, the page* layout array, and all editor state (cursor position, selection, search, history). Every operation — insert, delete, layout, render, file I/O — is a method of this class.

2.3 Class: Tabs
A thin wrapper over an Editor* array. It tracks the active editor, supports up to 10 tabs, and exposes navigation methods. The global Tabs tab object in the .cpp file is the single entry point for all WndProc event handlers.

3. Feature Development (Commit Timeline)
March 17 — Project Start
•	Initial Win32 window scaffolding: WinMain, WndProc, WNDCLASSW, message loop.
•	Cursor rendering: blinking cursor via WM_TIMER at 200ms intervals using FillRect on a 1px-wide RECT.

March 18 — Long Word Wrapping
•	recalculateLayout() introduced — the backbone of the editor. Iterates the text buffer word by word and fills Lines within columns within pages.
•	Special-case: words longer than line_length are truncated across multiple lines rather than overflowing.

March 19 — Click-to-Edit
•	generateCursorInfo(x, y) implemented: converts a mouse screen coordinate to a valid text buffer index by scanning stored screenX/screenY values from the layout.
•	WM_RBUTTONDOWN wired to generateCursorInfo — right-click places the cursor.

March 20 — OOP Refactor + insertAt + Footer
•	Entire procedural codebase converted to the Line / column / page / Editor class hierarchy. Header file THE ARCANUM EDITOR.h created.
•	insertAt(index, char): shifts the text buffer right from index, inserts the character, then calls recalculateLayout() and repositions the cursor by comparing old/new column+line from getColumnAndLine().
•	Footer added to render(): displays total words, total characters, characters without spaces, and page number.

March 21 — Selection, Deletion, Clipboard, File I/O
•	Text selection: WM_LBUTTONDOWN starts selection (setCapture), WM_MOUSEMOVE extends it, WM_LBUTTONUP finalises it. Selection stored as start_selection / end_selection buffer indices.
•	deletePortion(): deletes the selected range by shifting the buffer left from end_selection to start, then calls recalculateLayout().
•	deleteBack(index): backspace — shifts buffer left from index. If at end, just decrements length. Repositions cursor using before/after getColumnAndLine() comparison.
•	deleteForward(): delete key — shifts buffer left from cursor_to_text_index without moving the cursor index.
•	selectAll(): finds the first and last valid buffer positions across the current page and sets selection accordingly.
•	Ctrl+C: copies selected range to Windows clipboard via OpenClipboard / GlobalAlloc / SetClipboardData (CF_UNICODETEXT).
•	Ctrl+V: reads from clipboard via GetClipboardData, then calls insertAt() for each character.
•	saveFile(filename): opens a wofstream and writes the text buffer. Defaults to file.txt.
•	loadFile(filename): opens a wifstream in binary mode, clears the buffer, and calls append() for each character read.

March 22 — Search, History, Page Navigation, render() Refactor
•	render() moved from WM_PAINT into a member function of Editor, taking HDC, HWND, and start_time as parameters.
•	Page navigation: VK_UP decrements page_index, VK_DOWN increments it. Cursor repositions via generateCursorInfo().
•	Search (Ctrl+F): activates searchState. WM_CHAR routes printable characters into searchText[]. ENTER calls executeSearch(), ESC clears.
•	executeSearch(): scans the current page's buffer range character-by-character (case-insensitive via ±32 trick), stores matches in the marked_text Highlight array via pushback_highlight().
•	Highlighted rendering: if a line contains marked text, the render loop draws it character-by-character, switching SetTextColor to red for marked spans.
•	Search history: saveSearch() maintains a fixed-size ring of 5 recent queries with occurrence counts. Ctrl+Shift+H toggles the history display in the footer.
•	Sentence counter: recalculateLayout() counts '.', '?', '!' occurrences and stores in sentences, shown in footer.

March 23 — Multi-Tab Editing
•	Tabs class added: wraps an Editor* array, supports appendTabs(), closeTab(), incrementTab(), decrementTab(), switchTo(i).
•	Ctrl+N: appends a new tab and switches to it.
•	Ctrl+Tab / Ctrl+Shift+Tab: cycle forward/backward through tabs.
•	Ctrl+Q: closes the current tab (minimum 1 tab enforced).
•	Runtime layout controls added: Ctrl+L (lines), Ctrl+W (line length), Ctrl+K (columns) open input bars at the top of the window. User types a number and presses ENTER; the value is passed to setTotalLines(), setLineLength(), or setTotalColumns() which rebuild the page array and call recalculateLayout().
•	Validation fix: copy operation now checks selection state before proceeding to prevent out-of-bounds access.

4. Function Reference
4.1 Editor — Core Text Buffer
append(wchar_t c)	Pushes c onto the end of the text buffer, doubling capacity if needed. Sets cursor_to_text_index = length.
insertAt(index, c)	Inserts c at position index by right-shifting the buffer. Calls recalculateLayout() and updates cursor position by comparing column/line before and after insertion.
deleteBack(index)	Backspace. Shifts buffer left from index (or just decrements length if at end). Recalculates layout and moves cursor left by comparing column/line before and after.
deleteForward()	Delete key. Shifts buffer left from cursor_to_text_index. Does not move cursor. Recalculates layout.
deletePortion()	Deletes the selected text range (start_selection to end_selection). Shifts buffer, sets cursor to new length, recalculates.
selectAll()	Sets start_selection and end_selection to cover all visible text on the current page. Enables selectionState.

4.2 Editor — Layout Engine
recalculateLayout()	THE backbone. Clears all Line entries, then iterates text[] word by word (via getLen()). Fills Lines, columns, pages in order. Handles newlines, word-wrap, and long-word truncation. Also counts words, non-space characters, and sentences.
getLen(wchar_t* ptr)	Returns the character count of the next word including trailing space, or -1 if the next character is a newline.
verifyLayout()	Clamps total_lines, total_columns, line_length to their minimums and reduces them if their product exceeds MAX_CAPACITY (90 chars).
findCurrentPage()	Searches all pages/columns/lines for the one containing cursor_to_text_index. Returns the page index.
getColumnAndLine(index, arr)	Returns the column and line index (in arr[0], arr[1]) of the Line containing text buffer index. Used before and after insert/delete to compute cursor movement.
resizePages()	Doubles the pages array, copying existing pages and initialising new ones.
setTotalLines(n)	Validates and sets total_lines, rebuilds the pages array, recalculates layout.
setTotalColumns(n)	Same as setTotalLines but for columns.
setLineLength(n)	Same as setTotalLines but for line length.

4.3 Editor — Rendering
render(hdc, hwnd, start_time, current_tab, total_tabs)	Full repaint. Clears background, iterates pages[page_index] to draw text via TextOutW. Handles highlighted (red) and selected (blue) character drawing. Draws cursor rect (blinks every 2 seconds). Draws footer stats and page number. Draws search bar or layout input bar at y=0 if active.
convertToStr(int, wchar_t[])	Converts an integer to a wide-char string. Used for footer stats display.
merge(r1, r2, write)	Concatenates two wide-char strings into write. Returns combined length.
concatenate(r1, r2, r3, write)	Concatenates three wide-char strings into write.

4.4 Editor — Search
executeSearch()	Saves query to history, clears old highlights, then scans the current page's buffer range character-by-character. Matches are case-insensitive (±32). Matched spans are stored via pushback_highlight().
pushback_highlight(start, len)	Appends a Highlight to marked_text[], doubling capacity if needed.
clear_highlights()	Frees and reinitialises the marked_text[] array.
saveSearch(str)	Checks if str is already in searchHistory[]. If so, increments its count. Otherwise shifts the ring and inserts at [0] with count 1.
compareStr(ptr1, ptr2)	Case-insensitive wide string comparison (handles ±32 offset). Used by saveSearch.
clearSearchandLayoutInfo()	Resets all search and layout-info state flags, indices, and buffers to defaults.

4.5 Editor — File I/O
saveFile(filename)	Opens filename with wofstream and writes text[]. Defaults to file.txt. Returns false on failure.
loadFile(filename)	Opens filename in binary mode with wifstream. Clears the buffer, reads character by character via append(). Returns false on failure.

4.6 Tabs — Multi-Document
appendTabs()	Resizes the Editor array by 1 (up to MAX_TABS=10), copying existing editors via the assignment operator.
getActiveEditor()	Returns a reference to tab[current_tab].
incrementTab()	Moves to next tab and calls recalculateLayout().
decrementTab()	Moves to previous tab and calls recalculateLayout().
switchTo(i)	Clamps i to valid range, sets current_tab, calls recalculateLayout().
closeTab()	Copies all editors except current into a new array. Moves current_tab to previous if possible.

4.7 WndProc Helpers
generateCursorInfo(x, y, index, cursorX, cursorY)	Converts screen coordinates to a text buffer index. Scans stored screenY values to find the active line, then the active column by checking screenX ranges, then approximates character offset by stepping char_width until reaching x.
convertToInt(wchar_t*)	Parses a wide-char digit string into an int. Used for runtime layout number input.

5. Keyboard Shortcut Reference
Ctrl+S	Save current document to dummy.txt
Ctrl+O	Load sample-text_for_editor.txt into current tab
Ctrl+N	Open new tab (max 10)
Ctrl+Q	Close current tab
Ctrl+Tab	Switch to next tab
Ctrl+Shift+Tab	Switch to previous tab
Ctrl+F	Open search bar
Ctrl+Shift+H	Toggle search history panel
Ctrl+L	Open lines-per-column input
Ctrl+W	Open line-length input
Ctrl+K	Open columns-per-page input
Ctrl+A	Select all text on current page
Ctrl+C	Copy selected text to clipboard
Ctrl+V	Paste from clipboard at cursor
Backspace	Delete character before cursor (or selected range)
Delete	Delete character after cursor (or selected range)
Enter	Insert newline at cursor
VK_UP	Go to previous page
VK_DOWN	Go to next page
Right-click	Place cursor at click position
Left-click + drag	Select text range
ESC	Close search bar / clear highlights

6. Known Limitations & Design Notes
•	The text buffer is a flat wchar_t* array. There is no gap buffer or piece table, so insertAt() is O(n) for every keystroke.
•	Search operates only on the currently displayed page, not the full document.
•	The selection highlight renders as a single colour change on full lines, not per-character, due to the line-at-a-time TextOutW approach (unless the line is in the marked_text array, which triggers per-character drawing).
•	charWidth is derived from the average width of the last rendered line. Monospace fonts (Courier New) make this accurate; proportional fonts would break cursor alignment.
•	Undo/redo is not yet implemented.
•	File open/save use hardcoded filenames. A proper OPENFILENAME dialog is not yet wired up.


Blackwood & Hargrove  —  THE ARCANUM EDITOR  —  March 2026
