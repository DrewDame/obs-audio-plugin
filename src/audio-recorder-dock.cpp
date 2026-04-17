#include "audio-recorder-dock.h"
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
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

	const char *obs_path = obs_frontend_get_current_record_output_path();
	QString default_dir = obs_path ? QString::fromUtf8(obs_path)
				       : QDir::homePath() + "/Videos";
	bfree((void *)obs_path);

	path_edit_ = new QLineEdit(default_dir, this);
	path_edit_->setPlaceholderText("Output folder (e.g. C:/Recordings)");

	browse_btn_ = new QPushButton("Browse...", this);

	auto *path_row = new QHBoxLayout();
	path_row->setContentsMargins(0, 0, 0, 0);
	path_row->setSpacing(6);
	path_row->addWidget(path_edit_, 1);
	path_row->addWidget(browse_btn_);

	name_edit_ = new QLineEdit(this);
	name_edit_->setPlaceholderText("File name (optional, .mp3 auto-added)");

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

	layout->addLayout(path_row);
	layout->addWidget(name_edit_);
	layout->addWidget(toggle_btn_);
	layout->addWidget(status_label_);
	layout->addStretch();

	connect(toggle_btn_, &QPushButton::clicked, this,
		&AudioRecorderDock::on_toggle_clicked);
	connect(browse_btn_, &QPushButton::clicked, this,
		&AudioRecorderDock::on_browse_clicked);
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

void AudioRecorderDock::on_browse_clicked()
{
	QString selected = QFileDialog::getExistingDirectory(
		this, "Select output folder", path_edit_->text());
	if (!selected.isEmpty())
		path_edit_->setText(QDir::toNativeSeparators(selected));
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
	QString dir = path_edit_->text().trimmed();
	if (dir.isEmpty())
		dir = QDir::homePath() + "/Videos";

	QString filename = name_edit_->text().trimmed();
	if (filename.isEmpty()) {
		filename = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
	}
	if (!filename.endsWith(".mp3", Qt::CaseInsensitive))
		filename += ".mp3";

	QFileInfo out_info(QDir(dir).filePath(filename));
	QDir().mkpath(out_info.path());
	return out_info.absoluteFilePath().toStdString();
}

void AudioRecorderDock::set_recording_ui(bool recording)
{
	toggle_btn_->setChecked(recording);
	toggle_btn_->setText(recording ? obs_module_text("StopRecording")
				       : obs_module_text("StartRecording"));
	if (!recording)
		status_label_->setText(obs_module_text("Idle"));
}
