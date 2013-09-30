rem Main 68000 compilation (Main68k\star.c has been compiled before)

@cd starscream\main68k\release
star temp.asm -hog -name main68k_
..\..\..\..\nasmw -f win32 temp.asm -o ..\..\..\Gens\release\main68k.obj
@cd..\..\..

pause