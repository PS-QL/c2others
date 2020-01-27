from ctypes import *

# linking dll file
bs = CDLL('fin_recipe.dll')

# European Option
def blackscholes(CP,S,X,T,r,v):
    CP = c_int(CP)
    S = c_double(S)
    X = c_double(X)
    T = c_double(T)
    r = c_double(r)
    v = c_double(v)
    
    # By default functions are assumed to return the C int type. 
    # Other return types can be specified by setting the restype attribute of the function object.
    bs.blackscholes.restype = c_double

    # calling a c function
    return bs.blackscholes(CP,S,X,T,r,v)

print("European option value is : ", blackscholes(1,100,100,5.0,0.1,0.3)  )


# American Option
def BSAmericanApprox(CP,S,X,T,r,b,v):
    CP = c_int(CP)
    S = c_double(S)
    X = c_double(X)
    T = c_double(T)
    r = c_double(r)
    b = c_double(b)  # not in european option
    v = c_double(v)
    
    bs.BSAmericanApprox.restype = c_double

    return bs.BSAmericanApprox(CP,S,X,T,r,b,v)

result = map(lambda x : BSAmericanApprox(1,42,40,x,0.04,-0.04,0.35), [0.25,0.50,0.75,1.0,2.0]  )    

print("American option value is : ", list(result)  )
    
#print("American option value is : ", BSAmericanApprox(1,42,40,0.75,0.04,-0.04,0.35)  )

