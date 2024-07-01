mac:
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

win32:
	clang \
	-g -O0 -std=c++20 -lstdc++ \
	main.cpp \
	../thirdparty/imgui/backends/imgui_impl_sdlrenderer2.cpp \
	../thirdparty/imgui/backends/imgui_impl_sdl2.cpp \
	../thirdparty/imgui/imgui*.cpp \
	-I../thirdparty/imgui/ \
	-I../thirdparty/imgui/backends \
	-I../thirdparty/SDL2/include \
	-I../thirdparty/SDL2_image/include \
	-I../../game/src \
	-L../thirdparty/SDL2/lib/x64 \
	-L../thirdparty/SDL2_image/lib/x64 \
	-lSDL2 \
	-lSDL2_image \
	-o ../bin/editor.exe

run:
	../bin/editor
