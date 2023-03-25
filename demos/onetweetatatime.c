#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "../include/mobileadora.h"

#define MAX_PLAYERS 20
#define MAX_CHARS 50


typedef struct {
	char name[15];
	int playing;
	int submitted;
	Frame* write_frame;
} Player;

typedef struct {
	float end_time;
} Timer;

typedef struct {
	char text[MAX_CHARS * MAX_PLAYERS];
	int contributers[MAX_PLAYERS];
	int contributions;
} Story;



Player players[MAX_PLAYERS];
int active_players = 0;

Story stories[MAX_PLAYERS];

Font default_font;

Vector2 text_measure(const char* text, float fontSize) {
	return MeasureTextEx(default_font, text, fontSize, 1.f);
}
void text(const char* t, float x, float y, float fontSize, Color color) {
	DrawTextEx(default_font, t, (Vector2) {x, y}, fontSize, 1.f, color);
}
void text_centered(const char* t, float x, float y, float fontSize, Color color) {
	Vector2 m = text_measure(t, fontSize);
	text(t, x - (m.x / 2), y - (m.y / 2), fontSize, color);
}
void text_screen(const char* t, float fontSize, Color color) {
	Vector2 m = text_measure(t, fontSize);
	text(t, (GetScreenWidth() / 2) - (m.x / 2), (GetScreenHeight() / 2) - (m.y / 2), fontSize, color);
}

unsigned int min(unsigned int a, unsigned int b) {
	if(a > b)
		return b;
	else
		return a;
}

void draw_player_names() {
	int current = 0;
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i].playing) {
			text(players[i].name, current * (GetScreenWidth() / active_players), GetScreenHeight() - 20, 30, players[i].submitted ? WHITE : GRAY);
			current++;
		}
	}
}

int main() {
	ma_init(MAX_PLAYERS, 8000);

	InitWindow(1280, 720, "gaming");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	default_font = LoadFontEx("roboto.ttf", 30, NULL, 0);

	Frame* join_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_h3_add(join_frame, "Write your name");
	ma_frame_input_text_add(join_frame, 14);
	ma_frame_input_submit_add(join_frame);

	Frame* wait_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_h3_add(wait_frame, "Thank you! Now waiting for all the other gamers");

	Frame* oops_took_too_long_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_h3_add(oops_took_too_long_frame, "sorry mate, but the game's already started");

	Frame* write_frame = ma_frame_create(FRAME_STATIC, ORIENTATION_VERTICAL, 1, 0);
	ma_frame_element_text_add(write_frame, "                                                                                                      ");
	ma_frame_input_text_add(write_frame, MAX_CHARS);
	ma_frame_input_submit_add(write_frame);

	for(int i = 0; i < MAX_PLAYERS; i++) {
		players[i].write_frame = ma_frame_copy(write_frame);
	}

	int running = 1;

	while(running) {
		for(int i = 0; i < MAX_PLAYERS; i++) {
			players[i].name[0] = 0;
			players[i].playing = 0;
			players[i].submitted = 0;
			players[i].write_frame = ma_frame_copy(write_frame);
		}
		ma_frame_default(join_frame);
		memset(stories, 0, sizeof(Story) * MAX_PLAYERS);
		active_players = 0;

		// draw start screen, letting players join
		while(1) {
			if(WindowShouldClose() || IsKeyPressed(KEY_Q)) {
				running = 0;
				break;
			}

			else if(IsKeyPressed(KEY_R)) {
				break;
			}

			for(int i = 0; i < MAX_PLAYERS; i++) {
				if(ma_client_active(i)) {
					ma_client_input_text_get(i, 0, players[i].name);
					if(players[i].name[0] != 0 && !players[i].playing) {
						players[i].playing = 1;
						printf("%s joined!\n", players[i].name);
						ma_frame_send(wait_frame, i);
						active_players++;
					}
				}
			}

			BeginDrawing();
			ClearBackground(BLACK);

			text_screen("Join. Press \"r\" when ready.", 30, WHITE);
			draw_player_names();

			EndDrawing();
		}

		// if they didn't join by now then its too bad
		ma_frame_default(oops_took_too_long_frame);

		for(int i = 0; i < MAX_PLAYERS; i++) {
			if(players[i].playing) {
				players[i].submitted = 0;
				//ma_frame_element_text_set(players[i].write_frame, 0, " ");
				ma_frame_send(players[i].write_frame, i);
			}
			else
				ma_frame_send(oops_took_too_long_frame, i);
		}

		// draw countdown
		for(int round = 0; round < active_players; round++) {
			printf("round: %i!\n", round);
			if(round != 0) {
				for(int i = 0; i < MAX_PLAYERS; i++) {
					if(players[i].playing) {
						int len = strlen(stories[(i + round) % active_players].text);
						unsigned int offset = 0;
						if(len > MAX_CHARS)
							offset = len - MAX_CHARS;

						printf("length: %i\n", len);
						printf("offset: %i\n", offset);
						printf("Story %i, so far: \n --- \n %s \n---\n", (i + round) % active_players, stories[(i + round) % active_players].text);
						ma_frame_element_text_set(players[i].write_frame, 0, stories[(i + round) % active_players].text + offset);
					}
				}
			}

			for(int i = 0; i < MAX_PLAYERS; i++)
				players[i].submitted = 0;

			ma_flush();

			float time = GetTime();
			float countdown = 20.f + GetTime();
			while(time < countdown) {
				time = GetTime();

				int submissions = 0;
				for(int i = 0; i < MAX_PLAYERS; i++) {
					if(players[i].playing) {
						char buffer[MAX_CHARS];
						ma_client_input_text_get(i, 0, buffer);
						submissions += players[i].submitted;

						if(buffer[0] != 0 && !players[i].submitted) {
							players[i].submitted = 1;
							strcat(stories[(i + round) % active_players].text, buffer);
							stories[(i + round) % active_players].contributers[stories[(i + round) % active_players].contributions] = i;
							stories[(i + round) % active_players].contributions++;
							ma_frame_send(wait_frame, i);
						}
					}
				}

				if(submissions == active_players)
					break;

				BeginDrawing();
				ClearBackground(BLACK);
				text_screen("Write your answers!", 30.f, WHITE);
				char time_text[100] = "Only %f seconds LEFT!!";
				sprintf(time_text, "Only %.2f seconds LEFT!!!", countdown - GetTime());
				text_centered(time_text, GetScreenWidth() / 2, GetScreenHeight() / 2 + 30.f, 30.f, WHITE);
				draw_player_names();
				EndDrawing();
			}


			// deception
			time = GetTime();
			countdown = 1.f + GetTime();
			while(time < countdown) {
				time = GetTime();

				BeginDrawing();
				ClearBackground(BLACK);
				text_centered("Collecting responses...", GetScreenWidth() / 2, GetScreenHeight() / 2, 30, WHITE);
				EndDrawing();
			}

			// force people who haven't submitted into submitting also reset submission
			for(int i = 0; i < MAX_PLAYERS; i++) {
				if(!players[i].submitted && players[i].playing) {
					ma_fetch(i);
					char buffer[MAX_CHARS];
					ma_client_input_text_get(i, 0, buffer);

					if(strlen(buffer) > 1) {
						players[i].submitted = 1;
						strcat(stories[i + round].text, buffer);
						stories[i + round].contributers[stories[i + round].contributions] = i;
						stories[i + round].contributions++;
					}
					ma_frame_send(wait_frame, i);
				}
				players[i].submitted = 0;
			}


			time = GetTime();
			countdown = 2.f + GetTime();
			while(time < countdown) {
				time = GetTime();

				BeginDrawing();
				ClearBackground(BLACK);
				text_centered("Get ready to write!!", GetScreenWidth() / 2, GetScreenHeight() / 2, 30, WHITE);
				EndDrawing();
			}

		}

		// draw waiting music showing which players have submitted and which haven't

		// show players a random story and ask for a title

		// choose to play again or not
		while(1) {
			if(IsKeyPressed(KEY_Q)) {
				running = 0;
				break;
			}

			else if(IsKeyPressed(KEY_R)) {
				break;
			}
		}
	}

	ma_deinit();
	return 0;
}

