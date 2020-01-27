require 'dll'
lib =. dquote 'fin_recipe.dll'
bs =. (lib,' blackscholes > d i d d d d d') & cd
bs 1 100 100 5.0 0.1 0.3
