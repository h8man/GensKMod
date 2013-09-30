You need Visual C++ 6.0 with the DirectX 7.0 Runtime library and NASM installed
in the root directory of the drive where you've checked out Gens (\nasm) to
compile Gens project (NASM in included in the sources package, you just need
to move it in the root directory).

Gens uses the Starcream 680x0 library emulation for the genesis and Sega CD
68000 CPU emulation, the core has been customised and optimised for Gens.

Gens project is structured as follow :

- Gens directory contains all gens sources except the CPU cores.
- Main68k directory contains the sources for the main 68000 (genesis 68000).
- Sub68k directory contains the sources for the sub 68000 (Sega CD 68000).
- z80 directory contains the sources for the Z80 cpu (genesis).

I used 2 68000 cores for speed reasons :
- No context swap required.
- Each core is optimised for its function.

Here's the step to follow to compile succefully the win32 vesion of Gens :

- Be sure to have VC (Visual C++) 6.0 with the DirectX 7.0 Runtime library installed.
- Open the Main68k project (DSW file) with VC, build star.exe then close the project.
- Open the Sub68k project with VC, build star.exe then close the project.
- Exec the comp_m68k.bat file.
- Exec the comp_s68k.bat file.
- Open the Gens project with VC and build gens.exe, that's all :)

Gens.exe file is normally build in the gens\release directory.
To uses netplay features, Gens requires kailleraclient.dll, the file should be in
the same directory as gens.exe



Kaneda's note on compiling
The released directory must exist when creating main/sub68k obj
Don't change the directory structure!
There is a way to have subprojects on VC and call batch on end compilation so all the steps could be simplify...but I don't know how!



Kaneda's note on DirectX
With Visual C++ 6.0, you can only compile with Directx8 released before April04.
If you want to compile only Gens, considere to use dx7_min which only include librairie and headers.
Don't forget to put the directories on Tools>Options>Directories.
If you don't put these directories on top of the list, you'll get compile errors (wtf?!)



Kaneda's note on Nasm
asm is VERY sensitive! a mistake and Gens doesn't work at all!
The last versions of nasm break the message texts (wtf?!)
I used 0.98.08 (come with Gens source) with sucess while 0.98.34+ break all
So if the small case letter are wrong on printed message, change your nasmw!
If you find why misc.asm breaks these letters only...tell me!!!
The 'new' version of Gens use -01 option but I prefered change the asm file myself, to avoid error
Since I doesn't like to "move nasm on root", I updated scripts to use relative paths.
If it doesn't work for you, follow Stef's advice : put it on root and ajust path for your needs




