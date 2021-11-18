
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

#include "./osc/tables.c"
#include "./osc/basics.c"
#include "./osc/noises.c"
