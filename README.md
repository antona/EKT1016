Example program to exercise and use the ElectronicKit.com EKT-1016 Power Digital Shield:

Here are a sketch that will work without modifications with the EKT-1016 card (EKT1016 Tindie).
It will run in 1 of 4 different stepping modes for both bipolar and unipolar stepper motors.

Your code invokes the appropriate forward/backward routine to move 1 step ahead/back.  

This sketch works with the factory default jumper setting on the EKT-1016.

Note that the user turns a power driver output ON by setting its port binary bit to a '0' and OFF with a '1'.
