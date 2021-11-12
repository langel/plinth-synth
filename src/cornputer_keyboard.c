int keys_pressed[256];

int keys_debug = 0;

int cornputer_keyboard_get_layout() {
	/*
		getting keys from scancodes for Q and W and Y
		1 qwerty = 'qwy'
		2 dvorak = 'q,f' or "',f"
		3 colemak = 'qwj'
		4 qwertz = 'qwz'
		5 azerty = 'azy'
		6 bépo = 'b\xe9y'
		unknown == ?????
	*/
	char layout[8];
	int layout_id = 0;
	if (SDL_GetKeyFromScancode(SDL_SCANCODE_Q) == 'a') {
		layout_id = 5;
	}
	else if (SDL_GetKeyFromScancode(SDL_SCANCODE_W) == ',') {
		layout_id = 2;
	}
	else if (SDL_GetKeyFromScancode(SDL_SCANCODE_W) == 0xe9) {
		layout_id = 6;
	}
	else if (SDL_GetKeyFromScancode(SDL_SCANCODE_Y) == 'y') {
		layout_id = 1;
	}
	else if (SDL_GetKeyFromScancode(SDL_SCANCODE_Y) == 'z') {
		layout_id = 4;
	}
	else if (SDL_GetKeyFromScancode(SDL_SCANCODE_Y) == 'j') {
		layout_id = 3;
	}
	return layout_id;
}


void cornputer_keyboard_event_process(SDL_Event event) {
	int scancode = event.key.keysym.scancode;
	int sym = event.key.keysym.sym;
	int keycode = (sym < 256) ? sym : -1;
	if (event.type == SDL_KEYDOWN) {
		if (keys_debug && keys_pressed[scancode] == 0) printf("keydown: %12s %3d %3d\n", SDL_GetKeyName(sym), keycode, scancode);
		keys_pressed[scancode] = 1;
		
		if (keys_debug >= 3) printf("%d\n", cornputer_keyboard_get_layout());
	}
	if (event.type == SDL_KEYUP) {
		keys_pressed[scancode] = 0;
		if (keys_debug >= 2) printf("  keyup: %12s %3d %3d\n", SDL_GetKeyName(sym), keycode, scancode);
	}
}

