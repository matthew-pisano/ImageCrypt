error:
	@echo "Please choose one of the following target: debug, release"
	@exit 2

debug: src/main.cpp src/image_encode.cpp src/encodings.cpp src/base64.cpp
	g++ -g -Iinclude -Ilib src/main.cpp src/image_encode.cpp src/encodings.cpp src/base64.cpp \
		`pkg-config --cflags --libs opencv4` `Magick++-config --cxxflags --cppflags --ldflags --libs` \
		-o cmake-build-debug/icrypt

release: src/main.cpp src/image_encode.cpp src/encodings.cpp src/base64.cpp
	g++ -03 -Iinclude -Ilib -o icrypt src/main.cpp src/image_encode.cpp src/encodings.cpp src/base64.cpp \
		`pkg-config --cflags --libs opencv4` `Magick++-config --cxxflags --cppflags --ldflags --libs` \
		-o cmake-build-release/icrypt