void audio_spec_log(SDL_AudioSpec *as) {
	logout(
		" freq______%5d\n"
		" format____%5d\n"
		" channels__%5d\n"
		" silence___%5d\n"
		" samples___%5d\n"
		" size______%5d\n",
		(int) as->freq,
		(int) as->format,
		(int) as->channels,
		(int) as->silence,
		(int) as->samples,
		(int) as->size
	);
}

SDL_AudioSpec audio_spec;
SDL_AudioSpec audio_actual;
SDL_AudioDeviceID audio_device;

void audio_init(int sample_rate, int channels, int buffer_sample_count, SDL_AudioFormat audio_format, void (*callback)()) {
	audio_spec.freq = sample_rate;
	audio_spec.format = audio_format;
	audio_spec.channels = channels;
	audio_spec.samples = buffer_sample_count;
	audio_spec.callback = callback;
	audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, &audio_actual, 0);
	if (audio_device == 0) {
		sdl_debug("FAILED TO OPEN AUDIO");
		audio_spec_log(&audio_spec);
	}
	else {
		logout("device initialized\n");
		audio_spec_log(&audio_actual);
		SDL_PauseAudioDevice(audio_device, 0);
	}
}


