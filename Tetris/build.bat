@echo off

mkdir ..\..\build
pushd ..\..\build
cl -FC -Zi  C:\Users\Sam\Documents\GitHub\Tetris\Tetris\Tetris.cpp C:\Users\Sam\Documents\GitHub\Tetris\Tetris\Texture.cpp /I C:\Users\Sam\Desktop\SDL2-2.0.14\include /I C:\Users\Sam\Desktop\SDL2_image-2.0.5\include /link /LIBPATH:C:\Users\Sam\Desktop\SDL2-2.0.14\lib\x86 /LIBPATH:C:\Users\Sam\Desktop\SDL2_image-2.0.5\lib\x86 SDL2.lib SDL2main.lib shell32.lib SDL2_image.lib /SUBSYSTEM:CONSOLE
popd
