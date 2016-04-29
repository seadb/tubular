   blockid: 0 val_copy 22 s14 
   blockid: 0 val_copy s14 s9 
   blockid: 0 val_copy 10 s15 
   blockid: 0 val_copy s15 s10 
   blockid: 0 val_copy s9 s12 
   blockid: 0 val_copy s10 s13 
   blockid: 0 val_copy return_point0 s5 
   blockid: 0 jump function_sum 
return_point0:  blockid: 0 
   blockid: 1 out_int s6 
   blockid: 1 out_char '\n'             # End print statements with a newline.
   blockid: 1 jump function_sum_end 
function_sum:  blockid: 1 
   blockid: 2 add s12 s13 s16 
   blockid: 2 val_copy s16 s6 
   blockid: 2 jump s5 
function_sum_end:  blockid: 2 
