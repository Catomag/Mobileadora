// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include <raylib.h>

#include "../include/mobileadora.h"

#define PLAYER_COUNT 60
#define BULLET_COUNT 100
#define ASTEROID_COUNT 40
#define EXPLOSION_COUNT 40
#define EXPLOSION_FRAMES 16

// RULES
#define RESPAWN
//#define LASER

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
	bool joined;
	bool first;
	unsigned char health;

	unsigned int thicc;
	char name[5];
	Color col;
	float last_shot;
	Frame* frame;
	int score;
} Player;


typedef struct {
	EntityHeader entity;

	int start_index;
	int length;
} Asteroid;

typedef struct {
	EntityHeader entity;

	int source;
} Bullet;

typedef struct {
	int frame;
	float x;
	float y;
} Explosion;

// hopefully a nice selection of neon colours (generated with science and logic)
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
Explosion explosions[EXPLOSION_COUNT];
Camera2D camera = { 0 };
bool camera_shaking = 0;
float shake_duration = 0;

unsigned int bullet_current = 0;
unsigned int asteroid_current = 0;
unsigned int explosion_current = 0;

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

	asteroid_current = (asteroid_current + 1) % ASTEROID_COUNT;
}

void spawn_explosion(float x, float y) {
	explosions[explosion_current].x = x;
	explosions[explosion_current].y = y;
	explosions[explosion_current].frame = 0;
	explosion_current = (explosion_current + 1) % EXPLOSION_COUNT;
}

void generate_asteroid() {
	float angle = rand() % 1000 / 300.f;
	float size = rand() % 100;
	float speed = (100 / size) * 10;
	switch(rand() % 4) {
		case 0:
			spawn_asteroid(rand() % width, 0, cos(angle), sin(angle), size, speed);
		break;
		case 1:
			spawn_asteroid(rand() % width, height, cos(angle), sin(angle), size, speed);
		break;
		case 2:
			spawn_asteroid(0, rand() % height, cos(angle), sin(angle), size, speed);
		break;
		case 3:
			spawn_asteroid(width, rand() % height, cos(angle), sin(angle), size, speed);
		break;
	}
}

void shoot(int player_index, float dir_x, float dir_y, float vel) {
	int i = player_index;
	bullets[bullet_current].entity.x = players[i].entity.x + dir_x * 30.f;
	bullets[bullet_current].entity.y = players[i].entity.y + dir_y * 30.f;
	bullets[bullet_current].entity.vel_x = dir_x;
	bullets[bullet_current].entity.vel_y = dir_y;
	bullets[bullet_current].entity.enabled = 1;
	bullets[bullet_current].source = i;
	bullet_current = (bullet_current + 1) % BULLET_COUNT;
}

void reset() {

	// asteroid shapes

	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].entity.x = rand() % width;
		players[i].entity.y = rand() % height;

		players[i].thicc = 1;
		if(i < 18) {
			players[i].col = GetColor(colors[i]);
			players[i].col.a = 255;
		}
		else
			players[i].col = (Color) { rand()%255, rand()%255, rand()%255, 255 };

		float angle = (rand() % 1000) / 200.f;
		players[i].entity.vel_x = cos(angle);
		players[i].entity.vel_y = sin(angle);
		players[i].entity.rot = angle;
		players[i].entity.rad = 10;
		players[i].entity.speed = 24.f;
		players[i].health = 3;
		players[i].score = 0;
		players[i].first = 1;

		ma_frame_element_color_set(players[i].frame, 0, players[i].col.r, players[i].col.g, players[i].col.b);
		if(players[i].joined)
			ma_frame_send(players[i].frame, i);
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

bool entities_collide(EntityHeader* a, EntityHeader* b) {
	float rad = a->rad;
	if(rad < b->rad)
		rad = b->rad;

	float x1 = a->x;
	float y1 = a->y;
	float x2 = b->x;
	float y2 = b->y;

	return ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < rad * rad) && (a->enabled) && (b->enabled); 
}

void screen_shake(float duration) {
	camera_shaking = 1;
	shake_duration = duration;
}


// Simple form
int main() {
	ma_init(PLAYER_COUNT, 8000);

	Frame* join_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, false, false);
	ma_frame_element_text_add(join_frame, "Write your name here pls");
	ma_frame_input_text_add(join_frame, 5);
	ma_frame_input_submit_add(join_frame);
	Frame* main_frame = ma_frame_create(FRAME_DYNAMIC, ORIENTATION_HORIZONTAL, false, false);
	float test = 0;

	ma_frame_input_joystick_add(main_frame);
	ma_frame_element_spacer_add(main_frame);
	for(int i = 0; i < 2; i++)
		ma_frame_input_button_add(main_frame);

	// do stuff with data
	ma_frame_default(join_frame);

	for(int i = 0; i < PLAYER_COUNT; i++)
		players[i].frame = ma_frame_copy(main_frame);
	for(int i = 0; i < EXPLOSION_COUNT; i++)
		explosions[i].frame = 16;

	InitWindow(width, height, "testapp");
	Texture2D crown_texture = LoadTexture("demos/crown.png");
	Texture2D explosion_texture = LoadTexture("demos/explosion.png");
	float explosion_frame_time = 0;

	InitAudioDevice();
	Sound explosion_sound = LoadSound("demos/explosion.wav");
	Sound collision_sound = LoadSound("demos/collision.wav");
	Sound shoot_sound = LoadSound("demos/shooting.wav");
	Sound thrust_sound = LoadSound("demos/thruster.wav");

	Music music = LoadMusicStream("demos/pushingyourself.ogg");
	
	reset();

	float delta_time = 0;
	float prev_time = 0;

	bool requesting = false;
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);

	PlayMusicStream(music);

	float last_asteroid_time = GetTime();
	float asteroid_cooldown = 5.f;
	for(int i = 0; i < 5; i++) {
		generate_asteroid();
	}

	int players_joined = 0;
	while(!WindowShouldClose()) {
		// check if someone has joined
		for(int i = 0; i < PLAYER_COUNT; i++) {
			players[i].entity.enabled = players[i].joined;

			if(ma_client_active(i) && !players[i].joined) {
				ma_client_input_text_get(i, 0, players[i].name);
				if(players[i].name[0] != 0) {
					ma_frame_send(players[i].frame, i);
					players[i].joined = 1;
					players_joined++;
					printf("user: %s, joined!\n", players[i].name);
				}
			}
		}


		delta_time = GetTime() - prev_time;
		prev_time = GetTime();

		width  = GetScreenWidth();
		height = GetScreenHeight();

		camera.offset = (Vector2) { 0 };
		camera.zoom = 1.f;
		camera.rotation = 0.f;

		if(IsKeyPressed(KEY_R))
			reset();

		if(IsKeyDown(KEY_Z))
			requesting = !requesting;

		int asteroid_count = 0;
		for(int i = 0; i < ASTEROID_COUNT; i++)
			if(asteroids[i].entity.enabled)
				asteroid_count++;

		if(asteroid_count <= 3 && asteroid_cooldown + last_asteroid_time < GetTime()) {
			for(int i = 0; i < rand()%6; i++)
				generate_asteroid();
			last_asteroid_time = GetTime();
		}

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

							if( (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < 32) {
								reset_bullet(&bullets[i]);
								reset_bullet(&bullets[j]);
							}
						}
					}

					for(int j = 0; j < ASTEROID_COUNT; j++) {
						if(asteroids[j].entity.enabled) {
							float x2 = asteroids[j].entity.x;
							float y2 = asteroids[j].entity.y;

							if(entities_collide((EntityHeader*) &asteroids[j], (EntityHeader*) &bullets[i])) {
								float angle = atan2(y1, x1);

								if(asteroids[j].entity.rad / 2.f > 10.f) {
									spawn_asteroid(x2, y2, cos(angle), sin(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
									spawn_asteroid(x2, y2, sin(angle), cos(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
								}

								PlaySoundMulti(collision_sound);

								reset_bullet(&bullets[i]);
								reset_asteroid(&asteroids[j]);
							}
						}
					}

					for(int j = 0; j < PLAYER_COUNT; j++) {
						if(players[j].entity.enabled) {
							float x2 = players[j].entity.x;
							float y2 = players[j].entity.y;

							if(entities_collide((EntityHeader*) &players[j], (EntityHeader*) &bullets[i])) {
								float angle = atan2(y1, x1);

								if(j != bullets[i].source)
									players[bullets[i].source].score++;

								screen_shake(.5f);
								players[bullets[i].source].score += 1;

								if(players[j].health == 1)
									players[bullets[i].source].score += 5 + 10 * players[j].first;

								players[j].health = (players[j].health > 0) * (players[j].health - 1);
								PlaySoundMulti(collision_sound);


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
			if(ma_client_active(i) && players[i].joined && players[i].health) {

				bool b1 = 0;
				ma_client_input_button_get(i, 0, (unsigned char*) &b1);

				float angle = players[i].entity.rot - PI / 2.f;
				float dir_x = cos(angle);
				float dir_y = sin(angle);

				// movement
				if(b1) {
					players[i].entity.vel_x += dir_x * 4.f * delta_time;
					players[i].entity.vel_y += dir_y * 4.f * delta_time;
					players[i].boosting = 1;
				}
				else
					players[i].boosting = 0;

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
				if(b2 && !players[i].shooting && players[i].last_shot + .5f < GetTime()) {
					shoot(i, dir_x, dir_y, 240.f);

					// add recoil
					players[i].entity.vel_x -= dir_x * 1.5f;
					players[i].entity.vel_y -= dir_y * 1.5f;

					players[i].last_shot = GetTime();
					players[i].shooting = 1;
					PlaySoundMulti(shoot_sound);
				}
				else if(!b2)
					players[i].shooting = 0;

				// player collisions

				for(int j = 0; j < PLAYER_COUNT; j++) {
					if(ma_client_active(j) && j != i && players[j].joined && players[j].health) {
						if(entities_collide((EntityHeader*) &players[j], (EntityHeader*) &players[i])) {
							players[i].health = 0;
							players[i].entity.vel_x = 0;
							players[i].entity.vel_y = 0;
							players[j].health = 0;
							players[j].entity.vel_x = 0;
							players[j].entity.vel_y = 0;
							PlaySoundMulti(explosion_sound);
							spawn_explosion(players[i].entity.x, players[i].entity.y);
							screen_shake(.5f);
						}
					}
				}

				for(int j = 0; j < ASTEROID_COUNT; j++) {
					if(entities_collide((EntityHeader*) &asteroids[j], (EntityHeader*) &players[i])) {
							players[i].health = 0;
							players[i].entity.vel_x = 0;
							players[i].entity.vel_y = 0;
						float angle = atan2(players[i].entity.x, players[i].entity.y);

						if(asteroids[j].entity.rad / 2.f > 10.f) {
							spawn_asteroid(asteroids[j].entity.x, asteroids[j].entity.y, cos(angle), sin(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
							spawn_asteroid(asteroids[j].entity.x, asteroids[j].entity.y, sin(angle), cos(angle), asteroids[j].entity.rad / 2.f, asteroids[j].entity.speed * 2);
							PlaySoundMulti(explosion_sound);
						}
						reset_asteroid(&asteroids[j]);
						PlaySoundMulti(explosion_sound);
						spawn_explosion(players[i].entity.x, players[i].entity.y);
						screen_shake(.5f);
					}
				}
			}
#ifdef RESPAWN
			else if(ma_client_active(i) && players[i].joined) {
				players[i].entity.x = rand() % width;
				players[i].entity.y = rand() % height;
				players[i].health = 3;
				players[i].score = 0;
				players[i].entity.vel_x = 0;
				players[i].entity.vel_y = 0;
			}
#endif
		}

		int highest_score = 0;
		int highest_index = 0;
		for(int i = 0; i < PLAYER_COUNT; i++) {
			players[i].first = 0;
			if(players[i].score > highest_score) {
				highest_score = players[i].score;
				highest_index = i;
			}
		}

		if(shake_duration > 0) {
			camera.offset.x += rand()%20;
			camera.offset.y += rand()%20;
			shake_duration -= delta_time;
		}
		else {
			camera.offset.x = 0;
			camera.offset.y = 0;
		}

		if(highest_score > 0)
			players[highest_index].first = 1;

		if(explosion_frame_time < GetTime()) {
			explosion_frame_time = GetTime() + .06f;
			for(int i = 0; i < EXPLOSION_COUNT; i++) {
				if(explosions[i].frame < EXPLOSION_FRAMES)
					explosions[i].frame++;
			}
		}

#ifndef RESPAWN
		int alive = 0;
		int last_alive_index = 0;
		for(int i = 0; i < PLAYER_COUNT; i++) {
			if(players[i].health && ma_client_active(i) && players[i].joined) {
				last_alive_index = i;
				alive++;
			}
		}

		if(alive == 1 && players_joined > 1) {
			BeginDrawing();
			char string[40];
			printf("%s wins\n", players[last_alive_index].name);
			sprintf(string, "%s WINS!!!", players[last_alive_index].name);
			DrawText(string, GetScreenWidth() / 2.f - MeasureText(string, 30) / 2.f, GetScreenHeight() / 2.f, 30, WHITE);
			EndDrawing();
			float timer = GetTime() + .2f;
			while(timer > GetTime());		
			reset();
		}
#endif

		ClearBackground(BLACK);
		BeginDrawing();
		BeginMode2D(camera);

		float offsets[5 * 2] = {
			 0, 0,
			 width, 0,
			 0,  height,
			-width, 0,
			 0, -height,
		};

		// draw ships
		for(int i = 0; i < PLAYER_COUNT; i++)
			if(ma_client_active(i) && players[i].joined && players[i].health) {
				float angle = players[i].entity.rot;
				float angle_cos = cos(angle + PI);
				float angle_sin = sin(angle + PI); 

				// very intuitive matrix maths
				for(int j = 0; j < 5; j++) {
					float x = players[i].entity.x + offsets[j * 2 + 0] + (10.f * angle_sin);
					float y = players[i].entity.y + offsets[j * 2 + 1] - (10.f * angle_cos);

					DrawTriangleLines((Vector2) { x - (10.f * angle_cos), y - (10.f * angle_sin) },
									  (Vector2) { x - (25.f * angle_sin), y + (25.f * angle_cos) },
									  (Vector2) { x + (10.f * angle_cos), y + (10.f * angle_sin) },
									  players[i].health > 1 ? players[i].col : (Color) {players[i].col.r, players[i].col.g, players[i].col.b, (sin(GetTime() * 20)) * 50 + 200});

					DrawText(players[i].name, x - (50.f * angle_sin) - 20, y + (50.f * angle_cos), 15, players[i].col);
					char score[5];
					sprintf(score, "%i", players[i].score);
					DrawText(score, x - (50.f * angle_sin) - 20, y + (50.f * angle_cos) + 20, 15, players[i].col);
#ifdef LASER
					DrawLine(x - (30.f * angle_sin), y + (30.f * angle_cos), x - (200.f * angle_sin), y + (200.f * angle_cos), RED);
#endif

					if(players[i].boosting) {
						DrawTriangleLines((Vector2) { x - (5.f * angle_cos), y - (5.f * angle_sin) },
										  (Vector2) { x + ((10.f + (1.f * (sin(GetTime() * 10)))) * angle_sin), y - ((10.f + (1.f * (sin(GetTime() * 10)))) * angle_cos) },
										  (Vector2) { x + (5.f * angle_cos), y + (5.f * angle_sin) },
										  GREEN);
					}

					if(players[i].first) {
						DrawTextureEx(crown_texture, (Vector2) { x + (30.f * angle_sin) - (20 * angle_cos), y - (30.f * angle_cos) - (20.f * angle_sin) }, angle * 180.f / PI + 180.f, .05f, WHITE);
					}
				}
			}

		for(int i = 0; i < BULLET_COUNT; i++) {
			DrawRectangle(bullets[i].entity.x - 2, bullets[i].entity.y - 2, 4, 4, WHITE);
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

		for(int i = 0; i < EXPLOSION_COUNT; i++) {
			int frame = explosions[i].frame;
			if(frame < EXPLOSION_FRAMES) {
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

	UnloadSound(collision_sound);
	UnloadSound(shoot_sound);
	UnloadSound(explosion_sound);
	UnloadSound(thrust_sound);
	UnloadMusicStream(music);
	StopSoundMulti();
	CloseAudioDevice();
	CloseWindow();
	ma_frame_destroy(main_frame);
	ma_free();
}
