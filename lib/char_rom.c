#include <string.h>
#include "./char_roms/eagle_pc_cga.c"

// XXX gonna try a string 8x8 pixel schema for now
// all char roms must have 256 characters

SDL_Texture * char_rom_texture;

void char_rom_load_set(SDL_Renderer * renderer, unsigned char * set_array) {
	SDL_Texture * stashed_texture = SDL_GetRenderTarget(renderer);
	SDL_DestroyTexture(char_rom_texture);
	int data_length = 256 * 8;
	char_rom_texture = texture_create_generic(renderer, data_length, 8);
	SDL_SetTextureBlendMode(char_rom_texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, char_rom_texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int i = 0; i < data_length; i++) {
		unsigned char byte = set_array[i];
		unsigned char y = i % 8;
		for (unsigned char x = 0; x < 8; x++) {
			if ((1 << (7 - x)) & byte) {
				SDL_RenderDrawPoint(renderer, i - y + x, y);
			}
		}
	}
	SDL_SetRenderTarget(renderer, stashed_texture);
}

void char_rom_set_color(SDL_Color * color) {
	SDL_SetTextureColorMod(char_rom_texture, color->r, color->g, color->b);
}

void char_rom_string_to_texture(SDL_Renderer * renderer, SDL_Texture * target, char * string) {
	SDL_Texture * stashed_texture = SDL_GetRenderTarget(renderer);
	SDL_SetRenderTarget(renderer, target);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_Rect src = { 0, 0, 8, 8 };
	SDL_Rect dest = { 0, 0, 8, 8 };
	int string_length = strlen(string);
	for (int i = 0; i < string_length; i++) {
		src.x = string[i] * 8;
		SDL_RenderCopy(renderer, char_rom_texture, &src, &dest);
		dest.x += 8;
	}
	SDL_SetRenderTarget(renderer, stashed_texture);
}

