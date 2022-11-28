/*
 * textrender.h
 *
 *  Created on: Sep 21, 2022
 *      Author: Kenny
 */

#ifndef TEXTRENDER_H_
#define TEXTRENDER_H_

#include <string>
#include "../mathcommon.h"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::vec2   Size;       // Size of glyph
    glm::vec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
    unsigned int vaoID, vboID;
};

inline float originalFontSize = 48.0f;
inline float fontSize = 48.0f;
inline float scale = 1.0f;

inline int startHeight, endHeight;

void initRenderer();

void clear();
void clearBottom(float startY);
float renderChar(unsigned int c, float x, float y);

void cleanup();

void clearScreen(int width, int height);

void setScale(float scale);
void setColor(float r, float g, float b);

void removeCaret(float x, float y);
void renderCaret(float x, float y);

float deleteChar(unsigned int c, float x, float y);

void clearLine(float x, float y);

void changeHeight(float y);

float caretHeight();

#endif /* TEXTRENDER_H_ */
