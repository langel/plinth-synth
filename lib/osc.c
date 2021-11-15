
const float osc_tao = M_PI * 2;

float osc_helper_blep(float phase, float inc) {
	float t = phase;
	if (t < inc) {
		t /= inc;
		return t + t - t * t - 1.f;
	}
	else if (t > 1.f - inc) {
		t = (t - 1.f) / inc;
		return t * t + t + t + 1.f;
	}
	return 0.f;
}

float osc_noise_white() {
	return (((float)rand()/(float)(RAND_MAX)) * 2.f) - 1.f;
}

float osc_noise_brown() {
	static float b = 0.f;
	b += osc_noise_white() * 0.1;
	if (b > 1.f) b = 1.f;
	else if (b < -1.f) b = -1.f;
	return b;
}

float osc_noise_pink_filtered() {
	// pink noise algorithms from https://www.firstpr.com.au/dsp/pink-noise/
	static float b0 = 0.f;
	static float b1 = 0.f;
	static float b2 = 0.f;
	float white = osc_noise_white();
	b0 = 0.99765 * b0 + white * 0.0990460;
	b1 = 0.96300 * b1 + white * 0.2965164;
	b2 = 0.57000 * b2 + white * 1.0526913;
	return b0 + b1 + b2 + white * 0.1848;
}

float osc_noise_pink_filtered_more() {
	static float b0 = 0.f;
	static float b1 = 0.f;
	static float b2 = 0.f;
	static float b3 = 0.f;
	static float b4 = 0.f;
	static float b5 = 0.f;
	static float b6 = 0.f;
	float white = osc_noise_white();
	b0 = 0.99886 * b0 + white * 0.0555179;
	b1 = 0.99332 * b1 + white * 0.0750759;
	b2 = 0.96900 * b2 + white * 0.1538520;
	b3 = 0.86650 * b3 + white * 0.3104856;
	b4 = 0.55000 * b4 + white * 0.5329522;
	b5 = -0.7616 * b5 - white * 0.0168980;
	float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
	b6 = white * 0.115926;
	return pink;
}

float osc_noise_pink_stacked() {
	// a bit expensive but sounds great
	static int counter;
	static float o1 = 0.f;
	static float o2 = 0.f;
	static float o3 = 0.f;
	static float o4 = 0.f;
	static float o5 = 0.f;
	static float o6 = 0.f;
	static float o7 = 0.f;
	static float o8 = 0.f;
	if (counter & 0x01) o1 = osc_noise_white();
	else if (counter & 0x02) o2 = osc_noise_white();
	else if (counter & 0x04) o3 = osc_noise_white();
	else if (counter & 0x08) o4 = osc_noise_white();
	else if (counter & 0x10) o5 = osc_noise_white();
	else if (counter & 0x20) o6 = osc_noise_white();
	else if (counter & 0x40) o7 = osc_noise_white();
	else if (counter & 0x80) o8 = osc_noise_white();
	counter++;
	return (o1 + o2 + o3 + o4 + o5 + o6 + o7 + o8) * 0.125;
}

float osc_noise_pitched(float phase, float inc) {
	static float b = -1.f;
	if (phase < inc) {
		float white = osc_noise_white();
		if (osc_noise_white() > 0.f) b = -b;
	}
	return b;
}

float osc_noise_popcorn() {
	// aka impulse / bi-stable / burst / random telegraph noise
	static float b = -0.95f;
	float white = osc_noise_white();
	if (white > 0.95) b = -b;
	return b + white * 0.05f;
}

float osc_pulse(float phase, float width) {
	return ((phase > width) * 2.f) - 1.f;
}

float osc_saw(float phase) {
	return (phase * 2.f) - 1.f;
}

float osc_saw_blep(float phase, float inc) {
	float out = (phase * 2.f) - 1.f;
	return out -= osc_helper_blep(phase, inc);
}

float osc_saw_from_sines(float phase, int sine_count) {
	float out = 0.f;
	float sin_phase = phase * osc_tao;
	float count = 1.f;
	for (int i = 0; i < sine_count; i++) {
		out += sinf(sin_phase * count) * (1.f / count);
		count += 1.f;
	}
	return out;
}

float osc_sin(float phase) {
	return sinf(phase * osc_tao);
}

float osc_square(float phase) {
	return ((phase < 0.5f) * 2.f) - 1.f;
}

float osc_square_blep(float phase, float inc) {
	float out = ((phase < 0.5f) * 2.f) - 1.f;
	out += osc_helper_blep(phase, inc);
	phase += 0.5f;
	if (phase > 1.f) phase -= 1.f;
	return out -= osc_helper_blep(phase, inc);
}

float osc_square_from_sines(float phase, int sine_count) {
	float out = 0.f;
	float sin_phase = phase * osc_tao;
	float count = 1.f;
	for (int i = 0; i < sine_count; i++) {
		out += sinf(sin_phase * count) * (1.f / count);
		count += 2.f;
	}
	return out;
}

float osc_triangle(float phase) {
	return (((phase <= 0.5f) ? phase * 2.f : (1.f - phase) * 2.f) - 0.5f);
}

float osc_triangle_blep(float phase, float inc) {
	static float last_out = 0.f;
	float out = osc_square_blep(phase, inc);
	out = inc * out + (1.f - inc) * last_out;
	return last_out = out;
}
