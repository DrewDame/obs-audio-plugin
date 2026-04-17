#pragma once

#include <obs.h>
#include <string>

class AudioRecorder {
public:
	AudioRecorder();
	~AudioRecorder();

	bool start(const std::string &output_path);
	void stop();
	bool is_recording() const;

private:
	obs_output_t *output_ = nullptr;
	obs_encoder_t *audio_enc_ = nullptr;

	void release();
};
