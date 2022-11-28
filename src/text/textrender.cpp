/*
 * textrender.cpp
 *
 *  Created on: Sep 21, 2022
 *      Author: Kenny
 *
 * responsible solely for handling fonts and drawing them to the screen. Other files should not have to touch rendering code
 */

#include <ft2build.h>
#include FT_FREETYPE_H

#include "textrender.h"

#include "../mathcommon.h"
#include "../engine/shader.h"

#include <map>
#include <iostream>
#include <cmath>

#define NUMCHARS 1024

Character charMap[NUMCHARS];
Shader *shader;

int viewportWidth, viewportHeight;

void initRenderer(){
    glClearColor(0.3, 0.2, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT);

	viewportWidth = 800; viewportHeight = 600;
	startHeight = 0; endHeight = startHeight + viewportHeight;

	shader = new Shader("text.vs", "text.fs");

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
	    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	    exit(1);
	}

	FT_Face face;
	if (FT_New_Face(ft, "fonts/ProggyVectorRegular.ttf", 0, &face)) {
	    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	    exit(1);
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (FT_ULong c = 0; c < NUMCHARS; c++)
	{
		//std::cout << "char " << c << " " << static_cast<int>(c) << std::endl;
	    // load character glyph
	    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
	    {
	        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
	        continue;
	    }
	    // generate texture
	    unsigned int texture;
	    glGenTextures(1, &texture);
	    glBindTexture(GL_TEXTURE_2D, texture);
	    glTexImage2D(
	        GL_TEXTURE_2D,
	        0,
	        GL_RED,
	        face->glyph->bitmap.width,
	        face->glyph->bitmap.rows,
	        0,
	        GL_RED,
	        GL_UNSIGNED_BYTE,
	        face->glyph->bitmap.buffer
	    );
	    // set texture options
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F); //GL_CLAMP_TO_EDGE
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    // configure VAO/VBO for texture quads
		// -----------------------------------
	    unsigned int vaoID, vboID;
		glGenVertexArrays(1, &vaoID);
		glGenBuffers(1, &vboID);
		glBindVertexArray(vaoID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		float sizeX = face->glyph->bitmap.width;
		float sizeY = face->glyph->bitmap.rows;

		float bearingX = face->glyph->bitmap_left;
		float bearingY = face->glyph->bitmap_top;

		float xpos = 0 + bearingX;
		float ypos = 0 - bearingY;

		float w = sizeX;
		float h = sizeY;

		// update VBO for each character
		float vertices[6][4] = {
					{ xpos,     ypos + h,   0.0f, 1.0f },
					{ xpos,     ypos,       0.0f, 0.0f },
					{ xpos + w, ypos,       1.0f, 0.0f },

					{ xpos,     ypos + h,   0.0f, 1.0f },
					{ xpos + w, ypos,       1.0f, 0.0f },
					{ xpos + w, ypos + h,   1.0f, 1.0f }
				};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, texture);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

	    // now store character for later use
	    Character character = {
	        texture,
	        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	        (unsigned int) face->glyph->advance.x,
			vaoID, vboID
	    };
	    charMap[c] = character;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

	// activate corresponding render state
	shader->use();
	glActiveTexture(GL_TEXTURE0);
	shader->setVec3("textColor", {1, 1, 1});
	/*glm::mat4 view = glm::lookAt(
			{0, 0, 6},
			{},
			{});*/
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), 0.0f);
	shader->setMat4("projection", projection);
}

float renderChar(unsigned int idx, float x, float y){
	using namespace std;

	y -= startHeight;

	//shader->use();
	//assume shader is already in use
	shader->setVec2("position", x, y + fontSize);
	//glActiveTexture(GL_TEXTURE0);

	auto ch = charMap[idx];

	glBindVertexArray(ch.vaoID);
	//glEnableVertexAttribArray(0);

	// render glyph texture over quad
	glBindTexture(GL_TEXTURE_2D, ch.TextureID);

	// render quad
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    //glBindVertexArray(0);
    //glBindTexture(GL_TEXTURE_2D, 0);

	return (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))*/
}

void clear(){
	glClear(GL_COLOR_BUFFER_BIT);
}

void clearBottom(float startY){
	glScissor(0,
			0,
			viewportWidth, viewportHeight - startY);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.3, 0.2, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT); // (or whatever buffer you want to clear)
	glDisable(GL_SCISSOR_TEST);
}

float caret_vals[4];

void clearScreen(int width, int height){
	viewportWidth = width; viewportHeight = height;
	glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

	auto projection = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f);
	shader->setMat4("projection", projection);

	auto ch = charMap['|'];
	caret_vals[1] = viewportHeight - fontSize - (ch.Size.y - ch.Bearing.y) * scale;

	endHeight = startHeight + viewportHeight;
}

float max(float a, float b){
	if(a > b)
		return a;
	return b;
}

void setScale(float newScale){
	scale = newScale;
	fontSize = originalFontSize * newScale;

	shader->use();
	shader->setFloat("scale", scale);

	auto ch = charMap['|'];

	caret_vals[0] = ch.Bearing.x + 1;
	caret_vals[1] = viewportHeight - fontSize - (ch.Size.y - ch.Bearing.y) * scale;
	caret_vals[2] = (ch.Size.x) * scale + 1;
	caret_vals[3] = max(ch.Size.y, ch.Bearing.y) * scale + 1;
}

void setColor(float r, float g, float b){
	//shader->use();
	shader->setVec3("textColor", r, g, b);
}

#define CARET

void removeCaret(float x, float y){
#ifdef CARET
	glScissor(x + caret_vals[0] - caret_vals[0] - 1,
			caret_vals[1] - (y - startHeight),
			std::ceil(caret_vals[2]) + 1, caret_vals[3]);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(0.3, 0.2, 0.2, 1);
	glClear(GL_COLOR_BUFFER_BIT); // (or whatever buffer you want to clear)
	glDisable(GL_SCISSOR_TEST);
#endif
}

void renderCaret(float x, float y){
#ifdef CARET
	glScissor(x + caret_vals[0] - caret_vals[0] - 1,
			caret_vals[1] - (y - startHeight),
			std::ceil(caret_vals[2]) + 1,
			caret_vals[3]);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(255.0f/255.0f,127.0f/255.0f,80.0f/255.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT); // (or whatever buffer you want to clear)
	glDisable(GL_SCISSOR_TEST);
	glClearColor(0.3, 0.2, 0.2, 1);
	//setColor(255.0f/255.0f,127.0f/255.0f,80.0f/255.0f);
	//renderChar('|', x - /*caret_vals[2] / 2.0f -*/ caret_vals[0]/* - caret_vals[2] / 2.0f*/, y);
	//setColor(1.0f, 1.0f, 1.0f);
#endif
}

float deleteChar(unsigned int c, float x, float y){
	auto ch = charMap[c];

	glScissor(x - (ch.Advance >> 6) * scale + ch.Bearing.x * scale,
			viewportHeight - (y - startHeight) - fontSize - (ch.Size.y - ch.Bearing.y) * scale,
			(ch.Size.x + ch.Bearing.x) * scale,
			max(ch.Size.y, ch.Bearing.y) * scale + 1);
	glEnable(GL_SCISSOR_TEST);
	//glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT); // (or whatever buffer you want to clear)
	glDisable(GL_SCISSOR_TEST);

	return (ch.Advance >> 6) * scale;
}

void clearLine(float x, float y){
	glScissor(x,viewportHeight - y - fontSize * 1.25 - startHeight, viewportWidth, fontSize);
	glEnable(GL_SCISSOR_TEST);
	//glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT); // (or whatever buffer you want to clear)
	glDisable(GL_SCISSOR_TEST);
}

void changeHeight(float y){
	startHeight += y; endHeight += y;
}

float caretHeight(){
	return caret_vals[3];
}

void cleanup(){
	for(int i = 0; i < NUMCHARS; i++){
		glDeleteBuffers(1, &charMap[i].vboID);
		glDeleteVertexArrays(1, &charMap[i].vaoID);
		glDeleteTextures(1, &charMap[i].TextureID);
	}

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
	delete shader;
}
