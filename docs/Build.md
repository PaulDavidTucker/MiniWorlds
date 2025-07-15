### Build and Run docs

Requires:

- GCC 13
- Cmake 3.16
- SDL 3

## Build

Use Cmake to generate a build directory, cd into it, then build the project. Build options can be specified as required.
```bash
cmake -S . -B build
cd build
cmake --build .
```

Run the generated binary:
```bash
./MiniWorlds
```

The given executable runs at a 60fps cap by default, can be bumped up to a given value by passing in as a param. e.g.
```bash
./MiniWorlds 120
```
