
const dllfile = "fin_recipe"

function cnd(x) 
 ccall( 
     (:cnd, dllfile),    	# name of C function and library
	 Float64,				# output type
	 (Float64,),			# tuple of input types
	 x						# names of Julia variables to pass in
      )
end 

print( reshape(cnd.(-0.9:0.1:1), 4,5) ,"\n\n")


blackscholes(CP,S,X,T,r,v) =  ccall(  (:blackscholes, dllfile), Float64,	(Int32, Float64,Float64,Float64,Float64,Float64),  CP,S,X,T,r,v	) 

result = map( t -> blackscholes(1,100.0,100.0,t,0.1,0.3) , 1.0 : 10.0)

print(result,"\n --------------end of the program--------------\n")



