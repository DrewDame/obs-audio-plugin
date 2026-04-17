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

	last_error_.clear();

	// Configure ffmpeg_output for audio-only MP3
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "url", output_path.c_str());
	obs_data_set_string(settings, "format_name", "mp3");
	obs_data_set_string(settings, "format_mime_type", "audio/mpeg");
	// Let FFmpeg select a valid encoder for the mp3 container on this system.
	obs_data_set_int(settings, "audio_bitrate", 192);

	output_ = obs_output_create("ffmpeg_output", "audio_recorder_output",
				    settings, nullptr);
	obs_data_release(settings);

	if (!output_)
		return false;

	// ffmpeg_output reads mixers/media from output state, not these settings.
	obs_output_set_mixers(output_, 1);
	obs_output_set_media(output_, obs_get_video(), obs_get_audio());

	stopping_ = false;
	started_ = false;

	if (!obs_output_start(output_)) {
		const char *err = obs_output_get_last_error(output_);
		last_error_ = err ? err : "";
		release();
		return false;
	}

	started_ = true;
	return true;
}

void AudioRecorder::stop()
{
	if (!output_)
		return;
	stopping_ = true;
	obs_output_stop(output_);
}

void AudioRecorder::poll()
{
	if (!output_)
		return;

	/* User requested stop: wait until output is inactive, then release. */
	if (stopping_ && !obs_output_active(output_)) {
		release();
		return;
	}

	/* Encoder/output died without an explicit stop (error path). */
	if (started_ && !stopping_ && !obs_output_active(output_)) {
		const char *err = obs_output_get_last_error(output_);
		last_error_ = err ? err : "";
		release();
	}
}

bool AudioRecorder::is_recording() const
{
	return output_ != nullptr &&
	       (obs_output_active(output_) || stopping_);
}

void AudioRecorder::release()
{
	if (output_) {
		obs_output_release(output_);
		output_ = nullptr;
	}
	stopping_ = false;
	started_ = false;
}
