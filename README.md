frankenpedal
============

Controller sketches for the frankenpedals

The goal is to get a valid reading from the pedals *every* time,
so I'm focusing on fixing readPin() to return the most accurate reading
I can.

Right now it does so via this algorithm:

    do 
        read pin N times, storing into array dP
        sort dP in ascending order
        generate average of the lower 60% of dP
        generate stddev of lower 60% of dP
    while loop count < 5 and stddev is too high

The acceptable stddev is based on what the read value is,
since there's a built-in error tolerance on the resistors.

It uses a FASTADC switch currently; this allows 60 reads in 
3 ms, with a pretty high accuracy when the stddev algorithm is
used. Not sure if I'm overengineering the reads; maybe I'd do better
with better hardware (resistors with lower error tolerances 
might fluctuate far less).