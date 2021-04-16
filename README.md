# Project 3: Parallel Cryptocurrency Miner

Professor: Matthew Malensek\
Author: Marisa Tania  

## Program Overview 
Cryptocurrencies like Bitcoin rely on a distributed transaction ledger composed of blocks. The job of miners in the Bitcoin network is to verify transactions within these blocks, which is computationally expensive. Given a complete sequential program, we want to must parallelize it using the pthreads library. Because of the inherent randomness of cryptocurrency mining, using more threads will improve the probability of finding a solution in a shorter amount of time.

Bitcoin mining (and many others) are based on Hashcash. The idea behind these systems is called proof-of-work, which is somewhat like a CAPTCHA designed for computers instead of humans. The computer works on a computationally expensive problem to prove its actions are legitimate; in the case of Hashcash, performing the computation proves that you are not a spammer, while in Bitcoin it serves to verify transactions as being valid. A key feature of proof-of-work systems is that once the solution to the problem is found, it is trivial to verify that the answer is correct.

In Bitcoin, hash inversions are the computationally expensive problem being solved by the computer. Given a block, the algorithm tries to find a nonce (number only used once) that when combined with the block data produces a hash code with a set amount of leading zeros. The more leading zeros requested, the harder the problem is to solve. This program searches for the nonce that satisfies a given difficulty level by splitting the work across multiple threads.

### Demo Run
Here’s a demo run for the completed, parallel version of the program:
```bash
./miner 4 24 'Hello CS 521!!!'

Number of threads: 4
  Difficulty Mask: 00000000000000000000000011111111
       Block data: [Hello CS 521!!!]

----------- Starting up miner threads!  -----------

Solution found by thread 3:
Nonce: 10211906
 Hash: 0000001209850F7AB3EC055248EE4F1B032D39D0
10221196 hashes in 0.26s (39312292.30 hashes/sec)
```
In this example, 4 threads are used to find the solution to the block: the nonce that satisfies the given difficulty (24 leading zeros in this case). 

### Verification
To verify that the solution is correct is on the UNIX command line, we can test this with the sha1sum utility. In this example, block data was 'Hello CS 521!!!' and the nonce of the solution you found was 10211906 with a difficulty level of 24. 

```bash
echo -n 'Hello CS 521!!!10211906' | sha1sum
0000001209850f7ab3ec055248ee4f1b032d39d0
```
Note that the resulting hash has 6 zeros, which is what we’d expect: 24 bit difficulty means 6 hex characters worth of zeros (24 / 4 = 6).

### Included Files
There are several files included. These are:
   - <b>Makefile</b>: Including to compile and run the program.
   - <b>miner.c</b>: The program driver.
   - <b>sha1.c</b>: Contains shell history data structures and retrieval functions.
   - <b>ui.c</b>: Text-based UI functionality, primarily concerned with interacting with the readline library.
   - <b>util.c</b>: Util to tokenize the command input.
   - <b>job.c</b>: Background jobs functionality.
   - <b>signal.c</b>: Signal functionality.
   - <b>clist.c</b>: Circular list data structure to store the history.
   - <b>elist.h</b>: Elastic list data structure built from scratch.
   - <b>logger.h</b>: Helps facilitate debugging by providing basic logging functionality. 
There are also header files mine.h, sha1.h, signal.h, history.h, clist.h and ui.h.


To compile and run:

```bash
make
./jellyfish
```

### Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```

See spec here: https://www.cs.usfca.edu/~mmalensek/cs521/assignments/project-3.html


