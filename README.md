Problem Statement
You are given a maze. You enter the maze from outside in any cell, and start to explore it. 
あなたには迷路が与えられます。あなたはどこのセルからでも迷路に突入してもかまいません。 そして探索を始めます。
Each cell of the maze contains a relative operator which describes the direction in 
各セルには操作が記述されており、それぞれ方向を締めてしています
which you may leave the cell once you've entered it. There are 5 types of cells:
あなたが入ってから出るまで
'R' - turn 90 degrees right;
右に90度曲がる
'L' - turn 90 degrees left;
左に90度曲がる
'U' - turn 180 degrees (leave from the side of the cell from which you've entered);
180曲がる、最初にこれにぶつかるとすぐ出る
'S' - move forward;
前に進む
'E' - move everywhere (all leave directions are allowed).
任意の方向に進んで良い
So, you are given a maze, but you want to improve it and make it more interesting to explore. 
あなたには迷路が与えられ、そしてたくさん探索したいと願います
This is, you want to make as many maze cells reachable via paths as possible. 
そして出来るのであれば、なるべく多くのセルに到達したいと考えています
A path is a sequence of moves between cells which you can use to enter the maze from any border 
あなたは一度入ってから他の境界に突き当たるまで探索を続けます
cell and to exit it from another (or the same) border cell. 
A path can visit any cell only once, but different paths can visit same cell. Blue lines show valid paths on the image. 
各セルには1度しかたどり着けません。が複数のパスで同じセルに到達は出来ます。青いラインが有効な経路です
 

To modify the maze, you can use a limited number of maze fix operations. 
あなたは可能な限りこの迷路を修正することができます。
One fix changes the type of any one cell within the maze to any other type, 
Eのタイプを除いた他のセルの修正願いを出せます。
except for 'E'. You may not fix a cell that contains 'E'.
Eには修正できません！

Implementation Details
Your code should implement one method improve(vector <string>maze, int F). maze describes the maze itself: each character in maze will be 'R','L','U','S','E' for cells of the maze or '.' for a cell outside of the maze. F is the number of fixes you are allowed to use. Your return from this method should be a vector <string>, containing at most F elements. Each element of your return should be formatted as "R C T", where R and C are row and column coordinates of the cell you want to fix, and T is the new type of this cell.
Scoring
Your score for a test case will be calculated as the number of cells visited by at least one path, divided by the total number of cells in the maze ('.' cells are not counted). If your return contains an invalid fix or too many fixes, your score for this test case will be 0. Your overall score will be a sum of your scores for individual test cases.
Tools
An offline tester/visualizer is available here. You can use it to test/debug your solution locally. You can also check its source code for exact implementation of test case generation and score calculation.
 
Definition
      
Class:  MazeFixing
Method: improve
Parameters: vector <string>, int
Returns:  vector <string>
Method signature: vector <string> improve(vector <string> maze, int F)
(be sure your method is public)
    
 
Notes
- Each cell you change must be contained within the maze (you can't change '.' cells into maze cells). Both old and new type of cell must be one of: 'R','L','U' or 'S'.
- Borders of maze (first and last row and column) will contain only '.' cells.
- The width and height of the maze are chosen between 10 and 80, inclusive.
- The number of allowed fixes F will be between N/10 and N/3, where N is the number of non-'.' cells in the maze.
- The time limit is 10 seconds and the memory limit is 1024MB.
- There is 10 examples cases and 100 provisional cases.