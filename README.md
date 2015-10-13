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
1つの経路において同じセルを使用してはいけませんが、異なるパスが同じセルを通過するのは大丈夫です。
 

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
 
 
Notes
- Each cell you change must be contained within the maze (you can't change '.' cells into maze cells). 
あなたは各セルの状態を変更することが出来ます。'.'は変更できません。
Both old and new type of cell must be one of: 'R','L','U' or 'S'.
古いまたは新しい状態に変更できるのは'R','L','U' or 'S'のみです
- Borders of maze (first and last row and column) will contain only '.' cells.
迷路の最初および最後の要素はかならず'.'のみで構成されています。
- The width and height of the maze are chosen between 10 and 80, inclusive.
迷路のサイズは10から80で構成されています。
- The number of allowed fixes F will be between N/10 and N/3, where N is the number of non-'.' cells in the maze.
Fの数はN/10からN/3の数で構成されています。Nは'.'ではないセルの数です
- The time limit is 10 seconds and the memory limit is 1024MB.
制限時間は10秒、メモリは1GBまでです
- There is 10 examples cases and 100 provisional cases.
exampleは10ケース、本番は100ケースあります。


# 考察

## 未知のものは何か

* 迷路をなるべく多く探索できるような、修正クエリの集合

## 与えられているもの（データ）は何か

* 迷路の情報と修正可能なセルの個数

## 条件は何か

* 空のセルを修正してはいけない、同じセルは1度しか辿ってはいけない（ただ一度の探索で出来た複数の経路については同じセルを使用しても良い）

## 条件の各部を分離せよ、それを書き表すことができるか

* この迷路から導ける最長の経路を求めよ。
** 最長経路の作成にはコストが大きすぎる。

## 同じ問題を少し違った形でみたことがあるか

* 今のところ無い

## 似た問題を知っているか

* 最長経路探索問題が昔合った気がする

## 似た問題でその結果を使うことが出来ないか

* 最長経路、つまりなるべく多くのセルを探索できるような経路を定義して、それに沿うようにセルを修正する
こと自体は可能だと思う。ただ、修正コストが高すぎるきがする。


# 計画

* 迷路から'U'を削除する
** 'U'の動作的に探索時に遭遇するとそこで探索が終了してしまうので、存在するのはあまり好ましくない
*** 消してみた結果、スコアが上昇した