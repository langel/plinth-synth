

#define SCOPEX 420
#define SCOPEY 100
float scope_clean[SCOPEX] = { 0.f };
float scope_filtered[SCOPEX] = { 0.f };
unsigned long scope_pos = 0;
unsigned long scope_sample_pos = 0;
	
SDL_Rect scope_rect = { 20, 156, SCOPEX, SCOPEY };
SDL_Rect scope_brush_rect = { 0, 0, 4, 4 };

SDL_Texture * scope_texture;
SDL_Texture * scope_brush_texture;

void scope_init(SDL_Renderer * renderer) {
	scope_texture = texture_create_generic(renderer, SCOPEX, SCOPEY);
	scope_brush_texture = texture_from_image(renderer, "assets/brush.png");
	SDL_SetTextureBlendMode(scope_brush_texture, SDL_BLENDMODE_ADD);
}


void scope_draw(SDL_Renderer * renderer) {
	SDL_SetRenderTarget(renderer, scope_texture);
	renderer_set_color(renderer, &palette[7]);
	SDL_RenderClear(renderer);
	float scope_y_scale = (float) SCOPEY * 0.4f;
	int scope_y_offset = SCOPEY * 0.5 - 2;
	for (int x = 0; x < SCOPEX - 1; x++) {
		scope_brush_rect.x = x - 2;
		// clean line
		renderer_set_color(renderer, &palette[3]);
		SDL_RenderDrawLine(renderer, x, (int) (scope_clean[x] * scope_y_scale) + scope_y_offset, x + 1, (int) (scope_clean[x + 1] * scope_y_scale) + scope_y_offset);
		// clean dots
		scope_brush_rect.y = (int) (scope_clean[x] * scope_y_scale) + scope_y_offset;
		texture_set_color_mod(scope_brush_texture, &palette[3]);
		SDL_RenderCopy(renderer, scope_brush_texture, NULL, &scope_brush_rect);
		// filtered line
		renderer_set_color(renderer, &palette[4]);
		SDL_RenderDrawLine(renderer, x, (int) (scope_filtered[x] * scope_y_scale) + scope_y_offset, x + 1, (int) (scope_filtered[x + 1] * scope_y_scale) + scope_y_offset);
		// filtered dots
		scope_brush_rect.y = (int) (scope_filtered[x] * scope_y_scale) + scope_y_offset;
		texture_set_color_mod(scope_brush_texture, &palette[4]);
		SDL_RenderCopy(renderer, scope_brush_texture, NULL, &scope_brush_rect);
	}
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, scope_texture, NULL, &scope_rect);
}
