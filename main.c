#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define DEBUG 1
#include "lib/core.c"

#define NOTE_COUNT 25
#define FREQ_CENTER 440
#define SAMPLE_RATE 48000
#define KEY_MARGIN 5
// the C below A's FREQ_CENTER
// A 440 is octave 4 A
#define BASE_NOTE -9

unsigned long time_counter = 0;

typedef struct {
	float attack;
	float decay;
	float sustain;
	float release;
} adsr_envelope;
adsr_envelope amp_adsr;
float amp_adsr_pos[NOTE_COUNT];
float amp_adsr_release[NOTE_COUNT];
int amp_adsr_stage[NOTE_COUNT];

int octave;
float thiccness;
float volume;

#include "src/window.c"
#include "src/mouse.c"
#include "src/palette.c"
#include "src/voice.c"
#include "src/cornputer_keyboard.c"
#include "src/musical_keyboard.c"
#include "src/scope.c"
#include "src/osc_options.c"

char preset_filename[] = "settings.bin";

adsr_envelope filter_adsr;

typedef struct {
	float cutoff;
	float res;
} filter_data;
filter_data filter;

float lfo_pos;
int note_most_recent;

#include "src/audio_callback.c"


#define KNOB_COUNT 9
knob knobs[KNOB_COUNT] = {
	// ADSR1 attack
	{ 0.005f, 25.f, 0.f, 0.f, 0.5f, 2.5f,
		"ATTACK", { 10, 20, 72, 72 } },
	// ADSR1 decay
	{ 0.005f, 25.f, 0.f, 0.f, 0.1f, 2.5f,
		"DECAY", { 92, 20, 72, 72 } },
	// ADSR1 sustain
	{ 0.f, 1.f, 0.f, 0.f, 0.75f, 0.5f,
		"SUSTAIN", { 174, 20, 72, 72 } },
	// ADSR1 release
	{ 0.005f, 25.f, 0.f, 0.f, 0.250f, 2.5f,
		"RELEASE", { 256, 20, 72, 72 } },
	// Filter Frequency
	{ 0.01f, 0.99f, 0.f, 0.f, 0.8f, 2.5f,
		"CUTOFF", { 400, 20, 72, 72 } },
	// Filter Resonance
	{ 0.f, 1.f, 0.f, 0.f, 0.10f, 0.25f,
		"Q", { 482, 20, 72, 72 } },
	// Volume
	{ 0.f, 1.f, 0.f, 0.f, 0.25f, 1.f,
		"VOLUME", { 708, 20, 72, 72 } },
	// Thiccness
	{ 0.f, 1.f, 0.f, 0.f, 0.25f, 2.125f,
		"THICC", { 708, 152, 72, 72 } },
	// base octave
	{ 0.f, 8.f, 0.f, 0.f, 2.f, 1.f,
		"OCTAVE", { 595, 20, 72, 72 } },
};
char amp_attack_val_str[8];
SDL_Rect amp_attack_val_rect = { 18, 100, 56, 8 };
SDL_Rect amp_attack_label_rect = { 18, 8, 56, 8 };
char amp_decay_val_str[8];
SDL_Rect amp_decay_val_rect = { 100, 100, 56, 8 };
SDL_Rect amp_decay_label_rect = { 100, 8, 56, 8 };
char amp_sustain_val_str[8];
SDL_Rect amp_sustain_val_rect = { 182, 100, 56, 8 };
SDL_Rect amp_sustain_label_rect = { 182, 8, 56, 8 };
char amp_release_val_str[8];
SDL_Rect amp_release_val_rect = { 264, 100, 56, 8 };
SDL_Rect amp_release_label_rect = { 264, 8, 56, 8 };
char filter_freq_val_str[8];
SDL_Rect filter_freq_val_rect = { 400, 100, 56, 8 };
SDL_Rect filter_freq_label_rect = { 400, 8, 56, 8 };
char filter_q_val_str[8];
SDL_Rect filter_q_val_rect = { 482, 100, 56, 8 };
SDL_Rect filter_q_label_rect = { 482, 8, 56, 8 };
char volume_val_str[8];
SDL_Rect volume_val_rect = { 716, 100, 56, 8 };
SDL_Rect volume_label_rect = { 716, 8, 56, 8 };
char thiccness_val_str[8];
SDL_Rect thiccness_val_rect = { 716, 228, 56, 8 };
SDL_Rect thiccness_label_rect = { 716, 136, 56, 8 };
char octave_val_str[8];
SDL_Rect octave_val_rect = { 595, 100, 56, 8 };
SDL_Rect octave_label_rect = { 595, 8, 56, 8 };



int main(int argc, char* args[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	voice_freq_init(); 
	audio_init(SAMPLE_RATE, 2, 1024, AUDIO_F32SYS, &audio_callback);

	SDL_Event event;

	window_init();
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	// char rom initialization
	char_rom_load_set(renderer, char_rom_eagle_pc_cga);

	musical_keyboard_init();
	osc_options_init(renderer);
	scope_init(renderer);

	//keys_debug = 1;

	char_rom_set_color(&palette[0]); // white text
	
	// knob labels and val fields
	SDL_Texture * amp_attack_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(amp_attack_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, amp_attack_label_texture, "Attack");
	SDL_Texture * amp_attack_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * amp_decay_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(amp_decay_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, amp_decay_label_texture, "Decay");
	SDL_Texture * amp_decay_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * amp_sustain_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(amp_sustain_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, amp_sustain_label_texture, "Sustain");
	SDL_Texture * amp_sustain_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * amp_release_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(amp_release_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, amp_release_label_texture, "Release");
	SDL_Texture * amp_release_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * filter_freq_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(filter_freq_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, filter_freq_label_texture, "Cutoff");
	SDL_Texture * filter_freq_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * filter_q_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(filter_q_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, filter_q_label_texture, "Res / Q");
	SDL_Texture * filter_q_val_texture = texture_create_generic(renderer, 56, 8);
	
	SDL_Texture * volume_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(volume_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, volume_label_texture, "Volume");
	SDL_Texture * volume_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * thiccness_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(thiccness_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, thiccness_label_texture, "Thicc");
	SDL_Texture * thiccness_val_texture = texture_create_generic(renderer, 56, 8);

	SDL_Texture * octave_label_texture = texture_create_generic(renderer, 56, 8);
	SDL_SetTextureBlendMode(octave_label_texture, SDL_BLENDMODE_BLEND);
	char_rom_string_to_texture(renderer, octave_label_texture, "Octave");
	SDL_Texture * octave_val_texture = texture_create_generic(renderer, 56, 8);

	FILE * preset = fopen(preset_filename, "r");
	for (int i = 0; i < KNOB_COUNT; i++) {
		fread(&knobs[i].val, sizeof(float), 1, preset);
	}
	fread(&osc_option_selected, sizeof(char), 1, preset);
	fclose(preset);

	// knob texture
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	SDL_Texture * knob_texture = texture_from_image(renderer, "assets/knob2-smaller.png");
	for (int i = 0; i < KNOB_COUNT; i++) knob_init(&knobs[i]);
	// knob emboss texture
	SDL_Texture * knob_emboss_texture = texture_from_image(renderer, "assets/knob2-smaller-emboss.png");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	// mouse cursor
	mouse_data mouse = mouse_init();
	int mouse_knob_grab = 0;
	int mouse_knob_hover = 0;
	int mouse_knob_target = 0; // knob id of grabbed knob
	int mouse_osc_select_hover = 0;
	SDL_ShowCursor(SDL_DISABLE);
	SDL_Texture * mouse_hand_closed = texture_from_image(renderer, "assets/cursor_hand_closed.png");
	SDL_Texture * mouse_hand_1_finger = texture_from_image(renderer, "assets/cursor_hand_1_finger.png");
	SDL_Texture * mouse_hand_open = texture_from_image(renderer, "assets/cursor_hand_open.png");
	SDL_Texture * mouse_pointer = texture_from_image(renderer, "assets/cursor_pointer.png");
	

	int running = 1;
	while (running) {

		SDL_SetRenderTarget(renderer, NULL);
		renderer_set_color(renderer, &palette[3]);
		SDL_RenderClear(renderer);
		
		// mouse updates
		mouse_process(&mouse, &window_rect);
		mouse.x -= 8;
		mouse.y -= 8;
		mouse_cursor_rect.x = mouse.x;
		mouse_hotspot.x = mouse_cursor_rect.x;
		mouse_cursor_rect.y = mouse.y;
		mouse_hotspot.y = mouse_cursor_rect.y;

		// note handling (cornputer keybaord and mouse)
		for (int i = 0; i < NOTE_COUNT; i++) {
			if (window_has_focus && (keys_pressed[note_to_scancode[i]] || (!mouse_knob_grab && keys_mouse_hover && mouse.button_left && keys_mouse_target == i))) {
				if (voices[i].gate == 0) {
					amp_adsr_stage[i] = 0;
					note_most_recent = i;
				}
				voices[i].gate = 1;
			}
			else {
				if (voices[i].gate == 1) {
					amp_adsr_stage[i] = 3;
				}
				amp_adsr_release[i] = 1.f / (amp_adsr.release * (float) SAMPLE_RATE);
				voices[i].gate = 0;
			}
		}


		// mouse gui stuff
		mouse_knob_hover = 0;
		mouse_osc_select_hover = 0;
		for (int i = 0; i < KNOB_COUNT; i++) {
			SDL_RenderCopyEx(renderer, knob_texture, NULL, &knobs[i].rect, knobs[i].rot, NULL, SDL_FLIP_NONE);
			SDL_RenderCopy(renderer, knob_emboss_texture, NULL, &knobs[i].rect);
			if (window_has_focus && collision_detection(knobs[i].rect, mouse_hotspot)) {
				mouse_knob_hover = 1;
				if (!mouse_knob_grab) mouse_knob_target = i;
			}
		}
		if (mouse_knob_hover && mouse.button_left) mouse_knob_grab = 1;
		if (!mouse.button_left) mouse_knob_grab = 0;
		if (mouse_knob_grab) {
			// change value based on mouse movement
			knob_update_relative(&knobs[mouse_knob_target], (mouse.rel_x - mouse.rel_y) * 0.001f);
			//printf("attack: %6.3f  decay: %6.3f  sustain: %6.3f  release: %6.3f\n", knobs[0].val, knobs[1].val, knobs[2].val, knobs[3].val);
			//printf("attack: %6d  decay: %6d  sustain: %6.3f  release: %6d\n", amp_adsr.attack, amp_adsr.decay, amp_adsr.sustain, amp_adsr.release);
		}
		else if (collision_detection(osc_options_rect, mouse_hotspot)) {
			mouse_osc_select_hover = 1;
			if (mouse.button_left) {
				int osc_option_row = (mouse_hotspot.y - mouse_hotspot.h - osc_options_rect.y + 4) / 8;
				if (osc_option_row >= osc_option_count) osc_option_row = osc_option_count - 1;
				osc_option_selected = osc_visible_options[osc_option_row];
			}
		}

		// process/draw  knobs, labels and values
		// set text color to yellow
		char_rom_set_color(&palette[1]);

		amp_adsr.attack = 1.f / (knobs[0].val * (float) SAMPLE_RATE);
		sprintf(amp_attack_val_str, "%6.3fs", knobs[0].val);
		char_rom_string_to_texture(renderer, amp_attack_val_texture, amp_attack_val_str);
		SDL_RenderCopy(renderer, amp_attack_val_texture, NULL, &amp_attack_val_rect);
		SDL_RenderCopy(renderer, amp_attack_label_texture, NULL, &amp_attack_label_rect);

		amp_adsr.decay = (1.f - knobs[2].val) / (knobs[1].val * (float) SAMPLE_RATE);
		sprintf(amp_decay_val_str, "%6.3fs", knobs[1].val);
		char_rom_string_to_texture(renderer, amp_decay_val_texture, amp_decay_val_str);
		SDL_RenderCopy(renderer, amp_decay_val_texture, NULL, &amp_decay_val_rect);
		SDL_RenderCopy(renderer, amp_decay_label_texture, NULL, &amp_decay_label_rect);

		amp_adsr.sustain = knobs[2].val;
		sprintf(amp_sustain_val_str, "%6.2f%%", knobs[2].val * 100);
		char_rom_string_to_texture(renderer, amp_sustain_val_texture, amp_sustain_val_str);
		SDL_RenderCopy(renderer, amp_sustain_val_texture, NULL, &amp_sustain_val_rect);
		SDL_RenderCopy(renderer, amp_sustain_label_texture, NULL, &amp_sustain_label_rect);

		amp_adsr.release = knobs[3].val;
		sprintf(amp_release_val_str, "%6.3fs", knobs[3].val);
		char_rom_string_to_texture(renderer, amp_release_val_texture, amp_release_val_str);
		SDL_RenderCopy(renderer, amp_release_val_texture, NULL, &amp_release_val_rect);
		SDL_RenderCopy(renderer, amp_release_label_texture, NULL, &amp_release_label_rect);

		filter.cutoff = (knobs[4].val);
		sprintf(filter_freq_val_str, " %4.0fHz", knobs[4].val * (float) SAMPLE_RATE * 0.15f);
		char_rom_string_to_texture(renderer, filter_freq_val_texture, filter_freq_val_str);
		SDL_RenderCopy(renderer, filter_freq_val_texture, NULL, &filter_freq_val_rect);
		SDL_RenderCopy(renderer, filter_freq_label_texture, NULL, &filter_freq_label_rect);

		filter.res = (knobs[5].val);
		sprintf(filter_q_val_str, "%6.2f%%", knobs[5].val * 100.f);
		char_rom_string_to_texture(renderer, filter_q_val_texture, filter_q_val_str);
		SDL_RenderCopy(renderer, filter_q_val_texture, NULL, &filter_q_val_rect);
		SDL_RenderCopy(renderer, filter_q_label_texture, NULL, &filter_q_label_rect);

		volume = (knobs[6].val);
		sprintf(volume_val_str, "%6.2f%%", knobs[6].val * 100.f);
		char_rom_string_to_texture(renderer, volume_val_texture, volume_val_str);
		SDL_RenderCopy(renderer, volume_val_texture, NULL, &volume_val_rect);
		SDL_RenderCopy(renderer, volume_label_texture, NULL, &volume_label_rect);

		thiccness = (knobs[7].val);
		sprintf(thiccness_val_str, "%6.2f%%", knobs[7].val * 100.f);
		char_rom_string_to_texture(renderer, thiccness_val_texture, thiccness_val_str);
		SDL_RenderCopy(renderer, thiccness_val_texture, NULL, &thiccness_val_rect);
		SDL_RenderCopy(renderer, thiccness_label_texture, NULL, &thiccness_label_rect);

		octave = (int) knobs[8].val;
		sprintf(octave_val_str, "  %d  ", octave);
		char_rom_string_to_texture(renderer, octave_val_texture, octave_val_str);
		SDL_RenderCopy(renderer, octave_val_texture, NULL, &octave_val_rect);
		SDL_RenderCopy(renderer, octave_label_texture, NULL, &octave_label_rect);
		voice_freq_update();

		// some stuff is refactored to draw itself LOL
		musical_keyboard_draw(renderer);
		osc_options_draw(renderer);
		scope_draw(renderer);

		// draw mouse
		if (mouse_knob_grab) {
			SDL_RenderCopy(renderer, mouse_hand_closed, NULL, &mouse_cursor_rect);
		}
		else if (mouse_knob_hover) {
			SDL_RenderCopy(renderer, mouse_hand_open, NULL, &mouse_cursor_rect);
		}
		else if (keys_mouse_hover || mouse_osc_select_hover) {
			SDL_RenderCopy(renderer, mouse_hand_1_finger, NULL, &mouse_cursor_rect);
		}
		else {
			SDL_RenderCopy(renderer, mouse_pointer, NULL, &mouse_cursor_rect);
		}

		SDL_RenderPresent(renderer);


		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || 
			(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
				running = 0;
			}
			cornputer_keyboard_event_process(event);
			window_event_process(event);
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_CloseAudio();
	SDL_Quit();

	preset = fopen(preset_filename, "w+");
	for (int i = 0; i < KNOB_COUNT; i++) {
		fwrite(&knobs[i].val, sizeof(float), 1, preset);
	}
	fwrite(&osc_option_selected, sizeof(char), 1, preset);
	fclose(preset);

	return 0;
}
