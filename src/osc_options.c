
#define OSCOPTIONX 500
#define OSCOPTIONY 156
#define OSCOPTIONW 15 * 8
#define OSCOPTIONH 13 * 8

#define osc_option_string_max_length 16
//#define osc_option_count 2

enum osc_options {
	arcade_32_0,
	arcade_32_1,
	arcade_32_2,
	arcade_32_3,
	arcade_32_4,
	arcade_32_5,
	arcade_32_6,
	arcade_32_7,
	arcade_32_1_os,
	arcade_32_2_os,
	noise_pitched,
	saw_raw,
	saw_blep,
	saw_3sine,
	saw_6sine,
	saw_9sine,
	sine_raw,
	sine_ym_lut,
	square_raw,
	square_blep,
	square_3sine,
	square_6sine,
	square_9sine,
	tri_raw,
	tri_blep,
};

enum osc_options osc_option_selected = saw_blep;

int osc_visible_options[] = {
	saw_raw,
	saw_blep,
	square_blep,
	square_6sine,
	tri_blep,
	sine_raw,
	sine_ym_lut,
	arcade_32_1,
	arcade_32_1_os,
	arcade_32_2,
	arcade_32_2_os,
	arcade_32_3,
	noise_pitched,
};

int osc_option_count;

SDL_Texture * osc_options_texture;
SDL_Rect osc_options_rect = { OSCOPTIONX, OSCOPTIONY, OSCOPTIONW, OSCOPTIONH };

SDL_Texture * osc_option_select_texture;
SDL_Rect osc_option_select_rect = { OSCOPTIONX, OSCOPTIONY, OSCOPTIONW, 8 };


void osc_options_init(SDL_Renderer * renderer) {

	// oscillator options and strings
	static const char osc_opt_str[][osc_option_string_max_length] = {
		[arcade_32_0] = "arcade wave 00",
		[arcade_32_1] = "arcade wave 01",
		[arcade_32_2] = "arcade wave 02",
		[arcade_32_3] = "arcade wave 03",
		[arcade_32_4] = "arcade wave 04",
		[arcade_32_5] = "arcade wave 05",
		[arcade_32_6] = "arcade wave 06",
		[arcade_32_7] = "arcade wave 07",
		[arcade_32_1_os] = "arcade 01 overs",
		[arcade_32_2_os] = "arcade 02 overs",
		[noise_pitched] = "noise pitched",
		[saw_raw] = "sawtooth raw",
		[saw_blep] = "sawtooth blep",
		[saw_3sine] = "sawtooth 3 sines",
		[saw_6sine] = "sawtooth 6 sines",
		[saw_9sine] = "sawtooth 9 sines",
		[sine_raw] = "sine raw",
		[sine_ym_lut] = "sine lut 1024",
		[square_raw] = "sqaure raw",
		[square_blep] = "square blep",
		[square_3sine] = "square 3 sines",
		[square_6sine] = "square 6 sines",
		[square_9sine] = "square 9 sines",
		[tri_raw] = "triangle_raw",
		[tri_blep] = "triangle blep",
	};
	
	// oscillator option selected texture
	osc_option_select_texture = texture_create_generic(renderer, OSCOPTIONW, 8 );
	SDL_SetRenderTarget(renderer, osc_option_select_texture);
	renderer_set_color(renderer, &palette[7]);
	SDL_RenderClear(renderer);

	// number of visible options affects height
	osc_option_count = sizeof(osc_visible_options) / sizeof(osc_visible_options[0]);
	osc_options_rect.h = osc_option_count * 8;	
	// create view of all oscillator type options
	osc_options_texture = texture_create_generic(renderer, OSCOPTIONW, osc_options_rect.h);
	SDL_SetTextureBlendMode(osc_options_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, osc_options_texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	char_rom_set_color(&palette[0]);
	SDL_Rect char_rect = { 0, 0, 8, 8 };
	SDL_Rect dest_rect = { 0, 0, 8, 8 };
	for (int i = 0; i < osc_option_count; i++) {
		dest_rect.y = i * 8;
		int string_length = strlen(osc_opt_str[osc_visible_options[i]]);
		for (int j = 0; j < string_length; j++) {
			dest_rect.x = j * 8;
			char_rect.x = osc_opt_str[osc_visible_options[i]][j] * 8;
			SDL_RenderCopy(renderer, char_rom_texture, &char_rect, &dest_rect);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
}


void osc_options_draw(SDL_Renderer * renderer) {
	SDL_SetRenderTarget(renderer, NULL);
	int osc_index = 0;
	while (osc_option_selected != osc_visible_options[osc_index]) osc_index++;
	osc_option_select_rect.y = OSCOPTIONY + osc_index * 8;
	SDL_RenderCopy(renderer, osc_option_select_texture, NULL, &osc_option_select_rect);
	SDL_RenderCopy(renderer, osc_options_texture, NULL, &osc_options_rect);
}
