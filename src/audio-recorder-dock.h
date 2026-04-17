#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QLineEdit>
#include "audio-recorder.h"

class AudioRecorderDock : public QWidget {
	Q_OBJECT

public:
	static void create();
	static void destroy();

	explicit AudioRecorderDock(QWidget *parent = nullptr);
	~AudioRecorderDock() override = default;

private slots:
	void on_toggle_clicked();
	void on_browse_clicked();
	void sync_recording_state();

private:
	std::string build_output_path() const;
	void set_recording_ui(bool recording);

	QPushButton *toggle_btn_;
	QLabel *status_label_;
	QLineEdit *path_edit_;
	QLineEdit *name_edit_;
	QPushButton *browse_btn_;
	QTimer *state_timer_;
	AudioRecorder recorder_;

	static AudioRecorderDock *instance_;
};
