# c2others
Calling C function complied in dll, from other programming languages

# A. Successful so far:
1. Python using ctypes package
2. Julia (https://julialang.org/) using in-build ccall function 
3. J programming langauge (www.jsoftware.com) using default imported'dll' package

# B. Not successful so far:
1. Excel VBA - due bit size issue (Excel 32 bits while DLL is 64 bits)
2. R programming language - will require some change in original C code before using R wrapper function
3. q programming language (www.kx.com)
4. Lua programming language (used Universal Luajit with "ffi" module)  
