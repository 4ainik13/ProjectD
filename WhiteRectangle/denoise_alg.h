#pragma once

namespace alg
{
	void initMasks();
	void denoiseImage(unsigned char* data, int height, int width, int channels);
}