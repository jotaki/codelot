#! ./bfi2

> # move to temp space (mem{1})
+ # increment to nonzero

# if temp space is 0; ignore this
[
 > # move to input memory (mem{2})
 . # print input

 [-] # ensure input memory is 0
 < # move to temp space
 [-] # ensure temp space is 0

 < # move back to workspace (mem{0})
 , # get input

 # copy to mem{1} and mem{2}
 [>+>+<<-]

 # move 10 into workspace (mem{0})
 ++++++++++

 [  # begin loop
  > # move to temp space
  - # decrement
  < # move to work space
  - # decrement
 ]  # repeat 10 times

 > # move to temp space;
] # done
