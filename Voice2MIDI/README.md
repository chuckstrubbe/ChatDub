# Voice2MIDI

Voice2MIDI is an original JUCE/C++ voice-to-MIDI VST3 starter project. It listens to the incoming audio signal, estimates a monophonic fundamental frequency with a YIN-style detector, and emits MIDI note-on/note-off events. Audio is passed through unchanged.

It is inspired by the general voice-to-MIDI workflow popularized by products such as Dubler 2. It is not affiliated with, endorsed by, or a copy of Dubler 2 or Vochlea.

## Current feature set

- Real-time monophonic pitch tracking for a single dominant voice/instrument.
- MIDI note-on and note-off generation on channel 1.
- Adjustable pitch confidence threshold (Sensitivity).
- Adjustable time before a new note is accepted (Note stability).
- MIDI transpose from -24 to +24 semitones.
- Input amplitude gate to reduce room-noise triggering.
- Audio passthrough.
- Parameter state saved with the DAW project.
- GitHub Actions workflow that builds a Windows x64 VST3 artifact.

## Important limitations of this first version

- This is monophonic: it follows one dominant pitch and does not identify simultaneous chord notes.
- It does not yet include Dubler-style drum/phoneme triggering, scale quantization, MIDI learn, or a multi-voice engine.
- Pitch detection is block/window based. Expect some detection latency; performance depends on the source, buffer size, microphone, and DAW routing.
- A clean, close-miked vocal signal works best. Use the Input gate and Sensitivity controls to reduce false notes.

## Build on Windows 11 (Visual Studio 2022)

### Prerequisites

1. Install **Visual Studio 2022 Community** and select the **Desktop development with C++** workload.
2. Install **CMake** (3.22 or newer) and ensure `cmake` is available in your terminal. Visual Studio's bundled CMake is also suitable if available on PATH.
3. Install **Git for Windows**.

### Build locally

Open **x64 Native Tools Command Prompt for VS 2022**, then run:

```bat
git clone https://github.com/YOUR-USERNAME/Voice2MIDI.git
cd Voice2MIDI
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target Voice2MIDI_VST3 --parallel
```

The first CMake configure downloads JUCE 8.0.4 from the official JUCE GitHub repository, so the first build requires an internet connection.

CMake normally places the built plugin under a path similar to:

```text
build/Voice2MIDI_artefacts/Release/VST3/Voice2MIDI.vst3
```

The exact intermediate path can vary slightly by CMake generator and JUCE version.

### Install for Ableton Live on Windows

1. Copy the `Voice2MIDI.vst3` folder to:
   `%COMMONPROGRAMFILES%\VST3`
2. In Ableton Live, open **Preferences → Plug-Ins**.
3. Ensure VST3 plug-in support is enabled, then rescan if necessary.
4. Load **Voice2MIDI** on an audio track receiving your microphone.
5. Create a MIDI track with a virtual instrument. Set its MIDI input to receive from the Voice2MIDI track/plugin if your Ableton routing/version exposes the plug-in MIDI output. Enable monitoring/recording as needed.

**Routing note:** VST3 MIDI-output routing and track-to-track MIDI capture can vary by DAW and host version. If Ableton does not expose the generated MIDI from this audio effect, use a host that supports MIDI output from audio-effect plug-ins or add a dedicated MIDI-output wrapper/routing stage. Verify the MIDI output in a MIDI monitor before relying on it in a session.

## Build automatically on GitHub

1. Create a new empty GitHub repository named `Voice2MIDI`.
2. Upload the contents of this folder (the `CMakeLists.txt`, `Source` folder, `.github` folder, `.gitignore`, and `README.md`) to the repository's root. Do not upload the outer containing folder as an extra nested directory.
3. Open the repository's **Actions** tab.
4. Select **Build Voice2MIDI VST3** and click **Run workflow**. A push to `main` or `master` that changes source/build files also starts a build.
5. When the workflow finishes, open the run and download the `Voice2MIDI-VST3-Windows` artifact.
6. Extract the ZIP and install the `Voice2MIDI.vst3` folder as described above.

## Project files

```text
Voice2MIDI/
├── .github/
│   └── workflows/
│       └── build-windows.yml
├── Source/
│   ├── PluginEditor.cpp
│   ├── PluginEditor.h
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── YinPitchDetector.cpp
│   └── YinPitchDetector.h
├── .gitignore
├── CMakeLists.txt
└── README.md
```

## JUCE licensing

This project fetches JUCE at configure time rather than copying JUCE source into this repository. Review the current JUCE licensing terms (including GPLv3 and commercial licensing options) and choose the license appropriate for how you intend to distribute your plugin. Your own source code and JUCE's license are separate matters.
