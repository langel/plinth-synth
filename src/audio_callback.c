

float osc_pos_buf0_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf0_r[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_r[NOTE_COUNT] = { 0.f };


void audio_callback(void* userdata, uint8_t* byte_stream, int byte_stream_length) {
	float * float_stream = (float*) byte_stream;
	int float_stream_length = byte_stream_length >> 2;
	float osc_pos_l, osc_pos_r;
	float feedback = filter.res + filter.res / (1.f - filter.cutoff);
	if (feedback < 1.f) feedback = 1.f;

	// figure out waveform drawing 
	float wave_length = 100.f; // hertz of sampling
	float wave_sample_length = (float) SAMPLE_RATE / wave_length; // how many samples in source
	// 200 pixels on x axis
	int wave_sample_interval = (int) (wave_sample_length / (float) SCOPEX);
	if (wave_sample_interval < 1) wave_sample_interval = 1;

	float base = 0.f;
	float thicc1 = 0.f;
	float thicc2 = 0.f;
	float thicc3 = 0.f;
	float thicc4 = 0.f;

	// calculate thiccnesses increments for buffer duration
	for (int i = 0; i < NOTE_COUNT; i++) {
		voices[i].thicc1.inc = voices[i].base.inc * (1.f - thiccness * 0.05f);
		voices[i].thicc2.inc = voices[i].base.inc * (1.f + thiccness * 0.005f);
		voices[i].thicc3.inc = voices[i].base.inc * (1.f - thiccness * 0.005f);
		voices[i].thicc4.inc = voices[i].base.inc * (1.f + thiccness * 0.05f);
	}

	for (int i = 0; i < float_stream_length; i += 2) {
		float output_l = 0.f;
		float output_r = 0.f;

		for (int j = 0; j < NOTE_COUNT; j++) {
			// polyphony optimization!!
			if (!voices[j].gate && amp_adsr_pos[j] == 0.f) continue;
			float amp = 0.f;

			// oscillators!!!
			voices[j].base.phase += voices[j].base.inc;
			if (voices[j].base.phase > 1.f) {
				voices[j].base.phase -= 1.f;
				// scope stuff
				if (note_most_recent == j && scope_pos >= SCOPEX) {
					scope_pos = 0;
					scope_sample_pos = 0;
				}
			}
			voices[j].thicc1.phase += voices[j].thicc1.inc;
			if (voices[j].thicc1.phase > 1.f) voices[j].thicc1.phase -= 1.f;
			voices[j].thicc2.phase += voices[j].thicc2.inc;
			if (voices[j].thicc2.phase > 1.f) voices[j].thicc2.phase -= 1.f;
			voices[j].thicc3.phase += voices[j].thicc3.inc;
			if (voices[j].thicc3.phase > 1.f) voices[j].thicc3.phase -= 1.f;
			voices[j].thicc4.phase += voices[j].thicc4.inc;
			if (voices[j].thicc4.phase > 1.f) voices[j].thicc4.phase -= 1.f;

			if (osc_option_selected == 0) {
				base   = osc_saw(voices[j].base.phase);
				thicc1 = osc_saw(voices[j].thicc1.phase);
				thicc2 = osc_saw(voices[j].thicc2.phase);
				thicc3 = osc_saw(voices[j].thicc3.phase);
				thicc4 = osc_saw(voices[j].thicc4.phase);
			}
			else if (osc_option_selected == 1) {
				base   = osc_saw_blep(voices[j].base.phase, voices[j].base.inc);
				thicc1 = osc_saw_blep(voices[j].thicc1.phase, voices[j].thicc1.inc);
				thicc2 = osc_saw_blep(voices[j].thicc2.phase, voices[j].thicc2.inc);
				thicc3 = osc_saw_blep(voices[j].thicc3.phase, voices[j].thicc3.inc);
				thicc4 = osc_saw_blep(voices[j].thicc4.phase, voices[j].thicc4.inc);
			}
			else if (osc_option_selected == 2) {
				base   = osc_saw_from_sines(voices[j].base.phase,   3);
				thicc1 = osc_saw_from_sines(voices[j].thicc1.phase, 3);
				thicc2 = osc_saw_from_sines(voices[j].thicc2.phase, 3);
				thicc3 = osc_saw_from_sines(voices[j].thicc3.phase, 3);
				thicc4 = osc_saw_from_sines(voices[j].thicc4.phase, 3);
			}
			else if (osc_option_selected == 3) {
				base   = osc_saw_from_sines(voices[j].base.phase,   6);
				thicc1 = osc_saw_from_sines(voices[j].thicc1.phase, 6);
				thicc2 = osc_saw_from_sines(voices[j].thicc2.phase, 6);
				thicc3 = osc_saw_from_sines(voices[j].thicc3.phase, 6);
				thicc4 = osc_saw_from_sines(voices[j].thicc4.phase, 6);
			}
			else if (osc_option_selected == 4) {
				base   = osc_saw_from_sines(voices[j].base.phase,   9);
				thicc1 = osc_saw_from_sines(voices[j].thicc1.phase, 9);
				thicc2 = osc_saw_from_sines(voices[j].thicc2.phase, 9);
				thicc3 = osc_saw_from_sines(voices[j].thicc3.phase, 9);
				thicc4 = osc_saw_from_sines(voices[j].thicc4.phase, 9);
			}
			else if (osc_option_selected == 5) {
				base   = osc_square(voices[j].base.phase);
				thicc1 = osc_square(voices[j].thicc1.phase);
				thicc2 = osc_square(voices[j].thicc2.phase);
				thicc3 = osc_square(voices[j].thicc3.phase);
				thicc4 = osc_square(voices[j].thicc4.phase);
			}
			else if (osc_option_selected == 6) {
				base   = osc_square_blep(voices[j].base.phase, voices[j].base.inc);
				thicc1 = osc_square_blep(voices[j].thicc1.phase, voices[j].thicc1.inc);
				thicc2 = osc_square_blep(voices[j].thicc2.phase, voices[j].thicc2.inc);
				thicc3 = osc_square_blep(voices[j].thicc3.phase, voices[j].thicc3.inc);
				thicc4 = osc_square_blep(voices[j].thicc4.phase, voices[j].thicc4.inc);
			}
			else if (osc_option_selected == 7) {
				base   = osc_square_from_sines(voices[j].base.phase,   3);
				thicc1 = osc_square_from_sines(voices[j].thicc1.phase, 3);
				thicc2 = osc_square_from_sines(voices[j].thicc2.phase, 3);
				thicc3 = osc_square_from_sines(voices[j].thicc3.phase, 3);
				thicc4 = osc_square_from_sines(voices[j].thicc4.phase, 3);
			}
			else if (osc_option_selected == 8) {
				base   = osc_square_from_sines(voices[j].base.phase,   6);
				thicc1 = osc_square_from_sines(voices[j].thicc1.phase, 6);
				thicc2 = osc_square_from_sines(voices[j].thicc2.phase, 6);
				thicc3 = osc_square_from_sines(voices[j].thicc3.phase, 6);
				thicc4 = osc_square_from_sines(voices[j].thicc4.phase, 6);
			}
			else if (osc_option_selected == 9) {
				base   = osc_square_from_sines(voices[j].base.phase,   9);
				thicc1 = osc_square_from_sines(voices[j].thicc1.phase, 9);
				thicc2 = osc_square_from_sines(voices[j].thicc2.phase, 9);
				thicc3 = osc_square_from_sines(voices[j].thicc3.phase, 9);
				thicc4 = osc_square_from_sines(voices[j].thicc4.phase, 9);
			}
			else if (osc_option_selected == 10) {
				base   = osc_triangle(voices[j].base.phase);
				thicc1 = osc_triangle(voices[j].thicc1.phase);
				thicc2 = osc_triangle(voices[j].thicc2.phase);
				thicc3 = osc_triangle(voices[j].thicc3.phase);
				thicc4 = osc_triangle(voices[j].thicc4.phase);
			}
			else if (osc_option_selected == 11) {
				base   = osc_triangle_blep(voices[j].base.phase, voices[j].base.inc);
				thicc1 = osc_triangle_blep(voices[j].thicc1.phase, voices[j].thicc1.inc);
				thicc2 = osc_triangle_blep(voices[j].thicc2.phase, voices[j].thicc2.inc);
				thicc3 = osc_triangle_blep(voices[j].thicc3.phase, voices[j].thicc3.inc);
				thicc4 = osc_triangle_blep(voices[j].thicc4.phase, voices[j].thicc4.inc);
			}
			else if (osc_option_selected == 12) {
				base   = osc_noise_pitched(voices[j].base.phase, voices[j].base.inc, &voices[j].osc_state[0]);
				thicc1 = osc_noise_pitched(voices[j].thicc1.phase, voices[j].thicc1.inc, &voices[j].osc_state[1]);
				thicc2 = osc_noise_pitched(voices[j].thicc2.phase, voices[j].thicc2.inc, &voices[j].osc_state[2]);
				thicc3 = osc_noise_pitched(voices[j].thicc3.phase, voices[j].thicc3.inc, &voices[j].osc_state[3]);
				thicc4 = osc_noise_pitched(voices[j].thicc4.phase, voices[j].thicc4.inc, &voices[j].osc_state[4]);
			}

			osc_pos_l = (base + (thicc1 + thicc2) * thiccness) / (1.f + 1.f * thiccness);
			osc_pos_r = (base + (thicc3 + thicc4) * thiccness) / (1.f + 1.f * thiccness);

			// apply filter
			osc_pos_buf0_l[j] += filter.cutoff * (osc_pos_l - osc_pos_buf0_l[j] + feedback * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]));
			osc_pos_buf1_l[j] += filter.cutoff * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]);
			osc_pos_buf0_r[j] += filter.cutoff * (osc_pos_r - osc_pos_buf0_r[j] + feedback * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]));
			osc_pos_buf1_r[j] += filter.cutoff * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]);

			// apply adsr
			if (voices[j].gate) {
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
				if (scope_sample_pos % wave_sample_interval == 0) {
					if (scope_pos < SCOPEX) {
						// invert y value for grafx
						scope_clean[scope_pos] = -osc_pos_l * amp;
						scope_filtered[scope_pos] = -osc_pos_buf1_l[j] * amp;
					}
					scope_pos++;
				}
				scope_sample_pos++;
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
