
int note_to_scancode[NOTE_COUNT] = {
	29, // C-0
	22, // C#0
	27, // D-0
	7,  // D#0
	6,  // E-0
	25, // F-0
	10, // F#0
	5,  // G-0
	11, // G#0
	17, // A-0
	13, // A#0
	16, // B-0
	20, // C-1
	31, // C#1
	26, // D-1
	32, // D#1
	8,  // E-1
	21, // F-1
	34, // F#1
	23, // G-1
	35, // G#1
	28, // A-1
	36, // A#1
	24, // B-1
	12, // C-2
};
char accidentals[7] = { 1, 1, 0, 1, 1, 1, 0 };
int white_keys_to_note[15] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24 };
int black_keys_to_note[14] = { 1, 3, -1, 6, 8, 10, -1, 13, 15, -1, 18, 20, 22, -1 };

SDL_Texture * keyboard_texture;

SDL_Rect keyboard_rect = { 0, 0, 0, 0 };
SDL_Rect keys_white_rect = { 0, 0, 20, 100 };
SDL_Rect keys_black_rect = { 0, 0, 10, 60 };

int keys_mouse_hover = 0;
int keys_mouse_target = 0;

void musical_keyboard_init() {
	keyboard_rect.y = window_h / 2;
	keyboard_rect.w = window_w;
	keyboard_rect.h = window_h / 2;
}

void musical_keyboard_draw(SDL_Renderer * renderer) {
	keys_mouse_hover = 0;
	keyboard_rect.w = window_rect.w;
	keyboard_rect.h = window_rect.h / 4;
	keyboard_rect.y = window_rect.h / 2;
	// white keys
	keys_white_rect.w = (keyboard_rect.w - (15 * KEY_MARGIN) - 20) / 15;
	keys_white_rect.h = keyboard_rect.h - 10;
	keys_white_rect.x = 10;
	keys_white_rect.y = keyboard_rect.y;
	for (int i = 0; i < 15; i++) {
		if (notes_gate[white_keys_to_note[i]]) {
			renderer_set_color(renderer, &palette[2]); 
		}
		else {
			renderer_set_color(renderer, &palette[0]);
		}
		SDL_RenderFillRect(renderer, &keys_white_rect);
		if (collision_detection(keys_white_rect, mouse_hotspot)) {
			keys_mouse_hover = 1;
			keys_mouse_target = white_keys_to_note[i];
		}
		keys_white_rect.x += keys_white_rect.w + KEY_MARGIN;
	}
	// black keys
	keys_black_rect.w = keys_white_rect.w / 2;
	keys_black_rect.h = keys_white_rect.h * 0.6;
	keys_black_rect.y = keyboard_rect.y;
	for (int i = 0; i < 14; i++) {
		if (accidentals[i % 7]) {
			if (notes_gate[black_keys_to_note[i]]) {
				renderer_set_color(renderer, &palette[2]); 
			}
			else {
				renderer_set_color(renderer, &palette[7]);
			}
			keys_black_rect.x = i * (keys_white_rect.w + KEY_MARGIN) + 10 + keys_white_rect.w * 0.8;
			SDL_RenderFillRect(renderer, &keys_black_rect);
			if (collision_detection(keys_black_rect, mouse_hotspot)) {
				keys_mouse_hover = 1;
				keys_mouse_target = black_keys_to_note[i];
			}
		}
	}
}
