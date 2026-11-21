# Release by default; each build type in its own directory (CMakePresets.json): build/debug, build/release.
all: release

debug release:
	cmake --preset $@
	cmake --build --preset $@

.PHONY: all debug release
