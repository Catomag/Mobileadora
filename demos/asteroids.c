// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <raylib.h>
#include <math.h>

#define _H_DEBUG_MEMORY_
#include <Gaia/Hephaestus.h>

#define PLAYER_COUNT 60
#define WIDTH 1248
#define HEIGHT 720

#define BULLET_COUNT 100

// RULES
#define RESPAWN

typedef struct {
	float x;
	float y;
	float vel_x;
	float vel_y;
	float rot;
	bool boosting;
	bool shooting;

	unsigned int thicc;
	byte health;
	Color col;
} Player;

typedef struct {
	float x;
	float y;
	float rad;

	unsigned int line_count;
	float* lines;
} Asteroid;

typedef struct {
	float x;
	float y;
	float vel_x;
	float vel_y;

	Color col;
} Bullet;

Player players[PLAYER_COUNT];
Bullet bullets[BULLET_COUNT];
unsigned int bullet_current = 0;

void reset() {
	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].x = WIDTH / 2;
		players[i].y = HEIGHT / 2;

		players[i].thicc = 1;
		players[i].col = (Color) { rand()%255, rand()%255, rand()%255, 255 };

		float angle = (rand() % 1000) / 200.f;
		players[i].vel_x = cos(angle);
		players[i].vel_y = sin(angle);
		players[i].rot = angle;
		players[i].health = 5;
	}

	for(int i = 0; i < BULLET_COUNT; i++) {
		bullets[i].x = -1000;
		bullets[i].y = -1000;
		bullets[i].vel_x = 0;
		bullets[i].vel_y = 0;
		bullets[i].col = WHITE;
	}
}

// Simple form
int main() {

	ma_init(PLAYER_COUNT, 8000);

	Frame* main_frame = ma_frame_create(FRAME_DYNAMIC, ORIENTATION_HORIZONTAL, false, false);
	ma_frame_element_color_add(main_frame, 10, 10, 10);
	ma_frame_element_color_add(main_frame, 10, 16, 10);
	ma_frame_element_color_add(main_frame, 10, 255, 9);
	ma_frame_element_break_add(main_frame);
	ma_frame_element_h1_add(main_frame, "HEader");
	float test = 0;

	ma_frame_input_joystick_add(main_frame);
	for(int i = 0; i < 2; i++)
		ma_frame_input_button_add(main_frame);

	// do stuff with data
	ma_frame_print(main_frame);
	ma_frame_default(main_frame);

	InitWindow(WIDTH, HEIGHT, "testapp");
	
	reset();

	float delta_time = 0;
	float prev_time = 0;

	bool requesting = false;
	SetTargetFPS(60);


	while(!WindowShouldClose()) {
		delta_time = GetTime() - prev_time;
		prev_time = GetTime();


		if(IsKeyPressed(KEY_R))
			reset();

		if(IsKeyDown(KEY_Z))
			requesting = !requesting;

		// update bullets
		for(int i = 0; i < BULLET_COUNT; i++) {
			bullets[i].x += bullets[i].vel_x * delta_time * 120.f;
			bullets[i].y += bullets[i].vel_y * delta_time * 120.f;
		}

		for(int i = 0; i < PLAYER_COUNT; i++) {
			if(ma_client_active(i) && players[i].health) {

				bool b1 = 0;
				ma_client_input_button_get(i, 0, (unsigned char*) &b1);

				float dir_x = cos(players[i].rot - PI / 2.f);
				float dir_y = sin(players[i].rot - PI / 2.f);

				// movement
				if(b1) {
					players[i].vel_x += dir_x * 2.f * delta_time;
					players[i].vel_y -= dir_y * 2.f * delta_time;
				}

				float j1_x;
				float j1_y;
				if(ma_client_input_joystick_get(i, 0, &j1_x, &j1_y)) {
					if( (j1_x != 0 || j1_y != 0) &&
						(j1_x != -players[i].vel_x || j1_y != -players[i].vel_y)) {
						players[i].rot = atan2(j1_x, j1_y);
					}


					players[i].x += players[i].vel_x;
					players[i].y -= players[i].vel_y;


					if(players[i].x < 0)
						players[i].x = WIDTH;
					else if(players[i].x > WIDTH)
						players[i].x = 0;

					if(players[i].y < 0)
						players[i].y = HEIGHT;
					else if(players[i].y > HEIGHT)
						players[i].y = 0;
				}

				// shooting
				bool b2 = 0;
				ma_client_input_button_get(i, 1, (unsigned char*) & b2);
				if(b2 && !players[i].shooting) {
					bullets[bullet_current].x = players[i].x + dir_x * 42.f;
					bullets[bullet_current].y = players[i].y + dir_y * 42.f;
					bullets[bullet_current].vel_x = dir_x;
					bullets[bullet_current].vel_y = dir_y;
					bullets[bullet_current].col = players[i].col;

					players[i].shooting = 1;
					bullet_current = (bullet_current + 1) % BULLET_COUNT;
				}
				else
					players[i].shooting = 0;

				// player collisions

				for(int j = 0; j < PLAYER_COUNT; j++) {
#ifdef WRAP_SELF
					if(ma_client_active(j) && j != i) {
#endif
						if(ma_client_active(j)) {
						}
					}
#ifdef WRAP_SELF
				}
#endif
			}
#ifdef RESPAWN
			else if(ma_client_active(i)) {
				players[i].thicc = 3;
				players[i].health = 3;
			}
#endif
		}

		ClearBackground(BLACK);
		BeginDrawing();


		// draw ships
		for(int i = 0; i < PLAYER_COUNT; i++)
			if(ma_client_active(i)) {
				float x = players[i].x;
				float y = players[i].y;
				float angle = players[i].rot;
				float angle_cos = cos(angle + PI);
				float angle_sin = sin(angle + PI);

				// very intuitive matrix maths
				DrawTriangleLines((Vector2) { x - (10.f * angle_cos), y - (10.f * angle_sin) },
							 	  (Vector2) { x - (40.f * angle_sin), y + (40.f * angle_cos) },
							 	  (Vector2) { x + (10.f * angle_cos), y + (10.f * angle_sin) },
							 	  players[i].col);
			}

		for(int i = 0; i < BULLET_COUNT; i++) {
			DrawRectangle(bullets[i].x - 2, bullets[i].y - 2, 4, 4, bullets[i].col);
		}

		EndDrawing();
	}

	CloseWindow();
	ma_frame_destroy(main_frame);
	ma_free();
	h_debug_log_history();
	h_debug_log_free();
}
