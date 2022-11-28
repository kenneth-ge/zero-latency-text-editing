/*
 * textprocess.h
 *
 *  Created on: Sep 21, 2022
 *      Author: Kenny
 */

#ifndef TEXTPROCESS_H_
#define TEXTPROCESS_H_

#include "../glcommon.h"

void init();

void char_callback(GLFWwindow* window, unsigned int codepoint);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#endif /* TEXTPROCESS_H_ */
