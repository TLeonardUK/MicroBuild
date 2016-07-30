# MicroBuild
Quick, simple, cross platform project generator.

# Build State
<a href="https://scan.coverity.com/projects/tleonarduk-microbuild">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9006/badge.svg"/>
</a>

## Project Aims
MicroBuild aims to produce project files on all platforms it supports that match, as close as possible, the visual studio solution format. As this project is primarily created by game programmers its available functionality similarily reflects what would typically be required in game development.

It aims to  fulfills the following requirements:

+ Fast - Project files should never take a long time to generate.
+ No dependencies - Should be compilable off the bat on any standard C++ compiler, the project should also be able to be run from just the executable file on its own.
+ Quick to setup - You should not have to dig into complex documents to setup simple projects.
+ Cross platform - Should be able to generate projects on any support platform for any other platform.

## Project file layout

+ Binaries
Compiled binaries for quick distribution.
+ Build
Files required to build the project, the bootstrapper folder in it contains the premake files used to compile the project before microbuild can self-host itself.
+ Source
The actual source code for the project.
+ Tests
Various projects to test the projects functionality.