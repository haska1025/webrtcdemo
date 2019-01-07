#include "utility.h"
#include <webrtc/modules/utility/include/audio_frame_operations.h>

namespace hskdmo {
	void RemixAndResample(const int16_t* src_data,
		size_t samples_per_channel,
		size_t num_channels,
		int sample_rate_hz,
		webrtc::PushResampler<int16_t>* resampler,
		webrtc::AudioFrame* dst_frame)
	{
		const int16_t* audio_ptr = src_data;
		size_t audio_ptr_num_channels = num_channels;
		int16_t mono_audio[webrtc::AudioFrame::kMaxDataSizeSamples];

		// Downmix before resampling.
		if (num_channels == 2 && dst_frame->num_channels_ == 1) {
			webrtc::AudioFrameOperations::StereoToMono(src_data, samples_per_channel,
				mono_audio);
			audio_ptr = mono_audio;
			audio_ptr_num_channels = 1;
		}

		if (resampler->InitializeIfNeeded(sample_rate_hz, dst_frame->sample_rate_hz_,
			audio_ptr_num_channels) == -1) {			
			assert(false);
		}

		const size_t src_length = samples_per_channel * audio_ptr_num_channels;
		int out_length = resampler->Resample(audio_ptr, src_length, dst_frame->data_,
			webrtc::AudioFrame::kMaxDataSizeSamples);
		if (out_length == -1) {		
			assert(false);
		}
		dst_frame->samples_per_channel_ = out_length / audio_ptr_num_channels;

		// Upmix after resampling.
		if (num_channels == 1 && dst_frame->num_channels_ == 2) {
			// The audio in dst_frame really is mono at this point; MonoToStereo will
			// set this back to stereo.
			dst_frame->num_channels_ = 1;
			webrtc::AudioFrameOperations::MonoToStereo(dst_frame);
		}
	}

}