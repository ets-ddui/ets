cl /I"%~dp0..\boost_1_64_0" /I"%~dp0.." /D "WIN32" /D "NDEBUG" /MD /EHsc asyn_io.cpp /link /LIBPATH:"%~dp0..\out\lib" "shell32.lib"
