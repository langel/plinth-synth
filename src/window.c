
int window_w = 800;
int window_h = 600;

int window_has_focus = 0;

SDL_Rect window_rect;
SDL_Window * window;

void window_init() {
	window_rect.x = 200;
	window_rect.y = 200;
	window_rect.w = window_w;
	window_rect.h = window_h;
	window = SDL_CreateWindow("PLINTH-SYNTH", window_rect.x, window_rect.y, window_rect.w, window_rect.h, SDL_WINDOW_RESIZABLE);
}


void window_event_process(SDL_Event event) {
	int flags = SDL_GetWindowFlags(window);
	window_has_focus = flags & SDL_WINDOW_INPUT_FOCUS;
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
}
