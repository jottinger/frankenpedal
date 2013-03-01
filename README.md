frankenpedal
============

Controller sketches for the frankenpedals

The goal is to get a valid reading from the pedals *every* time,
so I'm focusing on fixing readPin() to return the most accurate reading
I can.

It used to use this algorithm:

    do 
        read pin N times, storing into array dP
        sort dP in ascending order
        generate average of the lower 60% of dP
        generate stddev of lower 60% of dP
    while loop count < 5 and stddev is too high

The acceptable stddev is based on what the read value is,
since there's a built-in error tolerance on the resistors.

This was not especially fast, as the sort was a bit long.

Now, it uses a set of slots matching the number of input states
(14, for 13 pedals + a no-input state).

What it does now, in pseudocode, is:

    for(0..SAMPLE_COUNT):
       add one to dataPoint for pin read

    return index of dataPoint with highest count

Since the pins tend to return a bell curve around the 
actual pin closed (esp with the lower resistances, with the
greater error tolerances) we will generally get the right pin.    

It uses a FASTADC switch currently; this allows 60 reads in 
3 ms, with a pretty high accuracy when the stddev algorithm is
used. Not sure if I'm overengineering the reads; maybe I'd do better
with better hardware (resistors with lower error tolerances 
might fluctuate far less).