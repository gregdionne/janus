# Janus
## _Optimal two-faced solving algorithm for the Rubik's Cube_
![Janus](https://upload.wikimedia.org/wikipedia/commons/c/c8/As_janus_rostrum_okretu_ciach.jpg)

### Description
Janus is an optimal "two-faced" solving algorithm based using either the quarter-turn or face-turn metrics for the Rubik's Cube published in the form developer notes and annotated C++ source code for reproducibility.

It is based upon the research of Herbert Kociemba, Michael Reid, and Richard Korf and includes an optimization suggested by Michiel de Bondt.

### Supported Metrics
Janus supports the quarter-turn and face-turn metrics.  The quarter-turn metric considers a move as a rotation of one of the six outer faces of the cube by 90&deg; in either a clockwise or counterclockwise direction. The face-turn metric (also called the half-turn metric) additionally allows a rotation by 180&deg;.

### Requirements
Janus requires a hardware and operating system capable of allocating 22 GB of free contiguous RAM for an in-memory database.  However, a command line option can be used to use a smaller table (11 GB) at the expense of longer solution times.

It was developed on a Mac mini (2018) with 64 GB 2667 MHz DDR4 RAM and a single 3.2 GHz 6-Core Intel Core i7.  Other hardware platforms may work, however these have not been tested.

### Performance
It takes about 10 seconds to load the database from the development platform's solid-state hard drive. Once the database is loaded, Janus typically finds the first solution to a random scramble in five to ten seconds and all remaining solutions in under a minute (for a typical 21 quarter-turn or 18 face-turn scramble).  Rare positions that require more than the typical number of moves can take five minutes.  Extremely rare positions like the "superflip" or "superflip-twist" can take a half-hour or so to completely exhaust the search space of the cube.  More modern computers should be considerably faster.

### Database
Janus creates a database for each move metric.  The database stores, for any arrangement of the cube, the number of moves (modulo three) required to completely solve any two opposing faces.  Once the database is created it is saved to disk for ready retrieval in the current directory.

Some attempt has been made to reduce the size of the database by symmetry of the positions of the four identical edges of one face and the four identical edges of the other face.  Instead of the 34,650 ways to encode the positions of the edges (12C4 &#183; 8C4), checking for symmetry gives 2,270 positions by any combination of color exchange, rotation, or inversion about the axes adjoining the opposing faces.  There are 2&#8312; ways to flip the eight edges, 8C4 ways to choose the positions of the four identical corners of one face and four identical corners of the other, and 3&#8311; ways to spin seven corners in-place where the last corner's spin is determined by the previous seven.  This gives a grand total of 2,270 &#183; 2&#8312; &#183; 8C4 &#183; 3&#8311; = 2,270 &#183; 256 &#183; 70 &#183; 2,187 = 88,963,660,000 positions in the table.

<table>
<tr><th>Quater-Turn Metric</th><th>Face-Turn Metric</th></tr>
<tr><td>

|depth | number of positions |
|-----:|--------------------:|
|     0|                    1|
|     1|                    1|
|     2|                    7|
|     3|                   52|
|     4|                  439|
|     5|                3,960|
|     6|               36,830|
|     7|              342,276|
|     8|          * 3,158,664|
|     9|         * 28,948,228|
|    10|        * 261,585,278|
|    11|      * 2,240,007,354|
|    12|     * 15,465,617,053|
|    13|     * 49,132,222,383|
|    14|       21,701,952,883|
|    15|          129,785,345|
|    16|                   53|
| total|       88,963,660,800|

</td><td>

|depth | number of positions |
|-----:|--------------------:|
|     0|                    1|
|     1|                    2|
|     2|                   15|
|     3|                  143|
|     4|                1,690|
|     5|               21,116|
|     6|              270,527|
|     7|            3,478,755|
|     8|         * 44,413,446|
|     9|        * 552,212,765|
|    10|      * 6,123,652,473|
|    11|     * 39,937,075,250|
|    12|       41,702,686,998|
|    13|          559,847,602|
|    14|                   20|
|    15|                    0|
| total|       88,963,660,800|
|     *| (over-estimate)     |

</td></tr> </table>

### &#274;n&#0257;r&#0275;s Database
Janus can optionally build a smaller database.  Instead of storing the number of moves to completely solve two opposing faces, the database allows for an exchange of the central cubes in the opposing faces.  When solving the cube, Janus is unable to distinguish between a solved cube and a "four-spot" pattern by using the database alone.  To verify a solution, Janus additionally keeps track of a completely cube to validate solutions, throwing away any "four-spot" patterns.  When compared to the regular database, above, the effective depth of the face-turn and quarter-turn metrics by about a half move.

&#274;n&#0257;r&#0275;s is the plural form of the Latin word, &#275;n&#0257;ris, meaning "without a nose."  As the Roman god Janus is often depicted with a head with two opposing faces, it was befitting to liken the center cubies to the noses of each face and to use Latin to specify table computation without the noses.
<table>
<tr><th>Quater-Turn Metric</th><th>Face-Turn Metric</th></tr>
<tr><th>(&#274;n&#0257;r&#0275;s)</th><th>(&#274;n&#0257;r&#0275;s)</th></tr>
<tr><td>

|depth | number of positions |
|-----:|--------------------:|
|     0|                    1|
|     1|                    1|
|     2|                    7|
|     3|                   53|
|     4|                  449|
|     5|                3,991|
|     6|               37,222|
|     7|              346,857|
|     8|          * 3,200,720|
|     9|         * 29,221,417|
|    10|        * 261,238,084|
|    11|      * 2,149,293,281|
|    12|     * 12,699,941,273|
|    13|     * 25,811,271,114|
|    14|        4,933,936,657|
|    15|            4,216,718|
|    16|                    4|
| total|       45,892,707,840|

</td><td>

|depth | number of positions |
|-----:|--------------------:|
|     0|                    1|
|     1|                    2|
|     2|                   16|
|     3|                  142|
|     4|                1,651|
|     5|               20,330|
|     6|              259,533|
|     7|            3,316,216|
|     8|         * 41,906,294|
|     9|        * 509,946,612|
|    10|      * 5,249,533,729|
|    11|     * 25,818,613,423|
|    12|       14,230,070,180|
|    13|           39,039,706|
|    14|                    9|
|    15|                    0|
| total|       45,892,707,840|
|     *| (over-estimate)     |

</td></tr> </table>

#### Comparison with Kociemba's Solver

As of this writing, Herbert Kociemba published notes only using the face-turn metric.

Since the face-turn metric database shown above (referred to as a pruning table on Kociemba's page) can make use of de Bondt's observation and has a large expected depth (45% of positions have a pruning value of 11, 47% have a pruning value of 12) an 18-twist random scramble often only needs a six-ply search before reaching the pruning table.

You can compare this database against the "Huge Solver" pruning table found on Kociemba's [Distribution Table](http://kociemba.org/math/distribution.htm).

### Algorithm

Once the database is constructed, it can be used to keep track of the minimum number of moves (twists) required to solve each of the three opposing pairs of faces independently, the maximum of which can be used as a lower bound for how many moves it would take to solve the entire cube.  Since twisting an arbitrary face from the solved state leaves both that face and its opposing face in a solved state, we can make use of Michiel de Bondt's observation and use n+1 as the lower bound to solve the entire cube when the minimum number of moves to solve each of the three opposing pairs of faces are all equal to n (when n is not zero).  This improves the speed of the algorithm by 35% or so.

To solve the cube, Janus attempts to solve the cube within a certain number of moves, called the "depth".  Janus first sets the depth to zero and checks to see if the cube is already solved.  If not, the depth is incremented by one.  When searching at a given depth, Janus makes a move then checks to see if the lower bound required to solve the faces of the cube exceeds the number of moves remaining.  If so, it backtracks and tries another move.  If no moves remain, then the cube is checked to see if it solved and it outputs the solution if so.  It then backtracks again to discover other solutions until the moves of the current depth are exhausted.  If no solutions were encountered, the depth is incremented by one and the process repeats.

More background on this approach can be found on Kociemba's [Optimal Solvers](http://kociemba.org/math/optimal.htm) page.

### Compiling

To compile on macOS (darwin), make sure you have Xcode installed with command line tools and then type "make".

If you're a linux user, it should be compatible with clang (c++14), however it hasn't been tested.  Volunteers for other operating systems are gratefully accepted -- especially for those familiar with how Windows manages large memory requests.

### Usage

#### Command-line terminal program

Janus has been wrapped for use in a simple command-line terminal program.

You scramble the cube much as in the way you do in real life: by twisting it!  Enter the moves using [Singmaster notation](https://rubiks.fandom.com/wiki/Notation).

When you execute it for the first time it will attempt to make a database "depthTable-FTM.janus" in your current working directory.  Make sure you have at least 22 GB RAM and storage space.  It takes about fifteen minutes to create on a Mac mini (2018) with 64 GB memory and a single 3.2 GHz 6-Core Intel Core i7.  Next time you execute it, it will read from this file.

```
$ janus
reading depthTable-FTM.janus... 22,240,915,200 bytes read
Enter scramble in Singmaster notation (Ctrl+D to exit):
```

Enter your scramble and then press return (you'll probably need at least 17 moves to make it interesting).  For example:

```
$ janus
reading depthTable-FTM.janus... 22,240,915,200 bytes read
Enter scramble in Singmaster notation (Ctrl+D to exit):
L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'
Solving Scramble: L B' L' F2 U F R2 U2 F U' F2 R2 F2 U' L2 U2 B' R'
Searching depth 1...
Searching depth 2...
Searching depth 3...
Searching depth 4...
Searching depth 5...
Searching depth 6...
Searching depth 7...
Searching depth 8...
Searching depth 9...
Searching depth 10...
Searching depth 11...
Searching depth 12...
Searching depth 13...
Searching depth 14...
Searching depth 15...
Searching depth 16...
Searching depth 17...
Searching depth 18...
minimal 18-move (face turn) solution(s) found:
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
You can also pipe in a file with many scrambles in it.  You can find the 100-move random scrambles that Ben Botto used for his very well-written [Rubik's Cube Cracker](https://github.com/benbotto/rubiks-cube-cracker) in the "tests" folder.  Janus takes a little over six minutes to enumerate all minimal-length solutions for the scrambles.

```
$ janus < tests/benbotto.txt
```

If you'd rather just solve one scramble right from the command line, you can enter it in quotes.  As an example you can try the "distance-20" scramble labeled "The hardest position for our programs." from [God's Number is 20](http://cube20.org).  There are 96 non-trivial solutions, which took thirty-six minutes to traverse through the entire state space of the cube.
```
$ janus "F U' F2 D' B U R' F' L D' R' U' L U B' D2 R' F U2 D2"
```

By default, Janus reports solutions using the face-turn metric.  To use the quarter-turn metric, use the "-qtm" option
as the first argument:

```
$ janus -qtm "F U' F2 D' B U R' F' L D' R' U' L U B' D2 R' F U2 D2"
reading depthTable-QTM.janus... 22,240,915,200 bytes read
```

Janus uses "depthTable-QTM.janus" as the filename for the quater-turn metric.

If you wish to the smaller &#275;n&#0257;r&#0275;s database, add the "-enares" option.  Janus uses "depthTable-FTM-enares.janus" and "depthTable-QTM-enares.janus" for the face-turn and quater-turn metrics, respectively.

```
$ janus -enares "F U' F2 D' B U R' F' L D' R' U' L U B' D2 R' F U2 D2"
$ janus -qtm -enares "F U' F2 D' B U R' F' L D' R' U' L U B' D2 R' F U2 D2"
reading depthTable-QTM-enares.janus...11,473,176,960 bytes read
```
#### Solutions
Janus attempts to find all minimal solutions.  To save time, Janus avoids "trivially equivalent" pairs of moves.

If you look carefully at the solutions you may find that some pairs of moves are placed inside parentheses.  The moves within each pair twist opposing sides of the cube (e.g. front and back) and may be entered in either order without affecting the solution.  To save time, Janus treats such pairs as trivially equivalent and always reports a front twist before a back, a right twist before a left, and an up twist before a down.  So, in [Singmaster notation](https://rubiks.fandom.com/wiki/Notation), Janus displays a single solution to a scramble like  **F B2 U L' R** as **(R' L) U' (F' B2)** instead of four distinct solutions: **_R' L_** **U'** **_F' B2_**, **_R' L_** **U'** **_B2 F'_**, **_L R'_** **U'** **_F' B2_**, and **_L R'_** **U'** **_B2 F'_**

#### Sample TCP Server
To facilitate running a separate front-end program, Janus includes a sample TCP server that wraps its functionality.
To obtain help for it, use the '-help' option
```
$ janus_server -help
usage: janus_server -help
       janus_server -help <option>
       janus_server [-qtm] [-enares]
                      port

DETAILS
  port:  port id to host (e.g., 3490)
  janus_server hosts a TCP server on the specified port.

  To use the server, connect to it via TCP.

  The server replies 'ready' whenever it can accept a new command.

  valid commands are "help", "metric", "abort", "solve", or "exit".

    help
      prints this help message

    metric
      prints the current move metric (face-turn or quater-turn).
      The quater-turn metric can be invoked via the "-q" option switch
      when starting the server from the command line.

    abort
      stops any solution in progress.

    solve  <moves>
      prints all minimal solutions using the current metric
      valid moves are entered in Singmaster notation:
        F  R  U  B  L  D  (clockwise moves)
        F' R' U' B' L' D' (counter-clockwise moves)
        F2 R2 U2 B2 L2 D2 (half-turn moves)

      When reporting solutions, some pairs of moves are placed
      within parentheses. The moves within each pair twist
      opposing faces of the cube (e.g. front and back) and may
      be entered in either order without affacting the solution.

    exit
      closes the program

OPTIONS

 -qtm          Use the quater-turn metric.

 -enares       Use a reduced depth table of 11GB instead of 22GB.

EXAMPLE

  In one terminal, start the server by entering:
    janus_server 3490

  Connect to the server via TCP from your own front-end program.
  For example, using a program like netcat (nc) enter:
    nc 127.0.0.1 3490

  Janus will reply with 'ready'.  Then enter:
    solve F R U U F U F L B D U F D B L U D F F U

  Janus will reply with:
    solving scramble: "F R U U F U F L B D U F D B L U D F F U"
    ready

  At this point the server may told to abort or solve a new scramble.
  Otherwise, it reports solutions as it finds them in the following format:
    searching depth 0...
    searching depth 1...
    searching depth 2...
    searching depth 3...
    searching depth 4...
    searching depth 5...
    searching depth 6...
    searching depth 7...
    searching depth 8...
    searching depth 9...
    searching depth 10...
    searching depth 11...
    searching depth 12...
    searching depth 13...
    searching depth 14...
    searching depth 15...
    searching depth 16...
    searching depth 17...
    searching depth 18...
    minimal 18-move (face turn) solution(s) found:
    solution 1: R B' D L2 F' D B L' U' F' U L' (U D2) (R' L2) (F B2)
    solution 2: U' F2 (U' D') L' B' D' F' (U' D') B' L' F' U' F' U2 R' F'
    solution 3: D' L U F' U' B' R' (U D) R2 D2 L2 D' R F2 U R U
    solution 4: R2 F' D2 R F2 L' B L' D' F2 D L2 F D2 B2 L2 D B'
    solution 5: (R2 L2) U F' U2 L U2 R' B U R B2 R F R2 F2 U R2
    solution 6: U2 F2 R2 U F U2 R' U2 F2 L D R U2 F' (R L2) F D
    search complete
```

To start it, choose a port number (say, 3490).

```
$ janus_server 3490
Initializing...
reading depthTable-FTM.janus... 43,355,088,000 bytes read
server: waiting for connection...
```

The server will then listen for connections on the specified port. To facilitate debugging, it will echo output to the terminal as well as the command output.

A program like netcat (nc) can be used to connect to it on your network from another terminal window.  Here is sample usage when the server is started on the same computer:

```
$ nc 127.0.0.1 3490
ready
```

The server outputs "ready" whenever it can accept a command from the connection.

A typical front-end client can connect to the server, wait for "ready" and then enter the solve commands whenever a solution is desired.  Solutions are output whenever they are encountered on the server.  It is only necessary to wait for the "ready" prompt before issuing a new solve command:  any previous solve in progress will be automatically aborted.  

For example if the client enters:
```
solve F R U U F U F L B D U F D B L U D F F U D L L F B B F U F D B L R R F L D B
```

Janus will first respond with:
```
solving scramble "F R U U F U F L B D U F D B L U D F F U D L L F B B F U F D B L R R F L D B"
ready
```
The client is expected to parse the output as it becomes available.

When issuing a 'solve' command, the client can parse the first line to know the scramble has been received.  The 'ready' on the second line can be parsed by the client so it knows it can enter other commands even when the server is solving the scramble.

Note that the server:
1. reports when it searches at a new depth level.
2. outputs the number of moves and the corresponding metric when an optimal solution is discovered
3. prefixes solutions with 'solution \<number\>:'
4. issues 'search complete' once all solutions are generated.

A front-end client can then report progress of the solver as well as solutions as they arise.

The front-end client can send the 'exit' command to close the connection with the server.
## Notices

Copyright (C) 2021-2022 Greg Dionne.
Distributed under MIT License

Rubik's Cube is a registered trademark of Seven Towns, Ltd.
