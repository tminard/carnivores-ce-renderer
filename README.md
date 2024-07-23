# CarnivoresRenderer

A Carnivores 1, 2, and Ice Age renderer using C++ and OpenGL 3.

## Build dependencies
 - CMake (to download click [here](https://cmake.org/download/))
 - IDE (e.g. Visual Studio)

## Build instructions

1. Make sure that CMake is installed and added to the system PATH variable.
2. Open terminal in the CMakeLists.txt directory and run:
 ```
 mkdir build
 cd build
 cmake -G "Visual Studio 15" ..

 # Note that: VS 15 is VS 2017, 
 #            VS 14 is VS 2015, 
 #            VS 12 is VS 2013,
 #            VS 11 is VS 2012,
 #            VS 10 is VS 2010
 ```

## Mac OS X

- Make sure CMake is installed
- Make sure glfw and glew is installed

```sh
brew install glew glwf3
```

- Run cmake and open the xcode project

```sh
cmake -G "Xcode" -B build
```

We use OpenAL but it should be already included. LMK if you run into issues and we can update the install guide.

Note that resources are hardcoded to a specific path. All files are included in runtime but you will need to update paths.

## Config

Resources for runtime are in `runtime`. For first run, you will need to update `config.json` to point to where you want to load these resources from and which map you wish to test.

Example `runtime/config.json`:

```json
{
  "basePath": "C:\\src\\carnivores-ce-renderer\\runtime\\cce\\",
  "map": {
    "type": "C1",
    "map": "game\\c1\\area2.map",
    "rsc": "game\\c1\\area2.rsc"
  }
}
```

Update to your paths. E.g., for Mac, `basePath` may be `~/cce/`.
`map` is an object with a `map` and `rsc` reference to the map to load.

`type` is either `C1` or `C2` (Ice Age is included in `C2`). Set according to the map type.

No rebuild is needed to change the map.
