#pragma once
#include "resource.h"
#include<windows.h>
//blueprint for a single page
class Page {
private:
	HDC hdc;
	int offset = 0; //determining top for each page
	int column_size = 20; //20 lines per column
	int line_length = 40; //40 chars per line of each column
public:
	void setColSize(int n) {
		column_size = n;
	}
	void setlinelength(int n) {
		line_length = n;
	}
	void setHDC(HDC& temp) {
		hdc = temp;
	}
	void addPage() {
		Rectangle(hdc, 10, offset, 300, 400);
	}

};

