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
#define BULLET_COUNT 100
#define ASTEROID_COUNT 40

// RULES
#define RESPAWN

typedef struct {
	bool enabled;
	float x;
	float y;
	float vel_x;
	float vel_y;
	float speed;
	float rad;
	float rot;
	float torque;
} EntityHeader;

typedef struct {
	EntityHeader entity;
	bool boosting;
	bool shooting;

	unsigned int thicc;
	byte health;
	Color col;
} Player;


typedef struct {
	EntityHeader entity;

	int start_index;
	int length;
} Asteroid;

typedef struct {
	EntityHeader entity;

	Color col;
} Bullet;


const Vector2 random_points[] = {
	(Vector2) { -0.06, -.5f },
	(Vector2) {  1.f,  .5f },
	(Vector2) {  1.f, -1.f },
	(Vector2) { -1.f, -1.f },
	(Vector2) {  .5f, -.5f },
};

Player players[PLAYER_COUNT];
Bullet bullets[BULLET_COUNT];
Asteroid asteroids[ASTEROID_COUNT];

unsigned int bullet_current = 0;
unsigned int asteroid_current = 0;

int width  = 1280;
int height = 720;

void reset_bullet(Bullet* bullet) {
	bullet->entity.enabled = 0;
	bullet->entity.x = -1000;
	bullet->entity.y = -1000;
	bullet->entity.vel_x = 0;
	bullet->entity.vel_y = 0;
	bullet->entity.speed = 280.f;
	bullet->entity.rad = 4;
	bullet->col = WHITE;
}

void reset_asteroid(Asteroid* asteroid) {
	asteroid->entity.enabled = 0;
	asteroid->entity.x = -1000;
	asteroid->entity.y = -1000;
	asteroid->entity.vel_x = 0;
	asteroid->entity.vel_y = 0;
	asteroid->entity.rot = cos((rand() % 1000) / 200.f);
	asteroid->entity.torque = (((rand() % 1000) / 1000.f) - .5f) * 100.f;

	asteroid->start_index = rand() % 32;
	asteroid->length = rand() % 4 + 5;
	asteroid->entity.rad = rand() % 5 * 20.f;
}

void spawn_asteroid(float x, float y, float vel_x, float vel_y, float size, float speed) {
	asteroids[asteroid_current].entity.enabled = 1;
	asteroids[asteroid_current].entity.x = x;
	asteroids[asteroid_current].entity.y = y;
	asteroids[asteroid_current].entity.vel_x = vel_x;
	asteroids[asteroid_current].entity.vel_y = vel_y;
	asteroids[asteroid_current].entity.rad = size;
	asteroids[asteroid_current].entity.speed = speed;
	asteroids[asteroid_current].entity.torque *= -1;

	asteroid_current++;
}

void reset() {

	// asteroid shapes

	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].entity.x = width  / 2.f;
		players[i].entity.y = height / 2.f;

		players[i].thicc = 1;
		players[i].col = (Color) { rand()%255, rand()%255, rand()%255, 255 };

		float angle = (rand() % 1000) / 200.f;
		players[i].entity.vel_x = cos(angle);
		players[i].entity.vel_y = sin(angle);
		players[i].entity.rot = angle;
		players[i].entity.speed = 24.f;
		players[i].health = 5;
	}

	for(int i = 0; i < BULLET_COUNT; i++) {
		reset_bullet(&bullets[i]);
	}

	for(int i = 0; i < ASTEROID_COUNT; i++) {
		reset_asteroid(&asteroids[i]);
	}
}

void do_physics(EntityHeader* entity, float delta) {
	entity->x += entity->vel_x * entity->speed * delta;
	entity->y += entity->vel_y * entity->speed * delta;

	entity->rot += entity->torque * delta;

	if(entity->x < 0)
		entity->x = width;
	else if(entity->x > width)
		entity->x = 0;

	if(entity->y < 0)
		entity->y = height;
	else if(entity->y > height)
		entity->y = 0;
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

	InitWindow(width, height, "testapp");
	
	reset();

	float delta_time = 0;
	float prev_time = 0;

	bool requesting = false;
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);

	for(int i = 0; i < 5; i++) {
		float angle = rand() % 1000 / 300.f;
		spawn_asteroid(rand() % width,  rand() % height, cos(angle), sin(angle), rand() % 100, 20.f);
	}


	while(!WindowShouldClose()) {
		delta_time = GetTime() - prev_time;
		prev_time = GetTime();

		width  = GetScreenWidth();
		height = GetScreenHeight();

		if(IsKeyPressed(KEY_R))
			reset();

		if(IsKeyDown(KEY_Z))
			requesting = !requesting;

		// update physics
		for(int i = 0; i < BULLET_COUNT; i++) {
			if(bullets[i].entity.enabled)
				if(bullets[i].entity.x != -1000) {
					do_physics((EntityHeader*) &bullets[i], delta_time);
					float x1 = bullets[i].entity.x;
					float y1 = bullets[i].entity.y;

					// if a bullet touches a bullet then they both disappear
					for(int j = 0; j < BULLET_COUNT; j++) {
						if(i != j) {
							float x2 = bullets[j].entity.x;
							float y2 = bullets[j].entity.y;

							if( (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < 16) {
								reset_bullet(&bullets[i]);
								reset_bullet(&bullets[j]);
							}
						}
					}

					for(int j = 0; j < ASTEROID_COUNT; j++) {
						if(asteroids[j].entity.enabled) {
							float x2 = asteroids[j].entity.x;
							float y2 = asteroids[j].entity.y;

							if( (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < asteroids[j].entity.rad * asteroids[j].entity.rad + 32) {
								float angle = rand() % 1000 / 300.f;

								if(asteroids[j].entity.rad / 2.f > 10.f) {
									spawn_asteroid(x2, y2, cos(angle), sin(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
									spawn_asteroid(x2, y2, sin(angle), cos(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
								}

								reset_bullet(&bullets[i]);
								reset_asteroid(&asteroids[j]);
							}
						}
					}
				}
		}
		for(int i = 0; i < ASTEROID_COUNT; i++) {
			if(asteroids[i].entity.enabled)
				do_physics((EntityHeader*) &asteroids[i], delta_time);
		}
		for(int i = 0; i < PLAYER_COUNT; i++) {
			if(players[i].entity.enabled)
				do_physics((EntityHeader*) &players[i], delta_time);
		}

		for(int i = 0; i < PLAYER_COUNT; i++) {
			players[i].entity.enabled = ma_client_active(i);
			if(ma_client_active(i) && players[i].health) {

				bool b1 = 0;
				ma_client_input_button_get(i, 0, (unsigned char*) &b1);

				float dir_x = cos(players[i].entity.rot - PI / 2.f);
				float dir_y = sin(players[i].entity.rot - PI / 2.f);

				// movement
				if(b1) {
					players[i].entity.vel_x += dir_x * 4.f * delta_time;
					players[i].entity.vel_y += dir_y * 4.f * delta_time;
				}

				float j1_x;
				float j1_y;
				if(ma_client_input_joystick_get(i, 0, &j1_x, &j1_y)) {
					if( (j1_x != 0 || j1_y != 0) &&
						(j1_x != -players[i].entity.vel_x || j1_y != -players[i].entity.vel_y)) {
						players[i].entity.rot = atan2(j1_x, j1_y);
					}
				}

				// shooting
				bool b2 = 0;
				ma_client_input_button_get(i, 1, (unsigned char*) & b2);
				if(b2 && !players[i].shooting) {
					bullets[bullet_current].entity.x = players[i].entity.x + dir_x * 42.f;
					bullets[bullet_current].entity.y = players[i].entity.y + dir_y * 42.f;
					bullets[bullet_current].entity.vel_x = dir_x;
					bullets[bullet_current].entity.vel_y = dir_y;
					bullets[bullet_current].entity.enabled = 1;
					bullets[bullet_current].col = players[i].col;

					players[i].shooting = 1;
					bullet_current = (bullet_current + 1) % BULLET_COUNT;
				}
				else if(!b2)
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

		float offsets[5 * 2] = {
			 0, 0,
			 width, 0,
			 0,  height,
			-width, 0,
			 0, -height,
		};

		// draw ships
		for(int i = 0; i < PLAYER_COUNT; i++)
			if(ma_client_active(i)) {
				float angle = players[i].entity.rot;
				float angle_cos = cos(angle + PI);
				float angle_sin = sin(angle + PI);


				// very intuitive matrix maths
				for(int j = 0; j < 5; j++) {
					float x = players[i].entity.x + offsets[j * 2 + 0];
					float y = players[i].entity.y + offsets[j * 2 + 1];

					DrawTriangleLines((Vector2) { x - (10.f * angle_cos), y - (10.f * angle_sin) },
									  (Vector2) { x - (30.f * angle_sin), y + (30.f * angle_cos) },
									  (Vector2) { x + (10.f * angle_cos), y + (10.f * angle_sin) },
									  players[i].col);

					if(players[i].boosting) {
					}
				}
			}

		for(int i = 0; i < BULLET_COUNT; i++) {
			DrawRectangle(bullets[i].entity.x - 2, bullets[i].entity.y - 2, 4, 4, bullets[i].col);
		}

		for(int i = 0; i < ASTEROID_COUNT; i++) {
			int line_count = asteroids[i].length + 1;
			Vector2 lines[line_count];

			for(int j = 0; j < asteroids[i].length; j++) {
				lines[j].x = random_points[j + asteroids[i].start_index].x * asteroids[i].entity.rad + 400;
				lines[j].y = random_points[j + asteroids[i].start_index].y * asteroids[i].entity.rad + 400;
			}

			lines[line_count - 1] = lines[0];

//			DrawLineStrip(lines, line_count, WHITE);

			for(int j = 0; j < 5; j++) {
				float x = asteroids[i].entity.x + offsets[j * 2 + 0];
				float y = asteroids[i].entity.y + offsets[j * 2 + 1];

				DrawPolyLines((Vector2) { x, y }, asteroids[i].length, asteroids[i].entity.rad, asteroids[i].entity.rot, WHITE);
			}
		}

		EndDrawing();
	}

	CloseWindow();
	ma_frame_destroy(main_frame);
	ma_free();
	h_debug_log_history();
	h_debug_log_free();
}
