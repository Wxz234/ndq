# ndq

![example workflow](https://github.com/Wxz234/ndq/actions/workflows/cmake-single-platform.yml/badge.svg)

This is a game framework. If you want to learn about Modules in C++20, you can use this library.

## Requirements

To successfully build and run this project, your development environment should meet the following requirements:

- Compiler : Visual Studio 2022 (17.4 or newer)
- Build System : CMake (3.28 RC2 or newer)
- Windows SDK : 10.0.17763.0 or newer

## Notice

IntelliSense might be unavailable. I can not solve it.

## Usage

Execute command:

```
git clone --recursive https://github.com/Wxz234/ndq
cd ndq
cmake -G "Visual Studio 17 2022" -A x64 -B build
```
Open the build folder,and execute ndq.sln.

## License

MIT license
