int keys_pressed[256];

int keys_debug = 0;

#define KB_LAYOUT_UNKNOWN 0x0000
#define KB_LAYOUT_QWERTY  0x0001  // qwy
#define KB_LAYOUT_DVORAK  0x0002  // ',f
#define KB_LAYOUT_COLEMAK 0x0004  // qwj
#define KB_LAYOUT_QWERTZ  0x0008  // qwz
#define KB_LAYOUT_AZERTY  0x0010  // azy
#define KB_LAYOUT_BEPO    0x0020  // béy

int cornputer_keyboard_get_layout() {
	/*
		detect keyboard layout based on OS return characters
		from Q, W, and Y qwerty scancode positions
	*/
	char q = SDL_GetKeyFromScancode(SDL_SCANCODE_Q);
	if (q == 'a') return KB_LAYOUT_AZERTY;
	char w = SDL_GetKeyFromScancode(SDL_SCANCODE_W);
	if (w == ',') return KB_LAYOUT_DVORAK;
	if (w == 0xe9) return KB_LAYOUT_BEPO;
	char y = SDL_GetKeyFromScancode(SDL_SCANCODE_Y);
	if (y == 'y') return KB_LAYOUT_QWERTY;
	if (y == 'z') return KB_LAYOUT_QWERTZ;
	if (y == 'j') return KB_LAYOUT_COLEMAK;
	return KB_LAYOUT_UNKNOWN;
}


void cornputer_keyboard_event_process(SDL_Event event) {
	int scancode = event.key.keysym.scancode;
	int sym = event.key.keysym.sym;
	int keycode = (sym < 256) ? sym : -1;
	if (event.type == SDL_KEYDOWN) {
		if (keys_debug && keys_pressed[scancode] == 0) {
			printf("keydown: %12s %3d %3d\n", SDL_GetKeyName(sym), keycode, scancode);
			if (keys_debug >= 3) printf("%d\n", cornputer_keyboard_get_layout());
		}
		keys_pressed[scancode] = 1;
		
	}
	if (event.type == SDL_KEYUP) {
		keys_pressed[scancode] = 0;
		if (keys_debug >= 2) printf("  keyup: %12s %3d %3d\n", SDL_GetKeyName(sym), keycode, scancode);
	}
}

