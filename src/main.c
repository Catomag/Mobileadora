// THIS FILE IS ONLY HERE FOR TESTING PURPOSES
#include "../include/library.h"
#include <stdio.h>
#include <unistd.h>
#include <raylib.h>

#define PLAYER_COUNT 60
#define WIDTH 1300
#define HEIGHT 720

typedef struct {
	float x;
	float y;
	int thicc;
	Color color;
} Player;

typedef struct {
	float x;
	float y;
} Pellet;

// Simple form
int main() {

	l_init(PLAYER_COUNT, 8000);

	Frame* main_frame = l_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL);
	l_frame_input_add(main_frame, l_input_joystick_create());
//	l_send(main_frame, )

	// Contents of first frame
//	l_frame_text_add(main_frame, "First name:");
//	l_frame_input_add(main_frame, l_input_text_create(20));
//	l_frame_text_add(main_frame, "Last name:");
//	l_frame_input_add(main_frame, l_input_text_create(30));
//	l_frame_input_add(main_frame, l_input_submit_create("Submit"));
//
//	// wait 10 seconds for clients to join
//	sleep(10);
//	unsigned int client_count = l_client_count();
//	for(int i = 0; i < client_count; i++) {
//		l_send(main_frame, i);
//	}
//
//	// wait 20 seconds
//	sleep(20);
//	l_frame_fetch(main_frame); // force clients to submit data
//	l_poll(); // update internal data
//
//	char first_client_name[20];
//	l_input_text_get(0, 0, first_client_name);
//
//	char names[20 * client_count];
//	char last_names[30 * client_count];
//	for(int i = 0; i < client_count; i++) {
//		l_input_text_get(i, 0, &names[20 * i]);
//		l_input_text_get(i, 1, &last_names[20 * i]);
//	}
	
	// do stuff with data
	l_frame_print(main_frame);
	l_frame_default(main_frame);

	InitWindow(WIDTH, HEIGHT, "testapp");
	

#define PELLET_COUNT 50

	Player players[PLAYER_COUNT];
	Pellet pellets[PELLET_COUNT];

	for(int i = 0; i < PELLET_COUNT; i++) {
		pellets[i].x = rand()%WIDTH;
		pellets[i].y = rand()%HEIGHT;
	}

	for(int i = 0; i < PLAYER_COUNT; i++) {
		players[i].x = rand()%WIDTH;
		players[i].y = rand()%HEIGHT;

		players[i].thicc = 8;
		players[i].color = (Color) { rand()%255, rand()%255, rand()%255, 255 };
	}

	float time = 0;
	float prev_time = 0;
	while(!WindowShouldClose()) {
		time += GetTime() - prev_time;
		prev_time = GetTime();

		if(time > .008f) {
			time = 0;

			for(int i = 0; i < PLAYER_COUNT; i++) {
				if(l_client_active(i)) {
					// movement
					float dir_x;
					float dir_y;
					if(l_input_joystick_get(i, 0, &dir_x, &dir_y)) {
						players[i].x += (3.f / players[i].thicc) * 640.f * dir_x * GetFrameTime();
						players[i].y -= (3.f / players[i].thicc) * 640.f * dir_y * GetFrameTime();

						if(players[i].x < 0)
							players[i].x = 0;
						else if(players[i].x > WIDTH)
							players[i].x = WIDTH;

						if(players[i].y < 0)
							players[i].y = 0;
						else if(players[i].y > HEIGHT)
							players[i].y = HEIGHT;
					}

					// eat pelet
					for(int j = 0; j < PELLET_COUNT; j++) {
						if( (players[i].x - pellets[j].x) * (players[i].x - pellets[j].x) +
							(players[i].y - pellets[j].y) * (players[i].y - pellets[j].y) < 9 + (players[i].thicc) * (players[i].thicc)) {
							players[i].thicc += 1;
							pellets[j].x = rand() % WIDTH;
							pellets[j].y = rand() % HEIGHT;
						}
					}

					// player collisions

					for(int j = 0; j < PLAYER_COUNT; j++) {
						if(l_client_active(j)) {
							if( (players[i].x - players[j].x) * (players[i].x - players[j].x) +
								(players[i].y - players[j].y) * (players[i].y - players[j].y) < (players[i].thicc / 2) * (players[i].thicc / 2) &&
								players[i].thicc > players[j].thicc) {

								players[i].thicc += players[j].thicc / 2;
								players[j].thicc = 8;
								players[j].x = rand()%WIDTH;
								players[j].y = rand()%HEIGHT;
							}
						}
					}

//					if(players[i].thicc > 30)
//						players[i].thicc -= (rand()%players[i].thicc) / 200;
				}
			}

			ClearBackground(BLACK);
			BeginDrawing();

			for(int i = 0; i < PELLET_COUNT; i++)
				DrawCircle(pellets[i].x, pellets[i].y, 3, GREEN);

			for(int i = 0; i < PLAYER_COUNT; i++)
				if(l_client_active(i))
					DrawCircle(players[i].x, players[i].y, players[i].thicc, players[i].color);
			EndDrawing();
		}
	}

	CloseWindow();
	l_frame_destroy(main_frame);
	printf("this ran\n");
}
