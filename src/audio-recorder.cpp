#include "audio-recorder.h"
#include <obs-module.h>
#include <util/platform.h>

AudioRecorder::AudioRecorder() {}

AudioRecorder::~AudioRecorder()
{
	release();
}

bool AudioRecorder::start(const std::string &output_path)
{
	if (is_recording())
		return false;

	// Configure ffmpeg_output for audio-only MP3
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", output_path.c_str());
	obs_data_set_string(settings, "format_name", "mp3");
	obs_data_set_string(settings, "format_mime_type", "audio/mpeg");
	obs_data_set_string(settings, "audio_encoder", "libmp3lame");
	obs_data_set_int(settings, "audio_bitrate", 192);
	obs_data_set_bool(settings, "video", false);
	obs_data_set_bool(settings, "audio", true);
	obs_data_set_int(settings, "audio_tracks", 1);

	output_ = obs_output_create("ffmpeg_output", "audio_recorder_output",
				    settings, nullptr);
	obs_data_release(settings);

	if (!output_)
		return false;

	if (!obs_output_start(output_)) {
		release();
		return false;
	}

	return true;
}

void AudioRecorder::stop()
{
	if (output_) {
		obs_output_stop(output_);
		release();
	}
}

bool AudioRecorder::is_recording() const
{
	return output_ != nullptr && obs_output_active(output_);
}

void AudioRecorder::release()
{
	if (output_) {
		obs_output_release(output_);
		output_ = nullptr;
	}
}
