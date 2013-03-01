frankenpedal
============

Controller sketches for the frankenpedals

Basically, all the work is in midi_debug right now. 
The goal is to get a valid reading from the pedals *every* time,
so I'm focusing on fixing readPin() to return the most accurate reading
I can.

Right now it does so via this algorithm:

    do 
        read pin N times, storing into array dP
        sort dP in ascending order
        generate average of the lower 80% of dP
        generate stddev of lower 80% of dP
    while loop count < 5 and stddev is too high

The acceptable stddev is based on what the read value is,
since there's a built-in error tolerance on the resistors.