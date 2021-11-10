
int collision_detection(SDL_Rect a, SDL_Rect b) {
	if (a.x + a.w < b.x) return 0;
	if (a.x > b.x + b.w) return 0;
	if (a.y + a.h < b.y) return 0;
	if (a.y > b.y + b.h) return 0;
	return 1;
}

float curve_apply(float val, float curve) {
	return powf(val, curve);
}

float curve_get(float min, float max, float mid) {
	return log(0.5f) / log((mid-min)/(max-min));
}

float curve_inverse(float curve) {
	return 1.f / curve;
}

void renderer_set_color(SDL_Renderer * renderer, SDL_Color * color) {
	SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
}

float sine_to_pos(float sine) {
	return (sin(sine) * 0.5f) + 0.5f;
}

SDL_Texture * texture_create_generic(SDL_Renderer * renderer, int w, int h) {
	return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
}

SDL_Texture * texture_from_image(SDL_Renderer * renderer, char * path) {
	SDL_Surface * surface = IMG_Load(path);
	if (!surface) {
		printf("Failed to load image at %s: %s\n", path, SDL_GetError());
//		return 1;
	}
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
}

void texture_set_color_mod(SDL_Texture * texture, SDL_Color * color) {
	SDL_SetTextureColorMod(texture, color->r, color->g, color->b);
}

float value_to_range_pos(float min, float max, float val) {
	return (val - min) / (max - min);
}

