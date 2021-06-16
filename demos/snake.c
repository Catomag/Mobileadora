// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>

#include <raylib.h>

#include "../include/mobileadora.h"

#define PLAYER_COUNT 60
#define WIDTH 1248
#define HEIGHT 720

// RULES
//#define WALLS
//#define WRAP_SELF
#define RESPAWN
#define SHAKE
//#define CLASSIC

#define EXPLOSION_COUNT 30
#define CELL_SIZE 8 

#define BOARD_WIDTH  WIDTH / CELL_SIZE
#define BOARD_HEIGHT HEIGHT / CELL_SIZE

#define DEFAULT_SPEED 5
#define BOOST_SPEED 2

#define PELLET_COUNT 160
#define MAX_LENGTH 300

typedef struct {
	float x[MAX_LENGTH];
	float y[MAX_LENGTH];
	float dir_x;
	float dir_y;

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

typedef struct {
	int frame;
	float x;
	float y;
} Explosion;
//
// hopefully a nice selection of neon colours (generated with science and logic)
const int colors[] = {
	0x44ff88ff,
	0xffff88ff,
	0xff4444ff,
	0xfd0000ff,
	0xff0000ff,
	0xff0d88ff,
	0x44ff44ff,
	0xffd488ff,
	0xffff88ff,
	0xfffd00ff,
	0x00ffffff,
	0x00ff88ff,
	0x44ffdfff,
	0x00ff00ff,
	0xffdf44ff,
	0x4dff88ff,
	0x00ff88ff,
	0xff4444ff,
};

Frame* base_frame;
Player players[PLAYER_COUNT];
Pellet pellets[PELLET_COUNT];

// vfx
Explosion explosions[EXPLOSION_COUNT];
Camera2D camera = { 0 };
bool camera_shaking = 0;
float shake_duration = 0;
unsigned int explosion_current = 0;

void screen_shake(float duration) {
	camera_shaking = 1;
	shake_duration = duration;
}

void spawn_explosion(float x, float y) {
	explosions[explosion_current].x = x;
	explosions[explosion_current].y = y;
	explosions[explosion_current].frame = 0;
	explosion_current = (explosion_current + 1) % EXPLOSION_COUNT;
}

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
		if(i < 17)
			players[i].color = GetColor(colors[i]);
		else
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

	ma_frame_input_joystick_add(base_frame);
	ma_frame_element_spacer_add(base_frame);
	ma_frame_input_button_add(base_frame);
	
	// do stuff with data
	ma_frame_print(base_frame);
	ma_frame_default(base_frame);

	InitWindow(WIDTH, HEIGHT, "testapp");
	InitAudioDevice();
	Texture explosion_texture = LoadTexture("demos/explosion.png");
	Sound num_sound = LoadSound("demos/num.wav");
	Sound explosion_sound = LoadSound("demos/explosion.wav");

	for(int i = 0; i < EXPLOSION_COUNT; i++)
		explosions[i].frame = 16;
	
	bzero(players, PLAYER_COUNT * sizeof(Player));
	reset();

	camera.zoom = 1;
	camera.rotation = 0;

	float time = 0;
	float grow_time = 0;
	float prev_time = 0;
	float delta = 0;
	float explosion_frame_time = 0;
	unsigned long tick = 0;

	SetTargetFPS(60);

	while(!WindowShouldClose()) {
		time = GetTime();
		grow_time += GetTime() - prev_time;
		delta = time - prev_time;
		prev_time = time;

		if(IsKeyPressed(KEY_R))
			reset();

		if(time > .008f) {
			tick++;
			time = 0;

			for(int i = 0; i < PLAYER_COUNT; i++) {
				if(ma_client_active(i) && players[i].alive) {
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
#ifdef CLASSIC
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
#endif

						if( ((int)(j1_x * 10.f) != 0 || (int)(j1_y * 10.f) != 0) &&
							((int)(j1_x * 10.f) != -(int)(players[i].dir_x * 10.f) || (int)(j1_y * 10.f) != -(int)(players[i].dir_y * 10.f))) {
							players[i].dir_x = j1_x / sqrt(j1_x * j1_x + j1_y * j1_y);
							players[i].dir_y = j1_y / sqrt(j1_x * j1_x + j1_y * j1_y);
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
						int was_alive = players[i].alive;
						if(players[i].x[0] < 0)
							players[i].alive = 0;
						else if(players[i].x[0] > BOARD_WIDTH)
							players[i].alive = 0;

						if(players[i].y[0] < 0)
							players[i].alive = 0;
						else if(players[i].y[0] > BOARD_HEIGHT)
							players[i].alive = 0;

						if(was_alive != players[i].alive) {
							spawn_explosion(players[i].x[0] * CELL_SIZE, players[i].y[0] * CELL_SIZE);
							screen_shake(.8f);
							PlaySoundMulti(explosion_sound);
						}
#endif
					}


					// eat pelet
					for(int j = 0; j < PELLET_COUNT; j++) {
						if( (int) players[i].x[0] == (int) pellets[j].x &&
							(int) players[i].y[0] == (int) pellets[j].y) {

							pellets[j].x = rand() % BOARD_WIDTH;
							pellets[j].y = rand() % BOARD_HEIGHT;
							player_increase_length(&players[i], 5);
							PlaySoundMulti(num_sound);
						}
					}

					// player collisions

					for(int j = 0; j < PLAYER_COUNT; j++) {
#ifdef WRAP_SELF
						if(ma_client_active(j) && j != i) {
#endif
							if(ma_client_active(j)) {
								for(unsigned int k = 2; k < players[j].thicc; k++) {
									if( (int) players[i].x[0] == (int) players[j].x[k] &&
										(int) players[i].y[0] == (int) players[j].y[k] && players[j].color.a > 100) {
										players[i].alive = 0;
										spawn_explosion(players[i].x[0] * CELL_SIZE, players[i].y[0] * CELL_SIZE);
										screen_shake(.8f);
										PlaySoundMulti(explosion_sound);
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
					players[i].x[0] = rand() % WIDTH;
					players[i].y[0] = rand() % HEIGHT;
				}
#endif
			}
		}

		// screen shake
		if(shake_duration > 0) {
			camera.offset.x += (rand() % 20 - 10);
			camera.offset.y += (rand() % 20 - 10);
			shake_duration -= delta;
		}
		else {
			camera.offset.x = 0;
			camera.offset.y = 0;
		}

		// update explosions
		if(explosion_frame_time < GetTime()) {
			explosion_frame_time = GetTime() + .06f;
			for(int i = 0; i < EXPLOSION_COUNT; i++) {
				if(explosions[i].frame < 16)
					explosions[i].frame++;
			}
		}

		ClearBackground(BLACK);
		BeginDrawing();
		BeginMode2D(camera);

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

		// draw splosions
		for(int i = 0; i < EXPLOSION_COUNT; i++) {
			int frame = explosions[i].frame;
			if(frame < 16) {
				Rectangle source;
				source.x = (frame % 4) * 64;
				source.y = (frame / 4) * 64;
				source.width = 64;
				source.height = 64;
				DrawTextureRec(explosion_texture, source, (Vector2) { explosions[i].x - 32, explosions[i].y - 32 }, WHITE);			
			}
		}

		EndMode2D();
		EndDrawing();
	}

	UnloadSound(num_sound);
	UnloadSound(explosion_sound);
	UnloadTexture(explosion_texture);
	CloseAudioDevice();
	CloseWindow();
	ma_free();
	for(int i = 0; i < PLAYER_COUNT; i++)
		if(players[i].frame != NULL)
			ma_frame_destroy(players[i].frame);
	ma_frame_destroy(base_frame);
}
