cc -fPIC -shared -o fin_recipe.dll fin_recipe_source.c

rem cc -fPIC -shared -o fin_recipe.dll test.c
size fin_recipe.dll

strip --strip-all fin_recipe.dll
upx -qq fin_recipe.dll

