
                                                            
                                                            
  

  - DLSSG MODIFED VERSION

                                                          
 ----------------------------------------------- -----------------------------------------------


  **NEW RELEASE 2.8.4**


-------------------------------------------------------------


UPDATE: **WORKS BOTH ON 20 SERIES AND 30 SERIES. no need to install two different managers anymore.**

**Picture quality (PSNR, higher is better) went from 29.70 to 30.30 dB.**

New update:

| Crop | Fix20 wrong pixels | Fix21 wrong pixels |
| :--- | :---: | :---: |
| Roofs / chimney | 2,275 | 320 (-86%) |
| Ground | 355 | 81 |
| HUD over clouds | 7,892 | 6,681 |
| Building edge | 5,373 | 5,013 |
| Other buildings | 1,372 | 1,118 |
| **Total** | **17,267** | **13,213** |


<img width="1905" height="983" alt="image" src="https://github.com/user-attachments/assets/585c3841-f802-4174-aaf5-a3f8d3f98011" />



-------------------------------------------------------------

To Activate it: activate it through Nvidia Profile Inspector


**To Use Nvidia profile inspector:**

**-Open NVIDIA Profile Inspector on your computer.**

**-Locate Smooth Motion**

**-Enable it and Save**

**FOR DOWNLOADS CHECK RELEASES**

--------------------------------------------------------------------------

**DLSSG INSTALL FOR 3000S SERIES:**

<img width="1516" height="493" alt="image" src="https://github.com/user-attachments/assets/5b754f4d-e8d0-4c87-bc09-1c3114fc08a9" />

1. Fully exit the game. Back up any existing mod proxy and INI outside the game folder.
2. Copy this package's `version.dll` and `dlssg_sm86.ini` beside the actual rendering EXE.
   If that DLL name is occupied, choose ONE original-name DLL from `altnative` that
   the game loads. Preserve other mods and the game's original DLSSG files. 
3. Start the game, enable frame generation, and select X6 if supported.

-------------------------------------------------------------

**DLSSG INSTALL FOR 2000S SERIES:**

  Same Steps As 3000s series's install

-------------------------------------------------------------

## Linux & Proton Build Support

This repository supports building both Linux native shared libraries (`.so`) and Windows PE binaries (`.dll`) for Proton/Wine directly on Linux.

### Requirements

- CMake 3.20+
- Vulkan SDK / Vulkan headers (`vulkan-headers`)
- C++17 compatible compiler (GCC or Clang)
- *(Optional for Proton)*: MinGW-w64 cross compiler (`mingw-w64-gcc`)

### Building

You can use the helper script `./build.sh` or standard CMake:

#### 1. Native Linux (`.so`)
```bash
./build.sh native
# Or via CMake directly:
cmake -B build-native -DCMAKE_BUILD_TYPE=Release
cmake --build build-native
```
Outputs generated in `build-native/src/`:
- `libdlssg_vulkan_route.so`
- `libdlssg_vulkan_proxy.so`
- `libdlssg_vulkan_ngx.so`

#### 2. Proton / Wine (`.dll` via MinGW cross-compilation)
```bash
./build.sh proton
# Or via CMake directly:
cmake -B build-proton -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-proton
```
Outputs generated in `build-proton/src/`:
- `dlssg_vulkan_route.dll`
- `dlssg_vulkan_proxy.dll`
- `dlssg_vulkan_ngx.dll`

### Proton / Steam Launch Options

When running games under Proton on Linux with these mods, place the DLLs next to the game executable and add:
```bash
WINEDLLOVERRIDES="version,dinput8=n,b" %command%
```

-------------------------------------------------------------

## Credits & Acknowledgements

- **sdii1995**: Original DLSS-G implementation
- **pipotoufikxyz-lgtm**: SM86 MFG modifications & Vulkan export surface
- **aethelisdev**: Linux Native (`.so`) & Proton/Wine (`.dll`) port, MinGW toolchain, and integration test suite
