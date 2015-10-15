#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/time.h>
#include <limits.h>
#include <cassert>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;

typedef long long ll;

char const g_cellType[6] = {'W','R','L','U','S','E'};

ll timeLimit = 8000;
ll currentTime;

const int W = -1;
const int R = 0;
const int L = 1;
const int U = 2;
const int S = 3;
const int E = 4;

const int DY[4] = {-1, 0, 1, 0};
const int DX[4] = { 0, 1, 0,-1};

const int MAX_WIDTH = 80;
const int MAX_HEIGHT = 80;

// 修正可能なセルの個数
int g_F;
int g_FO;

// 探索者のID
int g_ID;

// 壁ではないセルの数
int g_N;

// 迷路の横幅
int g_width;

// 迷路の縦幅
int g_height;

int g_fixCount;
int g_bestRemainCount;
int g_pathLen;
int g_changeValue;
bool g_success;
map<int, bool> g_bestIdList;

int g_maze[MAX_HEIGHT][MAX_WIDTH];
int g_mazeOrigin[MAX_HEIGHT][MAX_WIDTH];
int g_tempMaze[MAX_HEIGHT][MAX_WIDTH];

int g_bestMaze[MAX_HEIGHT][MAX_WIDTH];
int g_goodMaze[MAX_HEIGHT][MAX_WIDTH];
int g_copyMaze[MAX_HEIGHT][MAX_WIDTH];

// 外周からの距離を求める
int g_outsideDist[MAX_HEIGHT][MAX_WIDTH];

// 探索中に訪れた部分にチェックを付ける
int g_visitedOnePath[MAX_HEIGHT][MAX_WIDTH];

// 探索済みの箇所についてチェックをつける
int g_visitedOverall[MAX_HEIGHT][MAX_WIDTH];

int g_changedOnePath[MAX_HEIGHT][MAX_WIDTH];
int g_changedCheck[MAX_HEIGHT][MAX_WIDTH];

unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

string int2string(int number){
  stringstream ss; 
  ss << number;
  return ss.str();
}

ll getTime(){
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ll result = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
  return result;
}

typedef struct COORD {
  int y;
  int x;
  int dist;

  COORD(int y, int x, int dist = 0){
    this->y = y;
    this->x = x;
    this->dist = dist;
  }
} coord;

typedef struct EXPLORER {
  int id;
  int y;
  int x;
  int pathLen;
  int curDir;

  EXPLORER(int id, int y, int x, int curDir){
    this->id = id;
    this->y = y;
    this->x = x;
    this->curDir = curDir;
    this->pathLen = 0;
  }
} explorer;

typedef struct PATH {
  int pathLen;
  int fixCount;
  vector<coord> pathList;

  PATH(){
    this->pathLen = 0;
    this->fixCount = 0;
  }
} path;

typedef struct NODE{
  int score;
  int maze[MAX_HEIGHT][MAX_WIDTH];

  NODE(){
    this->score = -1;
  }
} node;

node dp[320][320];

vector<string> g_query;
vector<EXPLORER> g_explorerList;

class MazeFixing{
  public:

    void init(vector<string> maze, int F){
      g_F = F;
      g_FO = F;
      g_ID = 0;
      g_N = 0;
      g_height = maze.size();
      g_width = maze[0].size();
      memset(g_maze, W, sizeof(g_maze));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          char type = maze[y][x];

          if(type == 'R'){
            g_maze[y][x] = R;
          }else if(type == 'L'){
            g_maze[y][x] = L;
          }else if(type == 'U'){
            g_maze[y][x] = U;
          }else if(type == 'S'){
            g_maze[y][x] = S;
          }else if(type == 'E'){
            g_maze[y][x] = E;
          }else{
            g_maze[y][x] = W;
					}

          if(type != '.'){
            g_N++;
          }
        }
      }

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] != W){
            int dist = calcOutSideDist(y,x);
            g_outsideDist[y][x] = dist;
          }
        }
      }

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] == W){
            for(int i = 0; i < 4; i++){
              int ny = y + DY[i];
              int nx = x + DX[i];

              if(outside(ny, nx)){
                EXPLORER exp = createExplorer(y,x,i);
                g_explorerList.push_back(exp);
              }
            }
          }
        }
      }

      memcpy(g_mazeOrigin, g_maze, sizeof(g_maze));
      memcpy(g_tempMaze, g_maze, sizeof(g_maze));
    }

    EXPLORER createExplorer(int y, int x, int curDir){
      EXPLORER exp(g_ID, y, x, curDir);
      g_ID++;

      return exp;
    }

    EXPLORER *getExplorer(int id){
      return &g_explorerList[id];
    }

    void changeBest(int y, int x){
      int bestType = 0;

      /*
      for(int i = 0; i < 3; i++){
        int type = list[i];
        g_maze[y][x] = type;
        double score = calcScore();

        if(bestScore < score){
          bestScore = score;
          bestType = type;
        }
      }
      */

      bestType = S;

      g_maze[y][x] = bestType;
    }

    void createQuery(){
      fprintf(stderr,"createQuery =>\n");
			int cnt = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] != g_mazeOrigin[y][x]){
            string query = "";
            query += int2string(y) + " ";
            query += int2string(x) + " ";
            query += g_cellType[g_maze[y][x]+1];
            g_query.push_back(query);
            //fprintf(stderr,"%d: query = %s\n", cnt++, query.c_str());
          }
        }
      }
    }

    void randomChange(){
      int cnt = g_FO;
      int list[4] = {S, S, R, L};

      for(int y = 0; y < g_height && cnt > 0; y++){
        for(int x = 0; x < g_width && cnt > 0; x++){
          int r = xor128()%100;

          if(r < 20 && g_maze[y][x] == U){
            g_maze[y][x] = list[xor128()%4];
            cnt -= 1;
          }
        }
      }
    }

    void randomUchange(int cnt = 10){
      for(int y = 0; y < g_height && cnt > 0; y++){
        for(int x = 0; x < g_width && cnt > 0; x++){
          int r = xor128()%100;

          if(r < 30 && g_maze[y][x] == U){
            g_maze[y][x] = S;
            cnt -= 1;
          }
        }
      }
    }

    void solve(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(inside(y,x) && g_maze[y][x] == U && g_F > 0){
            changeBest(y,x);
            g_F--;
          }
        }
      }
    }

		void resetWalkData(){
			// 現在探索している経路
      memset(g_visitedOnePath, 0, sizeof(g_visitedOnePath));
			// 探索済みの経路
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));
			// 今回の探索で変更したセルを記録する
      memset(g_changedOnePath, 0, sizeof(g_changedOnePath));
			// 今回の探索で変更したセルの確認
      memset(g_changedCheck, 0, sizeof(g_changedCheck));
		}

    void saveMaze(){
      memcpy(g_bestMaze, g_maze, sizeof(g_maze));
    }

    void resetMaze(){
      memcpy(g_maze, g_mazeOrigin, sizeof(g_mazeOrigin));
    }

    void keepMaze(){
      memcpy(g_goodMaze, g_maze, sizeof(g_maze));
    }

    void rollback(){
      memcpy(g_maze, g_goodMaze, sizeof(g_goodMaze));
    }

    void restore(){
      memcpy(g_maze, g_bestMaze, sizeof(g_bestMaze));
    }

    vector<string> improve(vector<string> maze, int F){
      init(maze, F);

      //solve();

      dp[0][0].score = 0;
      memcpy(dp[0][0].maze, g_maze, sizeof(g_maze));

      showMaze();
      int limit = 5;
      int calcCount = 0;

      for(int t = 0; t < limit; t++){
        for(int i = 0; i < g_ID; i++){
          if(dp[t][i].score >= 0){
            for(int j = 0; j < g_ID; j++){
              if(i == j) continue;
              g_fixCount = 0;
              g_pathLen = 0;
              memcpy(g_maze, dp[t][i].maze, sizeof(g_maze));
              EXPLORER *e = getExplorer(j);

			        resetWalkData();
              realWalk(e->y, e->x, e->curDir, e->curDir, true);

              double score = calcScore();
              calcCount += 1;

              if(g_fixCount <= g_FO && dp[t+1][j].score < score){
                dp[t+1][j].score = score;
                memcpy(dp[t+1][j].maze, g_maze, sizeof(g_maze));
              }
            }
          }
        }
      }

      double bestScore = 0.0;

      for(int i = 0; i < g_ID; i++){
        double score = dp[limit][i].score;

        if(bestScore < score){
          bestScore = score;
          memcpy(g_maze, dp[limit][i].maze, sizeof(g_maze));
        }
      }

      fprintf(stderr,"calcCount = %d\n", calcCount);
      fprintf(stderr,"final remain f count = %d\n", g_F);
      createQuery();
      fprintf(stderr,"query size = %lu\n", g_query.size());

      return g_query;
    }

		// 経路として確定している部分は変更が出来ない
		bool canChangeCell(int y, int x){
			return !g_visitedOverall[y][x];
		}

    bool inside(int y, int x){
      return g_maze[y][x] != W;
    }

    bool outside(int y, int x){
      if(y < 0 || x < 0 || g_height <= y || g_width <= x) return false;
      if(g_maze[y][x] == W) return false;

      for(int i = 0; i < 4; i++){
        int ny = y + DY[i];
        int nx = x + DX[i];

        if(g_maze[ny][nx] == W){
          return true;
        }
      }

      return false;
    }

    double calcWalkValue(int id, int y, int x, int curDir, int origDir){
      g_fixCount = 0;
      g_pathLen = 0;
			g_changeValue = 0;
      g_success = false;

			resetWalkData();

      walk(y, x, curDir, origDir);

      //fprintf(stderr,"id = %d, fixCount = %d, pathLen = %d\n", id, g_fixCount, g_pathLen);

			if(g_fixCount == 0 || g_fixCount > g_F){
        return -1.0;
			}else if(!g_success){
        return g_fixCount;
      }else{
				if(g_FO < 1000){
        	return (g_pathLen) / (double)(g_fixCount);
				}else{
        	return g_pathLen;
				}
      }
    }

    void walk(int y, int x, int curDir, int origDir){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];

			// 既に探索されていた場合は探索を抜ける
      if(g_visitedOnePath[ny][nx]){
        return;
      }

      int type = g_maze[ny][nx];

			if(g_changedCheck[ny][nx]){
				type = g_tempMaze[ny][nx];
			}

      g_visitedOnePath[ny][nx] = 1;

      //fprintf(stderr,"y = %d, x = %d, type = %c\n", ny, nx, g_cellType[type+1]);

      if(type == W){
				// 経路作成に成功
        g_success = true;

        for(int dy = 0; dy < g_height; dy++){
          for(int dx = 0; dx < g_width; dx++){
						// 今回生成した経路で変更が確定していない部分があれば追加する
          	if(g_visitedOnePath[dy][dx] && g_changedOnePath[dy][dx] && !g_changedCheck[dy][dx]){
							if(g_maze[dy][dx] != U){
								g_fixCount += 1;
							}else{
								g_fixCount += 1;
							}
							g_changedCheck[dy][dx] = 1;
            }
						// 今回生成した経路でまだ未チェックの部分がある場合は経路長を伸ばす
						if(g_visitedOnePath[dy][dx] && !g_visitedOverall[dy][dx]){
							g_pathLen += 1;
							g_visitedOverall[dy][dx] = 1;	
						}
          }
        }
      }else if(type == S){
        walk(ny, nx, curDir, origDir);
      }else if(type == R){
				// 方向を変化させて結果元の方角と逆を向いてしまった場合
        if((curDir+1)%4 == (origDir+2)%4){
          //fprintf(stderr,"y = %d, x = %d, change R -> L\n", ny, nx)
					
					// セルが変更可能であれば修正する
          if(canChangeCell(ny,nx)){
            g_changedOnePath[ny][nx] = 1;
						g_tempMaze[ny][nx] = L;
            walk(ny, nx, (curDir+3)%4, origDir);
          }
        }else{
          walk(ny, nx, (curDir+1)%4, origDir);
        }
      }else if(type == U){
        //fprintf(stderr,"y = %d, x = %d, change U -> S\n", ny, nx);
				
        if(canChangeCell(ny,nx)){
          g_changedOnePath[ny][nx] = 1;
					g_tempMaze[ny][nx] = S;
          walk(ny, nx, curDir, origDir);
        }
      }else if(type == L){
        if((curDir+3)%4 == (origDir+2)%4){
          //fprintf(stderr,"y = %d, x = %d, change L -> R\n", ny, nx);
					
          if(canChangeCell(ny,nx)){
            g_changedOnePath[ny][nx] = 1;
						g_tempMaze[ny][nx] = R;
            walk(ny, nx, (curDir+1)%4, origDir);
          }
        }else{
          walk(ny, nx, (curDir+3)%4, origDir);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          if(i != 2){
            walk(ny, nx, (curDir+i)%4, (curDir+i)%4);
          }
        }
      }

      g_visitedOnePath[ny][nx] = 0;
    }

    void realWalk(int y, int x, int curDir, int origDir, bool first = false){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];

      if(g_visitedOnePath[ny][nx]){
        return;
      }

      int type = g_maze[ny][nx];

      g_visitedOnePath[ny][nx] = 1;

      //fprintf(stderr,"y = %d, x = %d, type = %c\n", ny, nx, g_cellType[type+1]);

      if(type == W){
        for(int dy = 0; dy < g_height; dy++){
          for(int dx = 0; dx < g_width; dx++){

          	if(g_visitedOnePath[dy][dx] && g_changedOnePath[dy][dx] && !g_changedCheck[dy][dx]){
            	//fprintf(stderr,"y = %d, x = %d, change %c -> %c\n", dy, dx, g_cellType[g_maze[dy][dx]+1], g_cellType[g_tempMaze[dy][dx]+1]);
								
							assert(g_maze[dy][dx] != g_tempMaze[dy][dx]);

							// 変更を保存していたものを実際の盤面に反映させる
              g_maze[dy][dx] = g_tempMaze[dy][dx];
							g_changedCheck[dy][dx] = 1;
            }

						// 探索経路上のセルは変更出来ない
						if(g_visitedOnePath[dy][dx] && !g_visitedOverall[dy][dx]){
              g_pathLen += 1;
              g_visitedOverall[dy][dx] = 1;
            }
          }
        }
      }else if(type == S){
        realWalk(ny, nx, curDir, origDir);
      }else if(type == R){
        if((curDir+1)%4 == (origDir+2)%4){
          
          if(canChangeCell(ny,nx)){
          	//fprintf(stderr,"y = %d, x = %d, change R -> L\n", ny, nx);
            g_changedOnePath[ny][nx] = 1;
            g_tempMaze[ny][nx] = L;
            realWalk(ny, nx, (curDir+3)%4, origDir);
          }
        }else{
          realWalk(ny, nx, (curDir+1)%4, origDir);
        }
      }else if(type == U){
        
        	if(canChangeCell(ny,nx)){
        		//fprintf(stderr,"y = %d, x = %d, change U -> S\n", ny, nx);
         		g_changedOnePath[ny][nx] = 1;
          	g_tempMaze[ny][nx] = S;
          	realWalk(ny, nx, curDir, origDir);
        	}
      }else if(type == L){
        if((curDir+3)%4 == (origDir+2)%4){
          
         	if(canChangeCell(ny,nx)){
          	//fprintf(stderr,"y = %d, x = %d, change L -> R\n", ny, nx);
            g_changedOnePath[ny][nx] = 1;
            g_tempMaze[ny][nx] = R;
            realWalk(ny, nx, (curDir+1)%4, origDir);
          }
        }else{
          realWalk(ny, nx, (curDir+3)%4, origDir);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          if(i != 2){
            realWalk(ny, nx, (curDir+i)%4, (curDir+i)%4);
          }
        }
      }

      g_visitedOnePath[ny][nx] = 0;
    }

    int calcOutSideDist(int y, int x){
      queue<COORD> que;
      que.push(coord(y,x,0));
      map<int, bool> checkList;

      while(!que.empty()){
        COORD coord = que.front(); que.pop();

        int z = coord.y * g_width + coord.x;
        if(checkList[z]) continue;
        checkList[z] = true;

        if(outside(coord.y, coord.x)){
          return coord.dist;
        }

        for(int i = 0; i < 4; i++){
          int ny = coord.y + DY[i];
          int nx = coord.x + DX[i];
          int nz = ny * g_width + nx;

          if(!checkList[nz]){
            que.push(COORD(ny,nx,coord.dist+1));
          }
        }
      }

      return 0;
    }

    double calcScore(){
      memset(g_visitedOnePath, 0, sizeof(g_visitedOnePath));
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int type = g_maze[y][x];

          if(type == W && !g_visitedOnePath[y][x]){
            for(int i = 0; i < 4; i++){
              int ny = y + DY[i];
              int nx = x + DX[i];

              if(inside(ny, nx)){
                search(y, x, i);
              }
            }
          }
        }
      }

      int nvis = 0;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(inside(y,x) && g_visitedOverall[y][x]){
            nvis++;
          }
          if(g_maze[y][x] != g_mazeOrigin[y][x]){
            g_fixCount += 1;
          }
        }
      }


      //fprintf(stderr,"%d/%d\n", nvis, g_N);
      return nvis;
    }

    void search(int curY, int curX, int curDir){
      int ny = curY + DY[curDir];
      int nx = curX + DX[curDir];

      if(g_visitedOnePath[ny][nx]){
        return;
      }

      g_visitedOnePath[ny][nx] = 1;

      int type = g_maze[ny][nx];

      if(type == W){
        for(int y = 0; y < g_height; y++){
          for(int x = 0; x < g_width; x++){
            g_visitedOverall[y][x] = g_visitedOverall[y][x] | g_visitedOnePath[y][x];
          }
        }
      }else if(type == S){
        search(ny, nx, curDir);
      }else if(type == R){
        search(ny, nx, (curDir+1)%4);
      }else if(type == U){
        search(ny, nx, (curDir+2)%4);
      }else if(type == L){
        search(ny, nx, (curDir+3)%4);
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          search(ny, nx, i);
        }
      }

      g_visitedOnePath[ny][nx] = 0;
    }

    void showMaze(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int type = g_maze[y][x];
          fprintf(stderr,"%c",g_cellType[type+1]);
        }
        fprintf(stderr,"\n");
      }
    }
};

int main(){
  int h,f;
  string line;
  cin >> h;
  vector<string> maze;
  for(int i = 0; i < h; i++){
    cin >> line;
    maze.push_back(line);
  }
  cin >> f;
  MazeFixing mf;
  timeLimit = 1000;
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
