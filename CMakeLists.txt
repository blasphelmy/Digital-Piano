# CMakeList.txt : CMake project for DigitalPiano_cmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("DigitalPiano_cmake")

# Add source to this project's executable.
add_executable (DigitalPiano_cmake   "Binasc.h" "MidiEvent.h" "MidiEventList.h" "MidiFile.h" "MidiMessage.h" "Options.h" "Binasc.cpp" "MidiEvent.cpp" "MidiEventList.cpp" "MidiFile.cpp" "MidiMessage.cpp" "Options.cpp"  "DigitalPiano.h" "GLOBALVARIABLES.h" "key.h" "main.cpp" "MAPPER.h" "minisdl_audio.c" "minisdl_audio.h" "olcPixelGameEngineGL.h" "RtMidi.cpp" "RtMidi.h" "tml.h" "tsf.h" "vectors.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET DigitalPiano_cmake PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
