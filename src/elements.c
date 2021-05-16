#include "../include/library_internal.h"
#include <string.h>


void ma_frame_element_text_add(Frame* frame, const char* string) {
	Element text;
	text.type = ELEMENT_TEXT;
	text.size = strlen(string) + 1;
	ma_frame_element_add(frame, text, (void*) string);
}

void ma_frame_element_h1_add(Frame* frame, const char* string) {
	Element h1;
	h1.type = ELEMENT_HEADER1;
	h1.size = strlen(string) + 1;
	ma_frame_element_add(frame, h1, (void*) string);
}

void ma_frame_element_h2_add(Frame* frame, const char* string) {
	Element h2;
	h2.type = ELEMENT_HEADER2;
	h2.size = strlen(string) + 1;
	ma_frame_element_add(frame, h2, (void*) string);
}

void ma_frame_element_h3_add(Frame* frame, const char* string) {
	Element h3;
	h3.type = ELEMENT_HEADER3;
	h3.size = strlen(string) + 1;
	ma_frame_element_add(frame, h3, (void*) string);
}

void ma_frame_element_color_add(Frame* frame, unsigned char r, unsigned char g, unsigned char b) {
	Element color;
	color.type = ELEMENT_COLOR;
	color.size = 3;

	unsigned char col[3];
	col[0] = r;
	col[1] = g;
	col[2] = b;

	ma_frame_element_add(frame, color, &col);
}

void ma_frame_element_break_add(Frame* frame) {
	Element br;
	br.type = ELEMENT_BREAK;
	br.size = 0;
	ma_frame_element_add(frame, br, NULL);
}

void ma_frame_element_line_add(Frame* frame) {
	Element line;
	line.type = ELEMENT_LINE;
	line.size = 0;
	ma_frame_element_add(frame, line, NULL);
}



void ma_frame_element_text_set(Frame* frame, unsigned char index, const char* string) {
	Element text;
	text.type = ELEMENT_TEXT;
	text.size = strlen(string) + 1;
	ma_frame_element_set(frame, text, index, (void*) string);
}

void ma_frame_element_h1_set(Frame* frame, unsigned char index, const char* string) {
	Element h1;
	h1.type = ELEMENT_HEADER1;
	h1.size = strlen(string) + 1;
	ma_frame_element_set(frame, h1, index, (void*) string);
}

void ma_frame_element_h2_set(Frame* frame, unsigned char index, const char* string) {
	Element h2;
	h2.type = ELEMENT_HEADER2;
	h2.size = strlen(string) + 1;
	ma_frame_element_set(frame, h2, index, (void*) string);
}

void ma_frame_element_h3_set(Frame* frame, unsigned char index, const char* string) {
	Element h3;
	h3.type = ELEMENT_HEADER3;
	h3.size = strlen(string) + 1;
	ma_frame_element_set(frame, h3, index, (void*) string);
}

void ma_frame_element_color_set(Frame* frame, unsigned char index, unsigned char r, unsigned char g, unsigned char b) {
	Element color;
	color.type = ELEMENT_COLOR;
	color.size = 3;

	unsigned char col[3];
	col[0] = r;
	col[1] = g;
	col[2] = b;

	ma_frame_element_set(frame, color, index, &col);
}
