#include "audio-recorder-dock.h"
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QString>

AudioRecorderDock *AudioRecorderDock::instance_ = nullptr;

void AudioRecorderDock::create()
{
	if (instance_)
		return;

	instance_ = new AudioRecorderDock(
		static_cast<QWidget *>(obs_frontend_get_main_window()));

	obs_frontend_add_dock_by_id("audio-recorder-dock",
				    obs_module_text("AudioRecorder"),
				    instance_);
}

void AudioRecorderDock::destroy()
{
	// OBS owns the dock widget lifetime after add_dock_by_id
	instance_ = nullptr;
}

AudioRecorderDock::AudioRecorderDock(QWidget *parent) : QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(8, 8, 8, 8);
	layout->setSpacing(6);

	toggle_btn_ = new QPushButton(obs_module_text("StartRecording"), this);
	toggle_btn_->setCheckable(true);
	toggle_btn_->setMinimumHeight(36);

	// Red dot style matching OBS record button feel
	toggle_btn_->setStyleSheet(
		"QPushButton {"
		"  background-color: #3a3a3a;"
		"  color: white;"
		"  border: 1px solid #555;"
		"  border-radius: 4px;"
		"  font-weight: bold;"
		"}"
		"QPushButton:checked {"
		"  background-color: #c0392b;"
		"  border-color: #e74c3c;"
		"}");

	status_label_ = new QLabel(obs_module_text("Idle"), this);
	status_label_->setAlignment(Qt::AlignCenter);
	status_label_->setStyleSheet("color: #aaa; font-size: 11px;");

	state_timer_ = new QTimer(this);
	state_timer_->setInterval(250);

	layout->addWidget(toggle_btn_);
	layout->addWidget(status_label_);
	layout->addStretch();

	connect(toggle_btn_, &QPushButton::clicked, this,
		&AudioRecorderDock::on_toggle_clicked);
	connect(state_timer_, &QTimer::timeout, this,
		&AudioRecorderDock::sync_recording_state);
	state_timer_->start();
}

void AudioRecorderDock::on_toggle_clicked()
{
	if (recorder_.is_recording()) {
		recorder_.stop();
		set_recording_ui(false);
	} else {
		std::string path = build_output_path();
		if (recorder_.start(path)) {
			set_recording_ui(true);
			status_label_->setText(
				QString("Saving to: %1")
					.arg(QString::fromStdString(path)));
		} else {
			toggle_btn_->setChecked(false);
			status_label_->setText(
				obs_module_text("ErrorStarting"));
		}
	}
}

void AudioRecorderDock::sync_recording_state()
{
	recorder_.poll();
	if (!recorder_.is_recording() && toggle_btn_->isChecked()) {
		set_recording_ui(false);
		if (!recorder_.last_error().empty())
			status_label_->setText(
				QString("Recording failed: %1")
					.arg(QString::fromStdString(recorder_.last_error())));
	}
}

std::string AudioRecorderDock::build_output_path() const
{
	// Mirror OBS's default recording directory
	const char *obs_path = obs_frontend_get_current_record_output_path();
	QString dir = obs_path ? QString::fromUtf8(obs_path)
			       : QDir::homePath() + "/Videos";
	bfree((void *)obs_path);

	QString filename = QDateTime::currentDateTime().toString(
				   "yyyy-MM-dd_HH-mm-ss") +
			   ".mp3";

	return QDir(dir).filePath(filename).toStdString();
}

void AudioRecorderDock::set_recording_ui(bool recording)
{
	toggle_btn_->setChecked(recording);
	toggle_btn_->setText(recording ? obs_module_text("StopRecording")
				       : obs_module_text("StartRecording"));
	if (!recording)
		status_label_->setText(obs_module_text("Idle"));
}
