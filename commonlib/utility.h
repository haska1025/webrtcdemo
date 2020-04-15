#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <inttypes.h>

#include <common_audio/resampler/include/push_resampler.h>
#include <api/audio/audio_frame.h>

namespace hskdmo {
	void RemixAndResample(const int16_t* src_data,
		size_t samples_per_channel,
		size_t num_channels,
		int sample_rate_hz,
		webrtc::PushResampler<int16_t>* resampler,
		webrtc::AudioFrame* dst_frame);

}
#endif // _UTILITY_H_

