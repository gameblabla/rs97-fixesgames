Spinng World:

Large Object File Demo                                                   
Copyright 2001, Joe Zbiciak.                                             
                                                                         
This demonstration shows how to handle programs that are larger than 8K. 
The default memory map maps the first 8K of a program at $5000-$6FFF.    
Beyond that, for 16K carts, you get a 4K block from $D000-$DFFF and a 4K 
block from $F000-$FFFF.                                                  
                                                                         
This causes a problem for cartridges that want to be larger than 8K.     
One way to handle this is to try to divide your cartridge into multiple  
source files and assemble them separately.  This doesn't work very well. 
The other is to tweak your existing assembly source file until it fits   
the memory map.  That's what this program does.                          
                                                                         
The demo below copies alot of data into the GRAM from the cartridge.     
There is significantly more than 8K of data in the array.  The array     
itself is segmented into frames, and each frame must be contiguous       
within itself.  Otherwise, the data can be placed anywhere.              
                                                                         
If you assemble this file with just a single ORG statement at the top    
(eg. "ORG $5000"), a portion of the ROM image will spill into the        
$7000 range.  If you look at the listing file, you will see that         
WORLD.frame_27 is the frame that actually crosses the 8K ROM boundary.   
                                                                         
If we load this ROM image as-is in the default config, part of Frame     
27 will be above location $7000, which will confuse the EXEC's bootup    
sequence.  That's not good.  If you try to run the code, you'll see      
that it doesn't run at all.                                              
                                                                         
To fix this, we need to add an ORG statement to force frame 27 into      
a different ROM segment than $7000.  I recommend $D000.                  
                                                                         
Try it.  Experiment.  Consider this an "active" example.                 

