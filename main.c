#include <math.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "lib/core.c"

#define NOTE_COUNT 25
#define FREQ_CENTER 440
#define SAMPLE_RATE 32000
#define KEY_MARGIN 5

int window_w = 800;
int window_h = 600;

int keys_pressed[256];

unsigned long time_counter = 0;
int notes_gate[NOTE_COUNT];

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
float note_freq[NOTE_COUNT];
float base_duty_len[NOTE_COUNT];
float base_duty_pos[NOTE_COUNT];
float thicc1_duty_pos[NOTE_COUNT];
float thicc2_duty_pos[NOTE_COUNT];
float thicc3_duty_pos[NOTE_COUNT];
float thicc4_duty_pos[NOTE_COUNT];

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
adsr_envelope filter_adsr;

typedef struct {
	float cutoff;
	float res;
} filter_data;
filter_data filter;

float volume;
float thiccness;

void note_freq_init() {
	int note_offset = -9 - 12;
	for (int i = 0; i < NOTE_COUNT; i++) {
		note_freq[i] = FREQ_CENTER * powf(1.059463, note_offset + i);
		base_duty_len[i] = SAMPLE_RATE / note_freq[i];
		base_duty_pos[i] = 0.f;
		thicc1_duty_pos[i] = 0.f;
		thicc2_duty_pos[i] = 0.f;
		thicc3_duty_pos[i] = 0.f;
		thicc4_duty_pos[i] = 0.f;
		notes_gate[i] = 0;
		amp_adsr_pos[i] = 0.f;
		amp_adsr_stage[i] = 0;
		//printf("note id: %3d    note freq: %12.8f    note duty: %12.8f \n", i, note_freq[i], note_duty_len[i]);
	}
}

float lfo_pos;

float osc_pos_buf0_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf0_r[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_r[NOTE_COUNT] = { 0.f };

int note_most_recent;
unsigned long waveform_pos = 0;
unsigned long waveform_sample_pos = 0;
#define SCOPEX 420
#define SCOPEY 100
float waveform_clean[SCOPEX] = { 0.f };
float waveform_filtered[SCOPEX] = { 0.f };

void audio_callback(void* userdata, uint8_t* byte_stream, int byte_stream_length) {
	float * float_stream = (float*) byte_stream;
	int float_stream_length = byte_stream_length >> 2;
	float osc_pos;
	float osc_pos_l, osc_pos_r;
	float feedback = filter.res + filter.res / (1.f - filter.cutoff);
	if (feedback < 1.f) feedback = 1.f;

	// figure out waveform drawing 
	float wave_length = 100.f; // hertz of sampling
	float wave_sample_length = (float) SAMPLE_RATE / wave_length; // how many samples in source
	// 200 pixels on x axis
	int wave_sample_interval = (int) (wave_sample_length / (float) SCOPEX);
	if (wave_sample_interval < 1) wave_sample_interval = 1;

	float thicc1 = 0.f;
	float thicc2 = 0.f;
	float thicc3 = 0.f;
	float thicc4 = 0.f;

	for (int i = 0; i < float_stream_length; i += 2) {
		float output_l = 0.f;
		float output_r = 0.f;

		for (int j = 0; j < NOTE_COUNT; j++) {
			// polyphony optimization!!
			if (!notes_gate[j] && amp_adsr_pos[j] == 0.f) continue;
			float amp = 0.f;
			// update duty cycle position
			base_duty_pos[j] += 1.f;
			if (base_duty_pos[j] > base_duty_len[j]) {
				base_duty_pos[j] -= base_duty_len[j];
				// scope stuff
				if (note_most_recent == j && waveform_pos >= SCOPEX) {
					waveform_pos = 0;
					waveform_sample_pos = 0;
				}
			}
			// get current waveform position
			osc_pos = osc_saw(base_duty_pos[j] / base_duty_len[j]);
			// do the thiccness
			float thicc_mod;
			thicc1_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f - thiccness * 0.04f);
			if (thicc1_duty_pos[j] >= thicc_mod) thicc1_duty_pos[j] -= thicc_mod;
			thicc1 = osc_saw(thicc1_duty_pos[j] / thicc_mod);
			thicc2_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f + thiccness * 0.004f);
			if (thicc2_duty_pos[j] >= thicc_mod) thicc2_duty_pos[j] -= thicc_mod;
			thicc2 = osc_saw(thicc2_duty_pos[j] / thicc_mod);
			thicc3_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f - thiccness * 0.004f);
			if (thicc3_duty_pos[j] >= thicc_mod) thicc3_duty_pos[j] -= thicc_mod;
			thicc3 = osc_saw(thicc3_duty_pos[j] / thicc_mod);
			thicc4_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f + thiccness * 0.04f);
			if (thicc4_duty_pos[j] >= thicc_mod) thicc4_duty_pos[j] -= thicc_mod;
			thicc4 = osc_saw(thicc4_duty_pos[j] / thicc_mod);
			/*
			osc_pos = osc_saw(fmodf((float) time_counter, note_duty_len[j]) / note_duty_len[j]);
			thicc1 = osc_saw(fmodf((float) time_counter, note_duty_len[j] * (1.f - thiccness * 0.04f)) / note_duty_len[j]);
			thicc2 = osc_saw(fmodf((float) time_counter, note_duty_len[j] * (1.f + thiccness * 0.004f)) / note_duty_len[j]);
			thicc3 = osc_saw(fmodf((float) time_counter, note_duty_len[j] * (1.f - thiccness * 0.004f)) / note_duty_len[j]);
			thicc4 = osc_saw(fmodf((float) time_counter, note_duty_len[j] * (1.f + thiccness * 0.04f)) / note_duty_len[j]);
*/
			osc_pos_l = (osc_pos + thicc1 + thicc2) / 3.f;
			osc_pos_r = (osc_pos + thicc3 + thicc4) / 3.f;

			// apply filter
			osc_pos_buf0_l[j] += filter.cutoff * (osc_pos_l - osc_pos_buf0_l[j] + feedback * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]));
			osc_pos_buf1_l[j] += filter.cutoff * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]);
			osc_pos_buf0_r[j] += filter.cutoff * (osc_pos_r - osc_pos_buf0_r[j] + feedback * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]));
			osc_pos_buf1_r[j] += filter.cutoff * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]);
//			osc_pos = osc_pos_buf1[j]; // don't overwrite source

			// apply adsr
			if (notes_gate[j]) {
				// is attack?
				if (amp_adsr_stage[j] == 0 && amp_adsr_pos[j] < 1.f) {
					amp_adsr_pos[j] += amp_adsr.attack;
				}
				// is decay?
				else if (amp_adsr_pos[j] > amp_adsr.sustain) {
					amp_adsr_stage[j] = 1;
					amp_adsr_pos[j] -= amp_adsr.decay;
				}
				// is sustain?
				else {
					amp_adsr_stage[j] = 2;
					amp_adsr_pos[j] = amp_adsr.sustain;
				}
			}
			// note is off / released?
			else if (amp_adsr_pos[j] > 0.f) {
				amp_adsr_stage[j] = 3;
				amp_adsr_pos[j] -= amp_adsr_release[j];
				if (amp_adsr_pos[j] < 0.f) amp_adsr_pos[j] = 0.f;
			}
			amp = amp_adsr_pos[j];
			amp *= amp;
			output_l += osc_pos_buf1_l[j] * amp;
			output_r += osc_pos_buf1_r[j] * amp;

			// scope waveform data
			if (note_most_recent == j) {
				if (waveform_sample_pos % wave_sample_interval == 0) {
					if (waveform_pos < SCOPEX) {
						waveform_clean[waveform_pos] = osc_pos_l * amp;
						waveform_filtered[waveform_pos] = osc_pos_buf1_l[j] * amp;
					}
					waveform_pos++;
				}
				waveform_sample_pos++;
			}
		}
		output_l *= volume;
		if (output_l > 1.f) output_l = 1.f;
		output_r *= volume;
		if (output_r > 1.f) output_r = 1.f;
		float_stream[i] = output_l;
		float_stream[i+1] = output_r;
		lfo_pos += 0.00003f;
		time_counter++;
	}
}


SDL_Texture * keyboard_texture;

SDL_Color palette[8] = {
	{ 0xf0, 0xf0, 0xdc, 0xff }, // white
	{ 0xfa, 0xc8, 0x00, 0xff }, // yellow
	{ 0x10, 0xc8, 0x40, 0xff }, // green
	{ 0x00, 0xa0, 0xc8, 0xff }, // blue
	{ 0xd2, 0x40, 0x40, 0xff }, // red
	{ 0xa0, 0x69, 0x4b, 0xff }, // brown
	{ 0x73, 0x64, 0x64, 0xff }, // grey
	{ 0x10, 0x18, 0x20, 0xff }, // black
};

#define KNOB_COUNT 8
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
	{ 0.f, 1.f, 0.f, 0.f, 0.25f, 1.25f,
		"THICC", { 708, 152, 72, 72 } },
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

void notes_update() {
	for (int i = 0; i < NOTE_COUNT; i++) {
	}
}


int main(int argc, char* args[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	note_freq_init(); 
	audio_init(SAMPLE_RATE, 2, 1024, AUDIO_F32SYS, &audio_callback);

	SDL_Event event;
	SDL_Rect window_rect = { 200, 200, window_w, window_h };
	SDL_Window * window = SDL_CreateWindow("PLINTH-SYNTH", window_rect.x, window_rect.y, window_rect.w, window_rect.h, SDL_WINDOW_RESIZABLE);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	SDL_Rect keyboard_rect = { 0, window_h / 2, window_w, window_h / 2 };
	SDL_Rect keys_white_rect = { 0, 0, 20, 100 };
	SDL_Rect keys_black_rect = { 0, 0, 10, 60 };


	// char rom initialization
	char_rom_load_set(renderer, char_rom_eagle_pc_cga);
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

	// knob texture
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	SDL_Texture * knob_texture = texture_from_image(renderer, "assets/knob2-smaller.png");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	for (int i = 0; i < KNOB_COUNT; i++) knob_init(&knobs[i]);

	// waveform texture
	SDL_Texture * waveform_texture = texture_create_generic(renderer, SCOPEX, SCOPEY);
	SDL_Rect waveform_rect = { 20, 156, SCOPEX, SCOPEY };
	SDL_Texture * wavebrush_texture = texture_from_image(renderer, "assets/brush.png");
	SDL_SetTextureBlendMode(wavebrush_texture, SDL_BLENDMODE_ADD);
	SDL_Rect wavebrush_rect = { 0, 0, 4, 4 };

	// mouse cursor
	mouse_data mouse = mouse_init();
	int mouse_keys_hover = 0;
	int mouse_keys_target = 0;
	int mouse_knob_grab = 0;
	int mouse_knob_hover = 0;
	int mouse_knob_target = 0; // knob id of grabbed knob
	char mouse_info_str[20];
	SDL_ShowCursor(SDL_DISABLE);
	SDL_Rect mouse_cursor_rect = { 0, 0, 32, 32 };
	SDL_Rect mouse_hotspot = { 0, 0, 4, 4 };
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
			if (keys_pressed[note_to_scancode[i]] || (!mouse_knob_grab && mouse_keys_hover && mouse.button_left && mouse_keys_target == i)) {
				if (notes_gate[i] == 0) {
					amp_adsr_stage[i] = 0;
					note_most_recent = i;
				}
				notes_gate[i] = 1;
			}
			else {
				if (notes_gate[i] == 1) {
					amp_adsr_stage[i] = 3;
				}
				amp_adsr_release[i] = 1.f / (amp_adsr.release * (float) SAMPLE_RATE);
				notes_gate[i] = 0;
			}
		}


		// process knobs
		mouse_knob_hover = 0;
		for (int i = 0; i < KNOB_COUNT; i++) {
			SDL_RenderCopyEx(renderer, knob_texture, NULL, &knobs[i].rect, knobs[i].rot, NULL, SDL_FLIP_NONE);
			if (collision_detection(knobs[i].rect, mouse_hotspot)) {
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

		// waveform draw
		SDL_SetRenderTarget(renderer, waveform_texture);
		renderer_set_color(renderer, &palette[7]);
		SDL_RenderClear(renderer);
		float waveform_y_scale = (float) SCOPEY * 0.4f;
		int waveform_y_offset = SCOPEY * 0.5 - 2;
		for (int x = 0; x < SCOPEX - 1; x++) {
			renderer_set_color(renderer, &palette[3]);
			SDL_RenderDrawLine(renderer, x, (int) (waveform_clean[x] * waveform_y_scale) + waveform_y_offset, x + 1, (int) (waveform_clean[x + 1] * waveform_y_scale) + waveform_y_offset);
			renderer_set_color(renderer, &palette[4]);
			SDL_RenderDrawLine(renderer, x, (int) (waveform_filtered[x] * waveform_y_scale) + waveform_y_offset, x + 1, (int) (waveform_filtered[x + 1] * waveform_y_scale) + waveform_y_offset);
		}
		for (int i = 0; i < SCOPEX; i++) {
			wavebrush_rect.x = i - 2;
			// clean
			wavebrush_rect.y = (int) (waveform_clean[i] * waveform_y_scale) + waveform_y_offset;
			texture_set_color_mod(wavebrush_texture, &palette[3]);
			SDL_RenderCopy(renderer, wavebrush_texture, NULL, &wavebrush_rect);
			// filtered
			wavebrush_rect.y = (int) (waveform_filtered[i] * waveform_y_scale) + waveform_y_offset;
			texture_set_color_mod(wavebrush_texture, &palette[4]);
			SDL_RenderCopy(renderer, wavebrush_texture, NULL, &wavebrush_rect);
		}
		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, waveform_texture, NULL, &waveform_rect);

		// process musical keyboard
		mouse_keys_hover = 0;
		keyboard_rect.w = window_rect.w;
		keyboard_rect.h = window_rect.h / 2;
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
				mouse_keys_hover = 1;
				mouse_keys_target = white_keys_to_note[i];
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
					mouse_keys_hover = 1;
					mouse_keys_target = black_keys_to_note[i];
				}
			}
		}


		// draw mouse
		if (mouse_knob_grab) {
			SDL_RenderCopy(renderer, mouse_hand_closed, NULL, &mouse_cursor_rect);
		}
		else if (mouse_knob_hover) {
			SDL_RenderCopy(renderer, mouse_hand_open, NULL, &mouse_cursor_rect);
		}
		else if (mouse_keys_hover) {
			SDL_RenderCopy(renderer, mouse_hand_1_finger, NULL, &mouse_cursor_rect);
		}
		else {
			SDL_RenderCopy(renderer, mouse_pointer, NULL, &mouse_cursor_rect);
		}
		SDL_RenderPresent(renderer);


		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;
				case SDL_KEYDOWN:
					if (keys_pressed[event.key.keysym.scancode] == 0) {
						keys_pressed[event.key.keysym.scancode] = 1;
		//				printf( "keydown: %8s %3d\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.scancode);
					}
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							running = 0;
							break;
					}
					break;
				case SDL_KEYUP:
					keys_pressed[event.key.keysym.scancode] = 0;
		//			printf( "  keyup: %8s %3d\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.scancode);
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_MOVED) {
						window_rect.x = event.window.data1;
						window_rect.y = event.window.data2;
						printf("window position changed: %d x %d\n", window_rect.x, window_rect.y);
					}
					if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						window_w = event.window.data1;
						window_h = event.window.data2;
						window_rect.w = window_w;
						window_rect.h = window_h;
						printf("window size changed: %d x %d\n", window_w, window_h);
					}
					break;
			}
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_CloseAudio();
	SDL_Quit();
	return 0;
}
