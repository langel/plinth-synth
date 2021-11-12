
const float osc_tao = M_PI * 2;

float osc_noise(float phase) {
	return (((float)rand()/(float)(RAND_MAX)) * 2) - 1;
}

float osc_pulse(float phase, float width) {
	return ((phase > width) * 2) - 1;
}

float osc_saw(float phase) {
	return (phase * 2) - 1;
}

float osc_saw_five_sine(float phase) {
	float out = 0.f;
	float sin_phase = phase * osc_tao;
	float count = 1.f;
	for (int i = 0; i < 5; i++) {
		out += sinf(sin_phase * count) * (1.f / count);
		count += 1.f;
	}
	return out;
}

float osc_sin(float phase) {
	return sinf(phase * osc_tao);
}

float osc_square(float phase) {
	return ((phase > 0.5) * 2) - 1;
}

float osc_square_five_sine(float phase) {
	float out = 0.f;
	float sin_phase = phase * osc_tao;
	float count = 1.f;
	for (int i = 0; i < 5; i++) {
		out += sinf(sin_phase * count) * (1.f / count);
		count += 2.f;
	}
	return out;
}

float osc_triangle(float phase) {
	return (((phase <= 0.5) ? phase * 2 : (1 - phase) * 2) - 0.5);
}

