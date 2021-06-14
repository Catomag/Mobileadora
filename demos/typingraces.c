#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#include <string.h>
#include "../include/library.h"


#define PLAYER_COUNT 100

typedef struct {
	int score;
	Color col;
	Frame* frame;
} Player;

const int colors[] = {
	0x00ffffff,
	0x00ff00ff,
	0xffff88ff,
	0xff0d88ff,
	0xffdf44ff,
	0xff0000ff,
	0x44ff44ff,
	0xffd488ff,
	0x44ff88ff,
	0xff4444ff,
	0x00ff88ff,
	0x4dff88ff,
	0x00ff88ff,
	0xffff88ff,
	0x44ffdfff,
	0xff4444ff,
	0xfd0000ff,
	0xfffd00ff,
};

Player players[PLAYER_COUNT];

const char text_a[] = 
"Id just like to interject for a moment. What youre refering to as Linux, is in fact, GNU/Linux, or as Ive recently taken to calling it, GNU plus Linux. Linux is not an operating system unto itself, but rather another free component of a fully functioning GNU system made useful by the GNU corelibs, shell utilities and vital system components comprising a full OS as defined by POSIX.\nMany computer users run a modified version of the GNU system every day, without realizing it. Through a peculiar turn of events, the version of GNU which is widely used today is often called Linux, and many of its users are not aware that it is basically the GNU system, developed by the GNU Project.\nThere really is a Linux, and these people are using it, but it is just a part of the system they use. Linux is the kernel: the program in the system that allocates the machines resources to the other programs that you run. The kernel is an essential part of an operating system, but useless by itself; it can only function in the context of a complete operating system. Linux is normally used in combination with the GNU operating system: the whole system is basically GNU with Linux added, or GNU/Linux. All the so-called Linux distributions are really distributions of GNU/Linux!";

char* current_text = text_a;
unsigned int text_length = 0;

float dist_table[128];
float line_height;


int main() {
	ma_init(PLAYER_COUNT, 8000);
	Frame* join_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_h3_add(join_frame, "NAME:");
	ma_frame_input_text_add(join_frame, 15);
	ma_frame_input_submit_add(join_frame);

	Frame* main_frame = ma_frame_create(FRAME_DYNAMIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_color_add(main_frame, 255, 0, 0);
	ma_frame_input_text_add(main_frame, 2000);
	ma_frame_default(join_frame);

	
	InitWindow(1280, 720, "testapp");
	Font roboto_font = LoadFontEx("demos/roboto.ttf", 80, 0, 0);
	SetTextureFilter(roboto_font.texture, TEXTURE_FILTER_BILINEAR);

	char dummy_text[2] = "\0\0";
	for(char i = 0; i < 127; i++) {
		dummy_text[0] = i;
		dist_table[i] += MeasureTextEx(roboto_font, dummy_text, 40, 0).x;
	}
	dummy_text[0] = '\n';
	line_height = MeasureTextEx(roboto_font, dummy_text, 40, 0).y;

	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].col = GetColor(colors[i]);
		players[i].score = 0;
		players[i].frame = ma_frame_copy(main_frame);
		ma_frame_element_color_set(players[i].frame, 0, players[i].col.r, players[i].col.g, players[i].col.b);
		ma_frame_send(players[i].frame, i);
	}

	SetTargetFPS(60);

	float prev_time = 0;
	float delta = 0;

	float vel = 0.f;
	float y_off = 0;

	text_length = strlen(current_text);

	while(!WindowShouldClose()) {
		char buffer[1500];
		for(int i = 0; i < PLAYER_COUNT; i++) {
			if(ma_client_active(i)) {
				ma_client_input_text_get(i, 0, buffer);
				players[i].score = 0;
				for(int j = 0; j < text_length; j++) {
					if(current_text[j] == buffer[j])
						players[i].score++;
					else
						break;
				}
			}
		}

		delta = GetTime() - prev_time;
		prev_time = GetTime();
		y_off += delta * vel;
		vel += delta / 100.f;
		if(IsKeyDown(KEY_X))
			players[0].score++;

		BeginDrawing();
		ClearBackground(WHITE);
		DrawRectangle(100, 100, GetScreenWidth() - 200, GetScreenHeight() - 200, GRAY);
		
		for(int i = 0; i < PLAYER_COUNT; i++) {
			if(ma_client_active(i)) {
				char score[5];
				sprintf(score, "%i", players[i].score);
				DrawTextEx(roboto_font, score, (Vector2) { i * 150 + 150, 0 }, 15, 0.f, BLACK);

				float x = 100;
				float y = 100 - y_off + i * (40.f / ma_client_active_count());
				float line_length = 0;

				for(int j = 0; j < players[i].score && i < text_length; j++) {
					float char_length = dist_table[current_text[j]];
					line_length += char_length;

					if(line_length > GetScreenWidth() - 200 || current_text[j] == '\n') {
						line_length = 0;
						y += 60;
						x = 100;
					}
					else {
						DrawRectangle(x, y, char_length, (40.f / ma_client_active_count()), players[i].col);
						x += char_length;
					}
				}

				if(y - 100 > y_off)
					y_off = y - 100;
			}
		}

		DrawTextRec(roboto_font, current_text, (Rectangle) { 100, 100 - y_off, GetScreenWidth() - 200, GetScreenHeight() - 200 + y_off }, 40, 0.f, 0, BLACK);

		DrawRectangle(0, 0, GetScreenWidth(), 100, WHITE);
		EndDrawing();
	}

	UnloadFont(roboto_font);
	CloseWindow();
	ma_frame_destroy(main_frame);
	ma_frame_destroy(join_frame);
	ma_free();
	return 0;
}
