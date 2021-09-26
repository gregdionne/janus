# Janus
## _Optimal two-faced solving algorithm for the Rubik's Cube_
![Janus](https://upload.wikimedia.org/wikipedia/commons/c/c8/As_janus_rostrum_okretu_ciach.jpg)

### Description
Janus is an optimal "two-faced" solving algorithm based on the half-turn (twist) metric for the Rubik's Cube published in the form developer notes and annotated C++ source code for reproducibility.  

It is based upon the research of Herbert Kociemba, Michael Reid, and Richard Korf and includes an optimization suggested by Michiel de Bondt.  The background of this approach can be found on Kociemba's [Optimal Solvers](http://kociemba.org/math/optimal.htm) page.

### Requirements
Janus requires a hardware and operating system capable of allocating 45 GB of free contiguous RAM for an in-memory database.

It was developed on a Mac mini (2018) with 64 GB 2667 MHz DDR4 RAM and a single 3.2 GHz 6-Core Intel Core i7.  Other hardware platforms may work, however these have not been tested.


### Performance
It takes about 20 seconds to load the database from the development platform's solid-state hard drive. Once the database is loaded, Janus typically finds the first solution to a random scramble in five to ten seconds and all remaining solutions in under a minute  (for an 18-twist solution).  19-twist solutions can take a minute to solve and five minutes to find all remaining solutions.  It can take a half-hour to completely exhaust the search space of a 20-twist solution.

### Algorithm
Janus solves the cube by first creating (or retrieving from disk) a database that, for any arrangement of the cube, reports the number of twists required to completely solve any two opposing faces, allowing an exchange of the two opposing center pieces.  Once the database is computed it is saved to disk for ready retrieval.  It then performs a triple search in parallel for each of the three axis directions (front-back, left-right, up-down).  When the parallel search is complete, the cube is either solved, or in a "four-spot" pattern (e.g., using Singmaster notation F2 B2 U D' R2 L2 U D').  The solution is checked to see if it is complete, otherwise it discards the four-spot pattern and continues.

Since the database (referred to as a pruning table on Kociemba's page) can make use of de Bondt's observation and has a large expected depth (56% of positions have a pruning value of 11, 31% have a pruning value of 12) an 18-twist random scramble often only needs a six-ply search before reaching the pruning table.

### Database
Some attempt has been made to reduce the size of the database by symmetry of the positions of the four identical edges of one face and the four identical edges of the other face.  Instead of the 34,650 ways to encode the positions of the edges (12C4 &#183; 8C4), checking for symmetry gives 2,256 positions (9, 12, 147, 2088 positions with 2-, 4-, 8-, and 16-way equivalent symmetric positions, respectively by any combination of reflection, inversion, and rotation about the axis joining the opposing faces).  There are 2&#8312; ways to flip the eight edges, 8C4 ways to choose the positions of the four identical corners of one face and four identical corners of the other, and 3&#8311; ways to spin seven corners in-place where the last corner's spin is determined by the previous seven.  This gives a grand total of 2,256 &#183; 2&#8312; &#183; 8C4 &#183; 3&#8311; = 2,256 &#183; 256 &#183; 70 &#183; 2,187 = 88,414,986,240 positions in the table.

|depth | number of positions |
|-----:|--------------------:|
|     0|                    1|
|     1|                    2|
|     2|                   23|
|     3|                  243|
|     4|                3,058|
|     5|               38,740|
|     6|              498,741|
|     7|            6,391,361|
|     8|           80,840,150|
|     9|          983,980,165|
|    10|       10,130,194,776|
|    11|       49,793,425,015|
|    12|       27,347,009,964|
|    13|           72,603,988|
|    14|                   13|
|    15|                    0|
| total|       88,414,986,240|

You can compare this database against the "Huge Solver" pruning table found on Kociemba's [Distrubution Table](http://kociemba.org/math/distribution.htm).

### Compiling

To compile on macOS (darwin), make sure you have Xcode installed with command line tools and then type "make".

If you're a linux user, it should be compatible with clang (c++14), however it hasn't been tested.  Volunteers for other operating systems are gratefully accepted -- especially for those familiar with how Windows manages large memory requests.

### Usage

Janus has been wrapped for use in a simple command-line terminal program.

You scramble the cube much as in the way you do in real life: by twisting it!  Enter the moves using [Singmaster notation](https://rubiks.fandom.com/wiki/Notation).

When you execute it for the first time it will attempt to make a database "depthtable.janus" in your current working directory.  Make sure you have at least 45GB RAM and storage space.  It takes about forty-five minutes to create on a Mac mini (2018) with 64 GB memory and a single 3.2 GHz 6-Core Intel Core i7.  Next time you execute it, it will read from this file.

```
$ janus
reading depthTable.janus...88414986240 positions read
Enter scramble in Singmaster notation (Ctrl+D to exit):
```

Enter your scramble and then press return (you'll probably need at least 17 moves to make it interesting).  For example:

```
$ janus
reading depthTable.janus...88414986240 positions read
Enter scramble in Singmaster notation (Ctrl+D to exit):
L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'
Solving Scramble: L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R' 
Trying depth 1...
Trying depth 2...
Trying depth 3...
Trying depth 4...
Trying depth 5...
Trying depth 6...
Trying depth 7...
Trying depth 8...
Trying depth 9...
Trying depth 10...
Trying depth 11...
Trying depth 12...
Trying depth 13...
Trying depth 14...
Trying depth 15...
Trying depth 16...
Trying depth 17...
Trying depth 18...
minimal 18-move solution(s) found:
 1: D F U F2 R2 B2 D' B L' F R2 B R2 (F B2) L' U' L' 
 2: D F U F2 R2 B2 D' B L' B U2 B U2 (F2 B) L' U' L' 
 3: D F U F2 R2 B2 D' B L' (F' B2) L2 B L2 F' L' U' L' 
 4: D F U F2 R2 B2 D' B L' (F2 B') D2 B D2 B' L' U' L' 
 5: F R D B' U2 B2 U2 F L2 D' F' L2 D F2 R2 (F' B') D2 
 6: U' B L2 B R' F2 D' B L2 B D R U R2 U2 B' U' R2 
 7: R B U2 L2 U F2 R2 F2 U F' U2 R2 F' U' F2 L B L' 
 8: U R' U2 L2 B' R2 D F2 (R L2) B R2 U R2 D' R (U' D2) 
 9: (U' D') F2 D F' R2 (F B2) D (F' B') L2 D2 L' F' (R2 L2) F2 
10: D' L' U2 F R' B' D B' U B' L' B D B2 R2 D' B2 R 
11: D' F2 R F' (R2 L) (F2 B2) (U2 D') R' F2 L2 B2 L2 U L' U2 
12: B2 U2 L' F R' U L' U' B L2 F R2 F (R' L) F2 (U' D2) 
13: F2 L2 D F D' F L' D F2 L' D2 F L2 D2 R' (U D) L 
14: (R' L2) B' U2 B' U F U L' (F2 B2) L' U2 R' U2 F U B' 
Enter scramble in Singmaster notation (Ctrl+D to exit):
```
You can also pipe in a file with many scrambles in it.  You can find the 100-move random scrambles that Ben Botto used for his very well-written [Rubik's Cube Cracker](https://github.com/benbotto/rubiks-cube-cracker) in the "tests" folder.  Janus takes a little over six minutes to exhaustively solve the scrambles.

```
$ janus < tests/benbotto.txt
```

If you'd rather just solve one scramble right from the command line, you can enter it in quotes.  As an example you can try the "distance-20" scramble labeled "The hardest position for our programs." from [God's Number is 20](http://cube20.org).  There are 96 non-trival solutions, which took a half-hour to traverse through the entire state space of the cube.
```
$ janus "F U' F2 D' B U R' F' L D' R' U' L U B' D2 R' F U2 D2"
```
### Solutions
Janus attempts to find all minimal solutions.  To save time, Janus avoids "trivially equivalent" pairs of moves.  

If you look carefully at the solutions you'll find that some pairs of moves are placed inside parentheses.  The moves within each pair twist opposing sides of the cube (e.g. front and back) and may be entered in either order without affecting the solution.  To save time, Janus treats such pairs as trivially equivalent and always reports a front twist before a back, a right twist before a left, and an up twist before a down.  So, in [Singmaster notation](https://rubiks.fandom.com/wiki/Notation), Janus displays a single solution to a scramble like  **F B2 U L' R** as **(R' L) U' (F' B2)** instead of four distinct solutions: **_R' L_** **U'** **_F' B2_** , **_R' L_** **U'** **_B2 F'_**, **_L R'_** **U'** **_F' B2_**, and **_L R'_** **U'** **_B2 F'_**

## Notices

Copyright (C) 2021 Greg Dionne.
Distributed under MIT License

Rubik's Cube is a registered trademark of Seven Towns, Ltd.
