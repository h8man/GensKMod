Updated version of GensKMod which could be compiled under Visual Studio Express 2013

Unlike previous version, every needed libraries and external tool are now included in this project.
No need to launch dos batch anymore.
Open the GensKMod project with VS2013 and build, that's all :)
It will build the 68000 cores, compile them and then build gensKmod.exe.


GensKMod for VS2013 uses NASM (old version).
It is also based on the newest zlibwapi, so be sure to include zlibwapi.dll in gens directory.
GensKMod is based on Gens and so uses the Starcream 680x0 library emulation for the genesis and Sega CD 68000 CPU emulation, the core has been customised and optimised for Gens.

Gens original project is structured as follow :
- Gens directory contains all gens sources except the CPU cores.
- Starscream\Main68k directory contains the sources for the main 68000 (genesis 68000).
- Starscream\Sub68k directory contains the sources for the sub 68000 (Sega CD 68000).

Gens uses 2 68000 cores for speed reasons :
- No context swap required.
- Each core is optimised for its function.

To uses netplay features, Gens requires kailleraclient.dll, the file should be in
the same directory as gens.exe

Help is compiled using HelpNDoc (http://www.helpndoc.com)

Many thanks to Stef, Fonzie, Edge and so many others for KMod
Special thanks to djcouchycouch for his helps on Visual Studio update !
and to Graz for his wonderful fixes !