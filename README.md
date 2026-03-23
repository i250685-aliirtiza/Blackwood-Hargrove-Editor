Blackwood & Hargrove — THE ARCANUM EDITOR

Author: i250685-aliirtiza
Platform: Win32 API (C++)
Started: March 17, 2026

Overview

A native Windows text editor built from scratch in C++ using the Win32 API. Features include:

Multi-tab editing (up to 10 tabs)
Text buffer management with full insert/delete
Word-wrap layout with multi-column/multi-page rendering
Cursor tracking, text selection, and highlights
Search functionality with history
Clipboard integration (Ctrl+C/Ctrl+V)
File save/load operations

Built incrementally over one week with a fully object-oriented design.

Architecture

Core Classes:

Editor – Manages text buffer, layout, cursor, selection, search, and rendering.
Tabs – Multi-document wrapper for Editor instances.
Line / Column / Page – Layout hierarchy for rendering text efficiently.

Data Structures:

Line – Represents a single line of text with screen coordinates.
Column – Array of Lines for a page column.
Page – Array of Columns forming a page.
Highlight – Marks search results in the buffer.
Key Features
OOP Refactor – Fully object-oriented design for modularity
Text Operations – Insert, delete, select, copy, paste
Search & Highlight – Case-insensitive search with history panel
Layout Controls – Dynamic lines, columns, and line-length adjustments
Page Navigation – Up/Down arrow keys navigate pages
Footer Stats – Words, characters, sentences, and page number
Keyboard Shortcuts
Shortcut	Action
Ctrl+S	Save document
Ctrl+O	Load sample text
Ctrl+N	New tab
Ctrl+Q	Close tab
Ctrl+Tab	Next tab
Ctrl+Shift+Tab	Previous tab
Ctrl+F	Open search
Ctrl+Shift+H	Toggle search history
Ctrl+L	Lines-per-column input
Ctrl+W	Line-length input
Ctrl+K	Columns-per-page input
Ctrl+A	Select all
Ctrl+C / Ctrl+V	Copy / Paste
Backspace / Delete	Delete character(s)
Enter	Newline
VK_UP / VK_DOWN	Page navigation
Right-click	Place cursor
Left-click + drag	Select text
ESC	Close search / clear highlights

Limitations
Text buffer is flat (wchar_t*), so insertions are O(n)
Search operates only on the current page
Undo/redo not implemented
Hardcoded filenames for save/load
Monospace font recommended for proper cursor alignment

Blackwood & Hargrove — THE ARCANUM EDITOR — March 2026
