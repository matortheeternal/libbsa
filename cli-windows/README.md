## Setting up to compile on Windows

1. This blog post is a good resource for building Boost on Windows: [Codeyarns](https://codeyarns.com/2014/06/06/how-to-build-boost-using-visual-studio/)
2. Make sure you get v1.59 or 1.60 of Boost if you're using VS2015.
3. Run bootstrap.bat
4. Run the following commands from the Developer Command Prompt for Visual Studio.  The first command builds release libraries and the second builds debug libraries.  If you want to be able to debug your project you need to build the debug libraries!

b2 toolset=msvc-14.0 link=static variant=release architecture=x86 address-model=32 --with-iostreams --with-filesystem --with-locale --with-regex --with-system --stagedir=stage-mingw-32

b2 toolset=msvc-14.0 link=static variant=debug runtime-debugging=on architecture=x86 address-model=32 --with-iostreams --with-filesystem --with-regex --with-locale --with-system --stagedir=stage-mingw-32

5. Right-click the project in visual studio and choose options, then add the directories as instructed from the command prompt.  Note that the current libbsa project already has directories added which are absolute paths for my development environment.  You'll want to replace these paths with the paths you're using for your environment!

Compiler include paths can be found in: Configuration Properties > C/C++ > General > Additional Include Directories
Linker library paths can be found in: Configuration Properties > Linker > General > Additional Library Directories

6. You should make and compile zlib as a separate Visual Studio project, and then require it.  I'll have a repository for this up soon.