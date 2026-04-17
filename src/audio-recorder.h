#pragma once

#include <obs.h>
#include <string>

class AudioRecorder {
public:
	AudioRecorder();
	~AudioRecorder();

	bool start(const std::string &output_path);
	void stop();
	/** Call from UI thread tick after stop() to finish async shutdown safely. */
	void poll();
	bool is_recording() const;
	const std::string &last_error() const { return last_error_; }

private:
	obs_output_t *output_ = nullptr;
	bool stopping_ = false;
	bool started_ = false;
	std::string last_error_;

	void release();
};
