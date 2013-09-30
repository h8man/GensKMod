rem Main 68000 compilation (Sub68k\star.c has been compiled before)

@cd starscream\sub68k\release
star temp.asm -hog -name sub68k_
..\..\..\..\nasmw -f win32 temp.asm -o ..\..\..\Gens\release\sub68k.obj
@cd..\..\..
pause