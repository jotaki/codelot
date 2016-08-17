# Collatz Conjecture

Simple app to produce chains from Collatz Conjecture.

# Sample output
```
$ ./cc
-10000000
Testing from G(1) to G(10000000):
Took 16.75s to complete 10000000 loops.
Found 8400511 with the longest chain count of 685
0 1
Switched to function g.
-10000000
Testing from g(1) to g(10000000):
Took 1.89s to complete 10000000 loops.
Found 8400511 with the longest chain count of 685
8400511
f(8400511)=25201534; g(8400511)=685
27
f(27)=82; g(27)=111
82
f(82)=41; g(82)=110
41
f(41)=124; g(41)=109
124
f(124)=62; g(124)=108
62
f(62)=31; g(62)=107
31
f(31)=94; g(31)=106
0 0
Switched to function G.
62
f(62)=31; G(62)=107
41
f(41)=124; G(41)=109
27
f(27)=82; G(27)=111
```
