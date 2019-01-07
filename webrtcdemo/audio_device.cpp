#include <commonlib/utility.h>

#include <webrtc/modules/audio_device/include/audio_device.h>
#include <webrtc/common_audio/resampler/include/resampler.h>
#include <webrtc/modules/audio_processing/aec/echo_cancellation.h>
#include <webrtc/common_audio/vad/include/webrtc_vad.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <Windows.h>



#define SAMPLE_RATE			32000
#define SAMPLES_PER_10MS	(SAMPLE_RATE/100)

FILE *record_file = NULL;


class AudioTransportImpl : public webrtc::AudioTransport
{
private:
	webrtc::Resampler *resampler_in;
	webrtc::Resampler *resampler_out;
	void *aec;
	VadInst *vad;

public:
	AudioTransportImpl(webrtc::AudioDeviceModule* audio) {
		resampler_in = new webrtc::Resampler(48000, SAMPLE_RATE, 2);
		resampler_out = new webrtc::Resampler(SAMPLE_RATE, 48000, 2);
		int ret;

		aec = WebRtcAec_Create();
		assert(aec);
		ret = WebRtcAec_Init(aec, SAMPLE_RATE, SAMPLE_RATE);
		assert(ret == 0);

		vad = WebRtcVad_Create();
		assert(vad);
		ret = WebRtcVad_Init(vad);
		assert(ret == 0);
	}

	~AudioTransportImpl() {
		delete resampler_in;
		delete resampler_out;

		WebRtcAec_Free(aec);
		WebRtcVad_Free(vad);
	}


	virtual int32_t RecordedDataIsAvailable(
		const void* audioSamples,
		const size_t nSamples,
		const size_t nBytesPerSample,
		const size_t nChannels,
		const uint32_t samplesPerSec,
		const uint32_t totalDelayMS,
		const int32_t clockDrift,
		const uint32_t currentMicLevel,
		const bool keyPressed,
		uint32_t& newMicLevel) override
	{
		
		printf("RecordedDataIsAvailable %d %d %d %d %d %d %d %d\n", nSamples,
		nBytesPerSample, nChannels,
		samplesPerSec, totalDelayMS,
		clockDrift, currentMicLevel, keyPressed);

		printf("RecordedDataIsAvailable samples:");

		/*const size_t *pSample = (const size_t*)audioSamples;
		for (size_t i = 0; i < nSamples*2; ++i) {
			printf(" %x", pSample[i]);
		}*/
		printf("\n");

		fwrite(audioSamples, nSamples * 4, 1, record_file);
		/*
		int ret;
		ret = WebRtcVad_Process(vad, samplesPerSec,
			(int16_t*)audioSamples, nSamples);
		if (ret == 0) {
			return 0;
		}
		printf("");

		int16_t *samples = (int16_t *)buffer->push();
		if (samples == NULL) {
			printf("drop oldest recorded frame");
			buffer->pop();
			samples = (int16_t *)buffer->push();
			assert(samples);
		}

		int maxLen = SAMPLES_PER_10MS;
		size_t outLen = 0;
		resampler_in->Push((const int16_t*)audioSamples,
			nSamples,
			samples,
			maxLen, outLen);


		ret = WebRtcAec_Process(aec,
		samples,
		NULL,
		samples,
		NULL,
		SAMPLES_PER_10MS,
		totalDelayMS,
		0);
		assert(ret != -1);
		if(ret == -1){
		    printf("%d %d", ret, WebRtcAec_get_error_code(aec));
		}
		int status = 0;
		int err = WebRtcAec_get_echo_status(aec, &status);

		if(status == 1){

		}
		*/

		return 0;
	}

	virtual int32_t NeedMorePlayData(
		const size_t nSamples,
		const size_t nBytesPerSample,
		const size_t nChannels,
		const uint32_t samplesPerSec,
		void* audioSamples,
		size_t& nSamplesOut,
		int64_t* elapsed_time_ms,
		int64_t* ntp_time_ms) override
	{
		// TODO: multithread lock
		
		printf("NeedMorePlayData %d %d %d %d\n", nSamples,
		nBytesPerSample,
			nChannels,
		samplesPerSec);

		fread(audioSamples, nSamples * 4, 2, record_file);
		nSamplesOut = nSamples;
		
		/*
		int16_t *samples = (int16_t *)buffer->pop();
		if (samples == NULL) {
			printf("no data for playout");
			int16_t *ptr16Out = (int16_t *)audioSamples;
			for (int i = 0; i < nSamples; i++) {
				*ptr16Out = 0; // left
				ptr16Out++;
				*ptr16Out = 0; // right (same as left sample)
				ptr16Out++;
			}
			return 0;
		}


		int ret = 0;
		//ret = WebRtcAec_BufferFarend(aec, samples, SAMPLES_PER_10MS);
		//assert(ret == 0);


		size_t outLen = 0;

		int16_t samplesOut[48000];
		resampler_out->Push(samples,
			SAMPLES_PER_10MS,
			samplesOut,
			nSamples * nBytesPerSample, outLen);

		//log_debug("play %d bytes", outLen * 2);

		int16_t *ptr16Out = (int16_t *)audioSamples;
		int16_t *ptr16In = samplesOut;
		// do mono -> stereo
		for (int i = 0; i < nSamples; i++) {
			*ptr16Out = *ptr16In; // left
			ptr16Out++;
			*ptr16Out = *ptr16In; // right (same as left sample)
			ptr16Out++;
			ptr16In++;
		}

		nSamplesOut = nSamples;
		*/
		return 0;
	}

	virtual int OnDataAvailable(
		const int voe_channels[],
		size_t number_of_voe_channels,
		const int16_t* audio_data,
		int sample_rate,
		size_t number_of_channels,
		size_t number_of_frames,
		int audio_delay_milliseconds,
		int current_volume,
		bool key_pressed,
		bool need_audio_processing) override
	{
		printf("OnDataAvailable\n");
		return 0;
	}

public:
};

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Need input the param recording(1) or playout(2).\n");
		return -1;
	}

	int mode = atoi(argv[1]);
	if (mode == 2){
		record_file = fopen("recordingdata.pcm", "r");
	}
	else {
		record_file = fopen("recordingdata.pcm", "w");
	}
	if (!record_file) {
		fprintf(stderr, "Open file failed!\n");
		return -1;
	}

	webrtc::AudioDeviceModule *audio = webrtc::CreateAudioDeviceModule(0, webrtc::AudioDeviceModule::kPlatformDefaultAudio);
	assert(audio);
	audio->Init();

	int num;
	int ret;

	num = audio->RecordingDevices();
	printf("Input devices: %d\n", num);
	for (int i = 0; i<num; i++) {
		char name[webrtc::kAdmMaxDeviceNameSize];
		char guid[webrtc::kAdmMaxGuidSize];
		int ret = audio->RecordingDeviceName(i, name, guid);
		if (ret != -1) {
			printf("	%d %s %s\n", i, name, guid);
		}
	}

	num = audio->PlayoutDevices();
	printf("Output devices: %d\n", num);
	for (int i = 0; i<num; i++) {
		char name[webrtc::kAdmMaxDeviceNameSize];
		char guid[webrtc::kAdmMaxGuidSize];
		int ret = audio->PlayoutDeviceName(i, name, guid);
		if (ret != -1) {
			printf("	%d %s %s\n", i, name, guid);
		}
	}
	AudioTransportImpl callback(audio);
	ret = audio->RegisterAudioCallback(&callback);

	if (mode==2) {
		ret = audio->SetPlayoutDevice(0);
		/*
		ret = audio->SetPlayoutSampleRate(16000);
		if (ret == -1) {
			uint32_t rate = 0;
			audio->PlayoutSampleRate(&rate);
			printf("use resampler for playout, device samplerate: %u\n", rate);
		}
		*/
		ret = audio->InitPlayout();
		printf("Start playout\n");
		ret = audio->StartPlayout();
	}
	else {
		ret = audio->SetRecordingDevice(0);
		/*
		ret = audio->SetRecordingSampleRate(16000);
		if (ret == -1) {
			uint32_t rate = 0;
			audio->RecordingSampleRate(&rate);
			printf("use resampler for recording, device samplerate: %u\n", rate);
		}
		*/
		ret = audio->InitRecording();
		printf("Start recording\n");
		ret = audio->StartRecording();
		
	}

	
	Sleep(20*1000);

	printf("End play or recording\n");

	if (mode == 1) {
		audio->StopRecording();
	}
	else {
		audio->StopPlayout();
	}
	audio->Terminate();

	fclose(record_file);
	record_file = NULL;

	return 0;
}