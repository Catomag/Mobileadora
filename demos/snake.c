// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <raylib.h>
#include <math.h>

#define _H_DEBUG_MEMORY_
#include <Gaia/Hephaestus.h>

#define PLAYER_COUNT 60
#define WIDTH 1248
#define HEIGHT 720

// RULES
//#define WALLS
//#define WRAP_SELF
#define RESPAWN

#define CELL_SIZE 8 

#define BOARD_WIDTH  WIDTH / CELL_SIZE
#define BOARD_HEIGHT HEIGHT / CELL_SIZE

#define DEFAULT_SPEED 5
#define BOOST_SPEED 2

#define PELLET_COUNT 160
#define MAX_LENGTH 300

typedef struct {
	short x[MAX_LENGTH];
	short y[MAX_LENGTH];
	short dir_x;
	short dir_y;

	unsigned int thicc;
	bool sped;
	bool alive;
	Color color;
	Frame* frame;
} Player;

typedef struct {
	short x;
	short y;
} Pellet;

Frame* base_frame;
Player players[PLAYER_COUNT];
Pellet pellets[PELLET_COUNT];

void player_increase_length(Player* player, int length) {
	for(int i = 0; i < length; i++) {
		if(player->thicc < MAX_LENGTH) {
			player->x[player->thicc] = (player->x[player->thicc - 1]) - player->dir_x; 
			player->y[player->thicc] = (player->y[player->thicc - 1]) + player->dir_y;

			player->thicc += 1;
		}
	}
}

void reset() {
	for(int i = 0; i < PELLET_COUNT; i++) {
		pellets[i].x = rand() % BOARD_WIDTH;
		pellets[i].y = rand() % BOARD_HEIGHT;
	}

	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].x[0] = rand() % BOARD_WIDTH;
		players[i].y[0] = rand() % BOARD_HEIGHT;

		players[i].thicc = 1;
		players[i].color = (Color) { rand()%255, rand()%255, rand()%255, 180 };
		players[i].dir_x = 0;
		players[i].dir_y = -1;
		players[i].alive = 1;
		player_increase_length(&players[i], 8);

		if(players[i].frame == NULL)
			players[i].frame = ma_frame_copy(base_frame);

		ma_frame_element_color_set(players[i].frame, 0, players[i].color.r, players[i].color.g, players[i].color.b);
		ma_frame_send(players[i].frame, i);
	}
}

// Simple form
int main() {

	ma_init(PLAYER_COUNT, 8000);

	base_frame = ma_frame_create(FRAME_DYNAMIC, ORIENTATION_HORIZONTAL, false, false);
	ma_frame_element_color_add(base_frame, 0, 255, 0);

//	ma_frame_element_break_add(main_frame);
//	ma_frame_element_h1_add(main_frame, "HEader");

	ma_frame_input_text_add(base_frame, 100);
	ma_frame_input_joystick_add(base_frame);
	for(int i = 0; i < 2; i++)
		ma_frame_input_button_add(base_frame);
	ma_frame_input_toggle_add(base_frame);
	ma_frame_input_submit_add(base_frame);

//	Element br;
//	br.type = ELEMENT_BREAK;
//	br.size = 0;
//
//	// this is how data has to be included in element
//	Element* text = malloc(sizeof(Element) + 5);
//	br.type = ELEMENT_TEXT;
//	br.size = 5;
//	br.data[0] = 'h';
//	br.data[1] = 'e';
//	br.data[2] = 'l';
//	br.data[3] = 'l';
//	br.data[4] = 'o';
//
//	ma_frame_element_add(main_frame, *text);
//	free(text);
//	ma_frame_element_add(main_frame, br);
//	ma_frame_element_add(main_frame, br);
	
	// do stuff with data
	ma_frame_print(base_frame);
	ma_frame_default(base_frame);

	InitWindow(WIDTH, HEIGHT, "testapp");
	
	bzero(players, PLAYER_COUNT * sizeof(Player));
	reset();

	float time = 0;
	float grow_time = 0;
	float prev_time = 0;
	unsigned long tick = 0;

	SetTargetFPS(60);


	while(!WindowShouldClose()) {
		time += GetTime() - prev_time;
		grow_time += GetTime() - prev_time;
		prev_time = GetTime();

		if(IsKeyPressed(KEY_R))
			reset();

		if(time > .008f) {
			tick++;
			time = 0;

			for(int i = 0; i < PLAYER_COUNT; i++) {
				if(ma_client_active(i) && players[i].alive) {

					char buf[100];
					if(ma_client_input_text_get(i, 0, buf)) {
						printf("%s\n", buf);
					}

					bool b1 = 0;
					ma_client_input_button_get(i, 0, (unsigned char*) &b1);
					if(b1) {
						if(players[i].color.a >= 200 && players[i].sped == 0)
							players[i].sped = 1;
					}
					else
						players[i].sped = 0;

					bool b2 = 0;
					ma_client_input_button_get(i, 1, (unsigned char*) & b2);
					if(b2) {
						player_increase_length(&players[i], 1);
					}

					if(players[i].sped)
						players[i].color.a -= 2;

					if(!players[i].sped && players[i].color.a < 255 && !b1)
						players[i].color.a++;

					if(players[i].color.a <= 140)
						players[i].sped = 0;

					// movement
					float j1_x;
					float j1_y;
					if( ma_client_input_joystick_get(i, 0, &j1_x, &j1_y) && 
						((tick % DEFAULT_SPEED == 0 && !players[i].sped) || (tick % BOOST_SPEED == 0 && players[i].sped))) {

						if(j1_x > .5f)
							j1_x = 1;
						else if(j1_x < -.5f)
							j1_x = -1;
						else
							j1_x = 0;

						if(j1_y > .5f)
							j1_y = 1;
						else if(j1_y < -.5f)
							j1_y = -1;
						else
							j1_y = 0;

						if( (j1_x != 0 || j1_y != 0) &&
							(j1_x != -players[i].dir_x || j1_y != -players[i].dir_y)) {
							players[i].dir_x = j1_x;
							players[i].dir_y = j1_y;
						}

						for(int j = players[i].thicc; j > 0; j--) {
							players[i].x[j] = players[i].x[j - 1];
							players[i].y[j] = players[i].y[j - 1];
						}

						players[i].x[0] += players[i].dir_x;
						players[i].y[0] -= players[i].dir_y;

#ifndef WALLS
						if(players[i].x[0] < 0)
							players[i].x[0] = BOARD_WIDTH;
						else if(players[i].x[0] > BOARD_WIDTH)
							players[i].x[0] = 0;

						if(players[i].y[0] < 0)
							players[i].y[0] = BOARD_HEIGHT;
						else if(players[i].y[0] > BOARD_HEIGHT)
							players[i].y[0] = 0;
#else
						if(players[i].x[0] < 0)
							players[i].alive = 0;
						else if(players[i].x[0] > BOARD_WIDTH)
							players[i].alive = 0;

						if(players[i].y[0] < 0)
							players[i].alive = 0;
						else if(players[i].y[0] > BOARD_HEIGHT)
							players[i].alive = 0;
#endif
					}


					// eat pelet
					for(int j = 0; j < PELLET_COUNT; j++) {
						if( players[i].x[0] == pellets[j].x &&
							players[i].y[0] == pellets[j].y) {

							pellets[j].x = rand() % BOARD_WIDTH;
							pellets[j].y = rand() % BOARD_HEIGHT;
							player_increase_length(&players[i], 1);
						}
					}

					// player collisions

					for(int j = 0; j < PLAYER_COUNT; j++) {
#ifdef WRAP_SELF
						if(ma_client_active(j) && j != i) {
#endif
							if(ma_client_active(j)) {
								for(unsigned int k = 1; k < players[j].thicc; k++) {
									if( players[i].x[0] == players[j].x[k] &&
										players[i].y[0] == players[j].y[k] && players[j].color.a > 100) {
										players[i].alive = 0;
									}
								}
							}
						}
#ifdef WRAP_SELF
					}
#endif
				}
#ifdef RESPAWN
				else if(ma_client_active(i)) {
					players[i].thicc = 3;
					players[i].alive = 1;
					players[i].color.a = 20;
				}
#endif
			}
		}

		ClearBackground(BLACK);
		BeginDrawing();

		// draw board
		for(int x = 0; x < BOARD_WIDTH; x++)
			for(int y = 0; y < BOARD_HEIGHT; y++)
				DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, (((y * BOARD_WIDTH + x) + y) % 2 == 0) ? BLACK : (Color) { 20, 20, 20, 255 });

		// draw pellets
		float glow[4] = {
			(sin(GetTime() + 0.f) + 1.5f) / 2.5f,
			(sin(GetTime() + 2.f) + 1.5f) / 2.5f,
			(sin(GetTime() + 4.f) + 1.5f) / 2.5f,
			(sin(GetTime() + 8.f) + 1.5f) / 2.5f,
		};

		for(int i = 0; i < PELLET_COUNT; i++)
			DrawRectangle(pellets[i].x * CELL_SIZE, pellets[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE, (Color) { 255, 0, 0, 255 * glow[i % 4] });

		// draw snaks
		for(int i = 0; i < PLAYER_COUNT; i++)
			if(ma_client_active(i)) {
				for(int j = players[i].thicc - 1; j >= 0; j--)
					DrawRectangle(((short) players[i].x[j]) * CELL_SIZE, ((short)players[i].y[j]) * CELL_SIZE, CELL_SIZE, CELL_SIZE, players[i].color);
			}

		EndDrawing();
	}

	CloseWindow();
	ma_free();
	for(int i = 0; i < PLAYER_COUNT; i++)
		if(players[i].frame != NULL)
			ma_frame_destroy(players[i].frame);
	ma_frame_destroy(base_frame);
	h_debug_log_history();
	h_debug_log_free();
}
