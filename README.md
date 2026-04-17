# OBS Audio Recorder Plugin

Audio-only recording dock plugin for OBS Studio.

## Install In OBS

1. Close OBS Studio.
2. Run the installer: [Open installer EXE](./build/obs-audio-recorder-1.0.0-windows-x64.exe)
3. In the installer, keep the install folder set to your OBS location (usually `C:\Program Files\obs-studio`).
4. Finish installation, then launch OBS Studio.
5. Open the plugin dock from OBS and use **Start Recording** / **Stop Recording**.

## Manual Install (ZIP fallback)

If the installer is blocked by security or permissions:

1. Open the ZIP package in `build/`.
2. Copy `obs-plugins\64bit\obs-audio-recorder.dll` into:
   - `C:\Program Files\obs-studio\obs-plugins\64bit\`
3. Copy `data\obs-plugins\obs-audio-recorder\` into:
   - `C:\Program Files\obs-studio\data\obs-plugins\obs-audio-recorder\`
4. Restart OBS Studio.

## Troubleshooting

- If the plugin does not appear, confirm files exist in the two OBS folders above.
- If recording fails, open OBS logs and search for `audio_recorder_output`.
