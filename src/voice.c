

typedef struct {
	float phase;
	float inc;
} voice_osc;

typedef struct {
	voice_osc base;
	voice_osc thicc1;
	voice_osc thicc2;
	voice_osc thicc3;
	voice_osc thicc4;
	int gate;
	int adsr_stage;
	float adsr_pos;
	int osc_state[5];
} voice;


voice voices[NOTE_COUNT];


void voice_freq_init() {
	for (int i = 0; i < NOTE_COUNT; i++) {
		voices[i].gate = 0;
		voices[i].base.phase = 0.f;
		voices[i].base.inc = 0.001f;
		voices[i].thicc1.phase = 0.f;
		voices[i].thicc1.inc = 0.001f;
		voices[i].thicc2.phase = 0.f;
		voices[i].thicc2.inc = 0.001f;
		voices[i].thicc3.phase = 0.f;
		voices[i].thicc3.inc = 0.001f;
		voices[i].thicc4.phase = 0.f;
		voices[i].thicc4.inc = 0.001f;
		for (int j = 0; j < 5; j++) {
			voices[i].osc_state[j] = 1;
		}

		// XXX the olds
		amp_adsr_pos[i] = 0.f;
		amp_adsr_stage[i] = 0;
	}
}

void voice_freq_update() {
	int base_semitone = octave * 12 - 48 + BASE_NOTE;
	for (int i = 0; i < NOTE_COUNT; i++) {
		float note_freq = FREQ_CENTER * powf(1.059463, (float) (base_semitone + i));
		float note_duty = SAMPLE_RATE / note_freq;
		float note_inc = 1 / note_duty;
		voices[i].base.inc = note_inc;
		//printf("note id: %3d    note freq: %12.8f    note duty: %12.8f    note inc: %12.8f \n", i, note_freq, note_duty, note_inc);
	}
}
