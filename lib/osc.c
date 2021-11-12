
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

float osc_noise(float phase) {
	return (((float)rand()/(float)(RAND_MAX)) * 2.f) - 1.f;
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
