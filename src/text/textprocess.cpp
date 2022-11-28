/*
 * textprocess.cpp
 *
 *  Created on: Sep 21, 2022
 *      Author: Kenny
 */

#include <iostream>
#include <fstream>
#include "textprocess.h"
#include "textrender.h"

#include "../mathcommon.h"
#include "../perfprofile.h"

#include <vector>
#include <thread>
#include <nfd/nfd.h>

using namespace std;

struct datachar {
	unsigned int c;
	float width;
};

struct metadata {
	float width;
};

float x = 0.0f, y = 0.0f;
int caret = 0; //which actual character you're on
vector<vector<datachar>> lines;
vector<metadata> lineData;
int lineIdx;

void init(){
	setScale(0.75f);
	setColor(1.0f, 1.0f, 1.0f);

	lines.push_back(vector<datachar>());
	lineData.push_back({0});
}

void printData(){
	for(int i = 0; i < lines.size(); i++){
		for(int j = 0; j < lines[i].size(); j++){
			cout << ((char) lines[i][j].c);
		}
		cout << endl;
	}
}

//only clearing the bottom half of the screen SOUNDS like a good optimization, but in practice adds a ton of complexity
//that might not be worth it
void rerenderAllNoCaret(){
	//int screenStartLine = startLineIdx - startHeight / fontSize;
	clear();

	float curr_x = 0, curr_y = startHeight;
	int num_rows = (endHeight - startHeight) / fontSize;
	int start_row = curr_y / fontSize;
	for(int i = start_row; i < min(lines.size(), static_cast<unsigned>(start_row + num_rows)); i++){
		for(int j = 0; j < lines[i].size(); j++){
			curr_x += renderChar(lines[i][j].c, curr_x, curr_y);
		}
		curr_y += fontSize;
		curr_x = 0;
	}
}

void rerenderAll(){
	rerenderAllNoCaret();
	renderCaret(x, y);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    //rerender all text
	clearScreen(width, height);

	rerenderAll();
}

//#define PERF_PROFILE

void char_callback(GLFWwindow* window, unsigned int codepoint){
#ifdef PERF_PROFILE
	start_timer();
#endif
	removeCaret(x, y);
	float width = renderChar(codepoint, x, y);
	if(caret == lines[lineIdx].size()){
		lines[lineIdx].push_back({codepoint, width});
	}else{
		clearLine(x, y);
		lines[lineIdx].insert(lines[lineIdx].begin() + caret, {codepoint, width});
		float x2 = x;
		for(int i = caret;i < lines[lineIdx].size(); i++){
			x2 += renderChar(lines[lineIdx][i].c, x2, y);
		}
	}
	x += width;
	lineData[lineIdx].width += width;
	caret++;
	renderCaret(x, y);
#ifdef PERF_PROFILE
	glFlush();
	end_timer();
#endif
}

void changeLine(){
	if(y + caretHeight() > endHeight){
		changeHeight(fontSize);
	}else if(y < startHeight){
		changeHeight(-fontSize);
	}else{
		return;
	}

	rerenderAll();
}

int signum(int x){
	return (x>>31) | ((unsigned)-x >> 31);
}

bool spaceChar(int lineIdx, int caret){
	unsigned int c = lines[lineIdx][caret].c;

	switch(c){
	case ' ':
		return 1;
	default:
		return 0;
	}
}

//shift by one. only updates caret representation (x, y, caret, lineIdx)
//right is positive, left is negative
//returns amount shifted
int shift(int amt, bool untilSpace=false){
	if(amt == 0 || (untilSpace && spaceChar(lineIdx, caret))){
		return 0;
	}

	int chars_left = abs(amt);
	if(amt > 0){
		if(caret >= lines[lineIdx].size()){ //end of line
			if(lineIdx == lines.size() - 1){
				return 0;
			}

			if(untilSpace){
				return 0;
			}

			x = 0; caret = 0;
			y += fontSize; lineIdx++;
			return 1 + shift(amt - 1);
		}
	}else{
		if(caret <= 0){ //handle newline
			if(untilSpace){
				return 0;
			}

			if(lineIdx > 0){
				lineIdx--;
				caret = lines[lineIdx].size();

				x = lineData[lineIdx].width;
				y -= fontSize;
			}
			return 1 + shift(amt + 1);
		}
	}

	float moveWidth = signum(amt) * lines[lineIdx][caret].width;

	x += moveWidth;
	caret += signum(amt);

	return 1 + shift(amt + -signum(amt), untilSpace);
}

void leftshift(){
	float moveWidth = lines[lineIdx][caret - 1].width;

	x -= moveWidth;
	caret -= 1;
}

void deleteChars(bool untilSpace, int numChars=1){
	if(numChars == 0){
		renderCaret(x, y);
		return;
	}

	auto &&line = lines[lineIdx];

	removeCaret(x, y);

	if(caret == 0){
		if(lineIdx > 0){
			int lenOld = lines[lineIdx].size();
			float widthOld = lineData[lineIdx].width;

			lines[lineIdx - 1].insert(lines[lineIdx - 1].end(),
			                lines[lineIdx].begin(),
			                lines[lineIdx].end());

			lineData[lineIdx - 1].width += lineData[lineIdx].width;

			lines.erase(lines.begin() + lineIdx, lines.begin() + lineIdx + 1);
			lineData.erase(lineData.begin() + lineIdx, lineData.begin() + lineIdx + 1);

			lineIdx--;

			caret = lines[lineIdx].size() - lenOld;
			y -= fontSize;
			x = lineData[lineIdx].width - widthOld;
			rerenderAll();
		}

		numChars--;
		deleteChars(untilSpace, numChars);
		return;
	}

	if(caret == lines[lineIdx].size()){
		auto ch = line.back();

		auto w = deleteChar(ch.c, x, y);
		lineData[lineIdx].width -= w;
		x -= w;
		caret--;

		line.pop_back();

		if(untilSpace){
			while(!line.empty()){
				auto ch2 = line.back();

				if(ch2.c == ' '){
					break;
				}

				line.pop_back();

				w = deleteChar(ch2.c, x, y);
				lineData[lineIdx].width -= w;
				x -= w;
				caret--;
			}
		}
	}else{
		caret--;
		int start_idx = caret;
		int end_idx = caret + 1;

		float width_diff = lines[lineIdx][start_idx].width;

		if(untilSpace)
			while(start_idx - 1 >= 0){
				if(lines[lineIdx][start_idx - 1].c == ' ')
					break;

				width_diff += lines[lineIdx][start_idx].width;

				start_idx--;
				caret--;
			}

		lines[lineIdx].erase(lines[lineIdx].begin() + start_idx, lines[lineIdx].begin() + end_idx);
		lineData[lineIdx].width -= width_diff;

		x -= width_diff;

		clearLine(x, y);

		float x2 = x;
		for(int i = caret; i < lines[lineIdx].size(); i++){
			x2 += renderChar(lines[lineIdx][i].c, x2, y);
		}
	}

	renderCaret(x, y);

	numChars--;
	deleteChars(untilSpace, numChars);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch(action){
	//case GLFW_RELEASE:
	case GLFW_PRESS:
	case GLFW_REPEAT:
		switch(key){
		case GLFW_KEY_DOWN:
			if(lineIdx >= lines.size() - 1)
				break;

			removeCaret(x, y);
			lineIdx++;
			y += fontSize;

			if(x > lineData[lineIdx].width){
				x = lineData[lineIdx].width;
			}

			if(caret > lines[lineIdx].size()){
				caret = lines[lineIdx].size();
			}

			renderCaret(x, y);

			changeLine();
			break;
		case GLFW_KEY_UP:
			if(lineIdx == 0)
				break;

			removeCaret(x, y);

			lineIdx--;
			y -= fontSize;

			if(x > lineData[lineIdx].width){
				x = lineData[lineIdx].width;
			}

			if(caret > lines[lineIdx].size()){
				caret = lines[lineIdx].size();
			}

			renderCaret(x, y);

			changeLine();
			break;
		case GLFW_KEY_LEFT:
			if(caret <= 0){ //handle newline
				if(lineIdx > 0){
					removeCaret(x, y);

					lineIdx--;
					caret = lines[lineIdx].size();

					x = lineData[lineIdx].width;// - lines[lineIdx][lines[lineIdx].size() - 1].width;
					y -= fontSize;

					renderCaret(x, y);

					changeLine();
					break;
				}else{
					break;
				}
			}

			if(caret < 0){
				break;
			}

			removeCaret(x, y);

			if(caret < lines[lineIdx].size()){
				deleteChar(lines[lineIdx][caret].c, x + lines[lineIdx][caret].width, y);
				renderChar(lines[lineIdx][caret].c, x, y);
			}

			{
				float moveWidth = lines[lineIdx][caret - 1].width;
				int deltaCaret = 1;

				if(mods & GLFW_MOD_CONTROL)
					while(caret - deltaCaret - 1 >= 0 && lines[lineIdx][caret - deltaCaret - 1].c != ' '){
						moveWidth += lines[lineIdx][caret - deltaCaret - 1].width;
						deltaCaret++;
					}

				x -= moveWidth;
				caret -= deltaCaret;

				renderCaret(x, y);
			}
			break;
		case GLFW_KEY_RIGHT:
			if(caret >= lines[lineIdx].size()){ //end of line
				if(lineIdx == lines.size() - 1){
					break;
				}
				removeCaret(x, y);

				x = 0; caret = 0;
				y += fontSize; lineIdx++;

				renderCaret(x, y);

				changeLine();
				break;
			}

			removeCaret(x, y);

			if(caret < lines[lineIdx].size()){
				deleteChar(lines[lineIdx][caret].c, x + lines[lineIdx][caret].width, y);
				renderChar(lines[lineIdx][caret].c, x, y);
			}

			{
			float moveWidth = lines[lineIdx][caret].width;
			int deltaCaret = 1;

			if(mods & GLFW_MOD_CONTROL)
				while(caret + deltaCaret < lines[lineIdx].size() && lines[lineIdx][caret + deltaCaret].c != ' '){
					moveWidth += lines[lineIdx][caret + deltaCaret].width;
					deltaCaret++;
				}

			x += moveWidth;
			caret += deltaCaret;

			renderCaret(x, y);
			}
			break;
		case GLFW_KEY_ENTER:
			lines.insert(lines.begin() + lineIdx + 1, vector<datachar>());
			lineData.insert(lineData.begin() + lineIdx + 1, {0});
			removeCaret(x, y);
			lineIdx++; y += fontSize;

			if(lineIdx < lines.size()){
				lines[lineIdx].insert(lines[lineIdx].end(),
				                make_move_iterator(lines[lineIdx - 1].begin() + caret),
				                make_move_iterator(lines[lineIdx - 1].end()));

				lines[lineIdx - 1].erase(lines[lineIdx - 1].begin() + caret, lines[lineIdx - 1].end());

				float diff = lineData[lineIdx - 1].width - x;
				lineData[lineIdx].width = diff;
				lineData[lineIdx - 1].width -= diff;

				rerenderAllNoCaret();
			}

			caret = 0; x = 0;
			renderCaret(x, y);
			changeLine();
			break;
		case GLFW_KEY_DELETE:
		{
			int amt = shift(1);

			if(mods & GLFW_MOD_CONTROL){
				amt += shift(INT_MAX / 2, true);
			}

			deleteChars(false, amt);

			break;
		}
			//fall into next case
		case GLFW_KEY_BACKSPACE:
			deleteChars(mods & GLFW_MOD_CONTROL);
			break;
		}
		case GLFW_KEY_S:
			if(mods & GLFW_MOD_CONTROL){
				/*namespace fs = std::filesystem;

				if (!fs::is_directory("C:\\fasttexteditor") || !fs::exists("C:\\fasttexteditor")) { // Check if src folder exists
				    fs::create_directory("C:\\fasttexteditor"); // create src folder
				}

				string originalFilename = "C:\\fasttexteditor\\doc.txt";
				string filename = "C:\\fasttexteditor\\doc.txt";

				int counter = 1;
				if(fs::exists(filename)){
					counter++;

					filename = originalFilename + to_string(counter);
				}*/

				string filename = "C:\\fasttexteditor\\doc.txt";

				wofstream fout(filename);

				for(int i = 0; i < lines.size(); i++){
					for(int j = 0; j < lines[i].size(); j++){
						fout << wchar_t(lines[i][j].c);
					}
					fout << '\n';
				}

				fout.close();
			}
			break;
	}
}
