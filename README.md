# MicroBuild
Quick, simple, cross platform project generator.

# Build State
<a href="https://scan.coverity.com/projects/tleonarduk-microbuild">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/9720/badge.svg?flat=1"/>
</a>
<a href="https://travis-ci.org/TLeonardUK/MicroBuild">
  <img alt="Travis CI"
       src="https://travis-ci.org/TLeonardUK/MicroBuild.svg?branch=master"/>
</a>
<a href="https://ci.appveyor.com/project/TLeonardUK/microbuild">
  <img alt="AppVeyor"
       src="https://ci.appveyor.com/api/projects/status/dufpylwdhvinr7m7?svg=true"/>
</a>

## Project Aims
MicroBuild aims to produce project files on all platforms it supports that match, as close as possible, the visual studio solution format. As this project is primarily created by game programmers its available functionality similarily reflects what would typically be required in game development.

It aims to  fulfills the following requirements:

+ __Fast__ - Project files should never take a long time to generate.
+ __No dependencies__ - Should be compilable off the bat on any standard C++ compiler, the project should also be capable of being distribued as a single dependency-less executable.
+ __Quick to setup__ - You should not have to dig into complex documents to setup simple projects. Just very simple ini-like files defining the properties required to build.
+ __Cross platform__ - Should be able to generate projects on any support platform for any other platform.

## Documentation
Extension documentation is available on our wiki page <a href="https://github.com/TLeonardUK/MicroBuild/wiki">Here</a>.
