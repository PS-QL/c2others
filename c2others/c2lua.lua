local ffi = require("ffi")
dllFile = ffi.load("fin_recipe.dll")

ffi.cdef("double cnd(double);")

-- Call the function.
io.write(dllFile.cnd(0.25) .. "\n")


-- error message while running this code
-- C:/Dev/Lua/ulua/pkg\1_0_beta10/init.lua:204: cannot load module 'fin_recipe.dll': %1 is not a valid Win32 application.
