build:
	clang \
	-g -O0 -std=c++20 -lstdc++ \
	main.cpp \
	../thirdparty/imgui/backends/imgui_impl_sdlrenderer2.cpp \
	../thirdparty/imgui/backends/imgui_impl_sdl2.cpp \
	../thirdparty/imgui/imgui*.cpp \
	-I../thirdparty/imgui/ \
	-I../thirdparty/imgui/backends \
	-I../../game/src \
	-lSDL2 \
	-lSDL2_ttf \
	-lSDL2_image \
	-o ../bin/editor

run:
	../bin/editor
