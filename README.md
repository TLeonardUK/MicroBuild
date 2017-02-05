# MicroBuild
Microbuild is a simple, fast, cross platform, build system.

# Build State
<a href="https://travis-ci.org/TwinDrills/MicroBuild">
  <img alt="Travis CI"
       src="https://travis-ci.org/TwinDrills/MicroBuild.svg?branch=master"/>
</a>
<a href="https://ci.appveyor.com/project/TwinDrills/microbuild">
  <img alt="AppVeyor"
       src="https://ci.appveyor.com/api/projects/status/dufpylwdhvinr7m7?svg=true"/>
</a>

## Project Aims
MicroBuild is designed to be a general purpose C-Language (C++/C#/ObjC) build system. It has the ability to generate project and workspace files on each platform for all IDE's it supports (currently Visual Studio, XCode and GNU Makefiles), as well as to build the projects for a variety of platforms (Desktop, HTML5, Consoles, etc). It also supports the ability to automate packaging of projects using the target platforms native packaging toolset.

Microbuild takes all its configuration from simple and intuitive INI style files, designed to be simpler and more straight-forward to get running than some of the alternative build systems (CMake, PreMake, etc).

Its primary aims are:

+ __Fast__ - Generation and build times should be minimal, at its core MicroBuild should act as a wrapper for native toolchains and should take no longer to build or generate files than building them natively would take.
+ __Minimal Dependencies__ - Should be compilable off the bat on any standard-compliant C++11 compiler, the project should also be capable of being distribued as a single executable.
+ __Intuitive__ - Project configuration should be quick and intuitive, the user should never have to spend any degree of time fine-tuning configuration files.
+ __Cross Platform__ - Should be compilable and usable on any standard desktop architecture - Windows/Linux/MacOS.
+ __Plugin Support__ - Shared libraries using a plugin framework can be easily created and loaded into microbuild, allow for extensions to the build system. This is used in other projects for doing things such as generating meta-data, or syntax checking.

## Supported IDEs
We currently support generating project and workspace files for the following IDE's.

| Target IDE             | Status          |
| ---------------------- | --------------- |
| Visual Studio 2015     | Supported       |
| Visual Studio 2017     | Supported       |
| GNU Makefiles          | Supported       |
| XCode                  | Supported       |
| MonoDevelop            | In Progress     |
| Code::Blocks           | In Progress     |

## Supported Platforms
We currently support building code for the following platforms. Support for each platform depends on the the toolchain being available on the host platform (eg. Console platforms are unlikely to be available on non-windows platforms).

| Target IDE               | Status                                       |
| ------------------------ | -------------------------------------------- |
| Windows (x86/x64)        | Supported; Using Clang, GCC or MSBuild       |
| Windows RT (x86/x64/ARM) | In Progress                                  |
| Linux (x86/x64)          | Supported; Using Clang or GCC                |
| MacOS (x86/x64)          | Supported; Using Clang or GCC                |
| HTML5 (Emscripten)       | Supported                                    |
| Android (NDK)            | Supported                                    |
| Nintendo 3DS             | Supported                                    |
| Nintendo WiiU            | Supported                                    |
| iOS                      | In Progress                                  |
| Playstation 3            | In Progress                                  |
| Playstation 4            | In Progress                                  |
| Playstation Vita         | In Progress                                  |
| Xbox 360                 | In Progress                                  |
| Xbox One                 | In Progress                                  |

## Packaging Support
We currently support packaging projects for the following targets:

| Target IDE                 | Status                                       |
| -------------------------- | -------------------------------------------- |
| Desktop Zip Package        | In Progress                                  |
| Desktop Installer Package  | In Progress                                  |
| Steam Depot                | In Progress                                  |
| MacOS AppStore             | In Progress                                  |
| iOS AppStore               | In Progress                                  |
| HTML5 Webpage              | In Progress                                  |
| Android Apk                | In Progress                                  |
| Linux .deb Package         | In Progress                                  |
| Nintendo 3ds ROM           | In Progress                                  |
| Nintendo WiU ROM           | In Progress                                  |
| Playstation 3 PKG          | In Progress                                  |
| Playstation 4 PKG          | In Progress                                  |
| Playstation Vita PKG       | In Progress                                  |
| Xbox 360 ISO               | In Progress                                  |
| Xbox One ISO               | In Progress                                  |

## Documentation
Extensive documentation is available on our wiki page <a href="https://github.com/TLeonardUK/MicroBuild/wiki">Here</a>.

## License
MicroBuild is free software and is distributed under GPL (version 3). If you need distribution under another license, shoot me a message.
