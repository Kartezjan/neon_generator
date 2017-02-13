# neon_generator
Generates light effects (neons) from pixels.

##Arguments:
1. input file (*.png files preferred)
2. output file (also *png)
3. pixel colour white list (pixels with different RGBA values are discarded - turned into (0,0,0,0) ) 
4. standard deviation (floating-point number)
5. radius towards X axis
6. radius towards Y axis
7. light intensity amplifier (how much more bright it is)

##White list file
Separate colours with return character (\n). Separate RGBA values with space. 

example:

0 255 255 255

255 0 0 255

255 0 255 255

0 255 174 255

255 0 228 255

0 198 255 255


##Example use

neon_generator.exe cyan_charge.png output.png whitelist.txt 4 11 11 200.0f

whitelist.txt is the same as in example above.

Original file:

![alt text](http://i.imgur.com/BTb2u8N.png "original file")

Output file:

![alt text](http://i.imgur.com/Vy1X1Tg.png "output file")
