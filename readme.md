# GensKMod (Visual Studio 2022 Edition)

Updated version of **GensKMod** that can be compiled directly with **Visual Studio 2022 Community**.

Unlike previous versions, all required libraries and external tools are now included in this project.

- ✅ No more DOS batch files  
- ✅ No manual core compilation  
- ✅ Just open and build  

Simply open the `GensKMod` solution in VS2022 and build.

The build process will:
1. Assemble the 68000 cores
2. Compile the cores
3. Generate `gensKmod.exe`

## Included Components

This version uses an older NASM release, which is included in the project.

GensKMod is based on the original **Gens** emulator and uses the **Starcream 680x0** library for Sega Genesis and Sega CD 68000 CPU emulation.

The CPU core has been customized and optimized for Gens.

### Directory Layout
```
Gens/
├── All Gens source files (except CPU cores)
Starscream/
├── Main68k/ -> Genesis 68000 CPU core
└── Sub68k/ -> Sega CD 68000 CPU core
```
Gens uses two separate 68000 cores for performance reasons:

- No context switching required
- Each core optimized for its specific function

## Netplay Support

To enable Netplay features, place `kailleraclient.dll` in the same directory as `gensKmod.exe`.

## Help Documentation

Help files are compiled using **HelpNDoc**:  
http://www.helpndoc.com

## Credits

Many thanks to:
- Stef  
- Fonzie  
- Edge  
- And many others for KMod  

Special thanks to:
- djcouchycouch — for help updating to Visual Studio  
- Graz — for his fixes and improvements  

GensKMod by KanedaFr

## Summary

This version modernizes the build process while preserving the original GensKMod structure and performance optimizations.

Open the solution, build, and run.

Enjoy!
