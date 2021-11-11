

float osc_pos_buf0_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_l[NOTE_COUNT] = { 0.f };
float osc_pos_buf0_r[NOTE_COUNT] = { 0.f };
float osc_pos_buf1_r[NOTE_COUNT] = { 0.f };



void audio_callback(void* userdata, uint8_t* byte_stream, int byte_stream_length) {
	float * float_stream = (float*) byte_stream;
	int float_stream_length = byte_stream_length >> 2;
	float osc_pos;
	float osc_pos_l, osc_pos_r;
	float feedback = filter.res + filter.res / (1.f - filter.cutoff);
	if (feedback < 1.f) feedback = 1.f;

	// figure out waveform drawing 
	float wave_length = 100.f; // hertz of sampling
	float wave_sample_length = (float) SAMPLE_RATE / wave_length; // how many samples in source
	// 200 pixels on x axis
	int wave_sample_interval = (int) (wave_sample_length / (float) SCOPEX);
	if (wave_sample_interval < 1) wave_sample_interval = 1;

	float thicc1 = 0.f;
	float thicc2 = 0.f;
	float thicc3 = 0.f;
	float thicc4 = 0.f;

	for (int i = 0; i < float_stream_length; i += 2) {
		float output_l = 0.f;
		float output_r = 0.f;

		for (int j = 0; j < NOTE_COUNT; j++) {
			// polyphony optimization!!
			if (!notes_gate[j] && amp_adsr_pos[j] == 0.f) continue;
			float amp = 0.f;
			// update duty cycle position
			base_duty_pos[j] += 1.f;
			if (base_duty_pos[j] > base_duty_len[j]) {
				base_duty_pos[j] -= base_duty_len[j];
				// scope stuff
				if (note_most_recent == j && waveform_pos >= SCOPEX) {
					waveform_pos = 0;
					waveform_sample_pos = 0;
				}
			}
			// get current waveform position
			/*
			osc_pos = osc_saw(base_duty_pos[j] / base_duty_len[j]);
			// do the thiccness
			float thicc_mod;
			thicc1_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f - thiccness * 0.05f);
			if (thicc1_duty_pos[j] >= thicc_mod) thicc1_duty_pos[j] -= thicc_mod;
			thicc1 = osc_saw(thicc1_duty_pos[j] / thicc_mod);
			thicc2_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f + thiccness * 0.005f);
			if (thicc2_duty_pos[j] >= thicc_mod) thicc2_duty_pos[j] -= thicc_mod;
			thicc2 = osc_saw(thicc2_duty_pos[j] / thicc_mod);
			thicc3_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f - thiccness * 0.005f);
			if (thicc3_duty_pos[j] >= thicc_mod) thicc3_duty_pos[j] -= thicc_mod;
			thicc3 = osc_saw(thicc3_duty_pos[j] / thicc_mod);
			thicc4_duty_pos[j]++;
			thicc_mod = base_duty_len[j] * (1.f + thiccness * 0.05f);
			if (thicc4_duty_pos[j] >= thicc_mod) thicc4_duty_pos[j] -= thicc_mod;
			thicc4 = osc_saw(thicc4_duty_pos[j] / thicc_mod);
			*/
			osc_pos = osc_saw(fmodf((float) time_counter, base_duty_len[j]) / base_duty_len[j]);
			thicc1 = osc_saw(fmodf((float) time_counter, base_duty_len[j] * (1.f - thiccness * 0.04f)) / base_duty_len[j]);
			thicc2 = osc_saw(fmodf((float) time_counter, base_duty_len[j] * (1.f + thiccness * 0.004f)) / base_duty_len[j]);
			thicc3 = osc_saw(fmodf((float) time_counter, base_duty_len[j] * (1.f - thiccness * 0.004f)) / base_duty_len[j]);
			thicc4 = osc_saw(fmodf((float) time_counter, base_duty_len[j] * (1.f + thiccness * 0.04f)) / base_duty_len[j]);

			osc_pos_l = (osc_pos + thicc1 + thicc2) / 3.f;
			osc_pos_r = (osc_pos + thicc3 + thicc4) / 3.f;

			// apply filter
			osc_pos_buf0_l[j] += filter.cutoff * (osc_pos_l - osc_pos_buf0_l[j] + feedback * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]));
			osc_pos_buf1_l[j] += filter.cutoff * (osc_pos_buf0_l[j] - osc_pos_buf1_l[j]);
			osc_pos_buf0_r[j] += filter.cutoff * (osc_pos_r - osc_pos_buf0_r[j] + feedback * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]));
			osc_pos_buf1_r[j] += filter.cutoff * (osc_pos_buf0_r[j] - osc_pos_buf1_r[j]);
//			osc_pos = osc_pos_buf1[j]; // don't overwrite source

			// apply adsr
			if (notes_gate[j]) {
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
				if (waveform_sample_pos % wave_sample_interval == 0) {
					if (waveform_pos < SCOPEX) {
						waveform_clean[waveform_pos] = osc_pos_l * amp;
						waveform_filtered[waveform_pos] = osc_pos_buf1_l[j] * amp;
					}
					waveform_pos++;
				}
				waveform_sample_pos++;
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
