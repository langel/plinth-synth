
#define OSCOPTIONX 500
#define OSCOPTIONY 156
#define OSCOPTIONW 15 * 8
#define OSCOPTIONH 13 * 8

char osc_options[13][16] = {
	"saw raw",
	"saw blep",
	"saw 3sine",
	"saw 6sine",
	"saw 9sine",
	"square raw",
	"square blep",
	"square 3sine",
	"square 6sine",
	"square 9sine",
	"tri raw",
	"tri blep",
	"noise pitched",
};
char osc_option_selected = 1;


SDL_Texture * osc_options_texture;
SDL_Rect osc_options_rect = { OSCOPTIONX, OSCOPTIONY, OSCOPTIONW, OSCOPTIONH };

SDL_Texture * osc_option_select_texture;
SDL_Rect osc_option_select_rect = { OSCOPTIONX, OSCOPTIONY, OSCOPTIONW, 8 };


void osc_options_init(SDL_Renderer * renderer) {
	
	// oscillator option selected texture
	osc_option_select_texture = texture_create_generic(renderer, OSCOPTIONW, 8 );
	SDL_SetRenderTarget(renderer, osc_option_select_texture);
	renderer_set_color(renderer, &palette[7]);
	SDL_RenderClear(renderer);

	// create view of all oscillator type options
	osc_options_texture = texture_create_generic(renderer, OSCOPTIONW, OSCOPTIONH);
	SDL_SetTextureBlendMode(osc_options_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, osc_options_texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	char_rom_set_color(&palette[0]);
	SDL_Rect char_rect = { 0, 0, 8, 8 };
	SDL_Rect dest_rect = { 0, 0, 8, 8 };
	for (int i = 0; i < 13; i++) {
		dest_rect.y = i * 8;
		int string_length = strlen(osc_options[i]);
		for (int j = 0; j < string_length; j++) {
			dest_rect.x = j * 8;
			char_rect.x = osc_options[i][j] * 8;
			SDL_RenderCopy(renderer, char_rom_texture, &char_rect, &dest_rect);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
}


void osc_options_draw(SDL_Renderer * renderer) {
	SDL_SetRenderTarget(renderer, NULL);
	osc_option_select_rect.y = OSCOPTIONY + osc_option_selected * 8;
	SDL_RenderCopy(renderer, osc_option_select_texture, NULL, &osc_option_select_rect);
	SDL_RenderCopy(renderer, osc_options_texture, NULL, &osc_options_rect);
}
