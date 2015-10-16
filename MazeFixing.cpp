#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
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

char const g_cellType[6] = {'W','R','U','L','S','E'};

const int W = 0;
const int R = 1;
const int U = 2;
const int L = 3;
const int S = 4;
const int E = 5;

const int DY[4] = {-1, 0, 1, 0};
const int DX[4] = { 0, 1, 0,-1};

const int MAX_WIDTH = 80;
const int MAX_HEIGHT = 80;

// 修正可能なセルの個数
int g_F;
int g_FO;

int g_RCount;
int g_LCount;

// 探索者のID
int g_ID;

// 壁ではないセルの数
int g_N;

// 迷路の横幅
int g_width;

// 迷路の縦幅
int g_height;

int g_fixCount;
int g_pathLen;
int g_changeValue;
bool g_success;
map<int, bool> g_bestIdList;

int g_maze[MAX_HEIGHT][MAX_WIDTH];
int g_mazeOrigin[MAX_HEIGHT][MAX_WIDTH];
int g_tempMaze[MAX_HEIGHT][MAX_WIDTH];

// 外周からの距離を求める
int g_outsideDist[MAX_HEIGHT][MAX_WIDTH];

// 探索中に訪れた部分にチェックを付ける
int g_visitedOnePath[MAX_HEIGHT][MAX_WIDTH];

// 探索済みの箇所についてチェックをつける
int g_visitedOverall[MAX_HEIGHT][MAX_WIDTH];

int g_visitedCount[MAX_HEIGHT][MAX_WIDTH];

int g_changedOnePath[MAX_HEIGHT][MAX_WIDTH];
int g_changedCheck[MAX_HEIGHT][MAX_WIDTH];
int g_changePoint[MAX_HEIGHT][MAX_WIDTH];
int g_notChangedPath[MAX_HEIGHT][MAX_WIDTH];

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

vector<string> g_query;
vector<EXPLORER> g_explorerList;

class MazeFixing{
  public:

    void init(vector<string> maze, int F){
      g_F = F;
      g_FO = F;
      g_ID = 0;
			g_LCount = 0;
			g_RCount = 0;
      g_N = 0;
      g_height = maze.size();
      g_width = maze[0].size();
      memset(g_maze, W, sizeof(g_maze));
      memset(g_notChangedPath, 0, sizeof(g_notChangedPath));
      memset(g_changePoint, 0, sizeof(g_changePoint));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          char type = maze[y][x];

          if(type == 'R'){
            g_maze[y][x] = R;
						g_RCount += 1;
          }else if(type == 'L'){
            g_maze[y][x] = L;
						g_LCount += 1;
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
          if(g_changePoint[y][x]){
          //if(g_changePoint[y][x]){
            string query = "";
            query += int2string(y) + " ";
            query += int2string(x) + " ";
            query += g_cellType[g_maze[y][x]];
            g_query.push_back(query);
            //fprintf(stderr,"%d: query = %s\n", cnt++, query.c_str());
          }
        }
      }
    }

    void solve(){
			/*
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] != W && g_maze[y][x] != E && g_maze[y][x] != S && outside(y,x) && g_F > 0){
            g_maze[y][x] = S;
            g_F--;
          }
        }
      }
			*/

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(inside(y,x) && g_maze[y][x] == U && g_F > 0){
            changeBest(y,x);
            g_F--;
          }
        }
      }
			/*

      calcScore();

      int dist = 2;
      while(f > 0){
        for(int y = 0; y < g_height && f > 0; y++){
          for(int x = 0; x < g_width && f > 0; x++){
            if(g_maze[y][x] != W && g_maze[y][x] != E && g_maze[y][x] != S && g_outsideDist[y][x] == dist){
              g_maze[y][x] = S;
            	g_F--;
              calcScore();
            }
          }
        }

        dist += 2;
        if(dist > g_height/2) break;
      }
			*/
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

    vector<string> improve(vector<string> maze, int F){
      init(maze, F);

      //solve();
      //showMaze();
      int cnt = 0;

      while(g_F > 0){
        double maxValue = 0.0;
        bool update = false;
        int bestId = -1;

				calcScore();

        for(int id = 0; id < g_ID; id++){
					if(g_bestIdList[id]) continue;

          EXPLORER *exp = getExplorer(id);

          double value = calcWalkValue(exp->id, exp->y, exp->x, exp->curDir, exp->curDir);
          //fprintf(stderr,"y = %d, x = %d, d = %d, value = %4.2f\n", exp->y, exp->x, exp->curDir, value);

          if(maxValue < value && !g_bestIdList[id]){
            maxValue = value;
            bestId = id;
            update = true;
          }
        }

        if(!update) break;
        cnt++;

        fprintf(stderr,"bestId = %d\n", bestId);

        EXPLORER *exp = getExplorer(bestId);

				resetWalkData();
        walk(exp->y, exp->x, exp->curDir, exp->curDir, true);

        fprintf(stderr,"remain f count = %d\n", g_F);
        g_bestIdList[bestId] = true;
      }

      fprintf(stderr,"Current = %f\n", calcScore());
      fprintf(stderr,"final remain f count = %d\n", g_F);
      createQuery();
      fprintf(stderr,"query size = %lu\n", g_query.size());

      return g_query;
    }

		// 経路として確定している部分は変更が出来ない
		bool canChangedCell(int y, int x){
			return (!g_visitedOverall[y][x] && !g_notChangedPath[y][x]);
		}

    bool inside(int y, int x){
      return (0 <= y && 0 <= x && y < g_height && x < g_width && g_maze[y][x] != W);
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
        	return (g_pathLen) / (double)(g_fixCount);
        	return g_pathLen;
				}
      }
    }

    void walk(int y, int x, int curDir, int origDir, bool real = false){
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

      //fprintf(stderr,"y = %d, x = %d, type = %c\n", ny, nx, g_cellType[type]);

      if(type == W){
				// 経路作成に成功
        g_success = true;

        for(int dy = 0; dy < g_height; dy++){
          for(int dx = 0; dx < g_width; dx++){
						int odist = g_outsideDist[dy][dx];
						int vCnt = g_visitedCount[dy][dx];
						// 今回生成した経路で変更が確定していない部分があれば追加する
          	if(g_visitedOnePath[dy][dx] && g_changedOnePath[dy][dx] && !g_changedCheck[dy][dx]){
							if(g_maze[dy][dx] != U){
								g_fixCount += 1;
							}else{
								g_fixCount += 1;
							}
							g_changedCheck[dy][dx] = 1;

							if(real){
              	g_maze[dy][dx] = g_tempMaze[dy][dx];
								g_changePoint[dy][dx] = 1;
              	g_F -= 1;
							}
            }
						// 今回生成した経路でまだ未チェックの部分がある場合は経路長を伸ばす
						if(g_visitedOnePath[dy][dx] && !g_visitedOverall[dy][dx]){
							if(vCnt < 1){
								g_pathLen += 1;
							}else{
								g_pathLen += 0;
							}
							g_visitedOverall[dy][dx] = 1;	
						}
						// 探索経路上のセルは変更出来ない
						if(real && g_visitedOnePath[dy][dx] && !g_notChangedPath[dy][dx]){
           		g_notChangedPath[dy][dx] = 1;
            }
          }
        }
      }else if(type == S){
        walk(ny, nx, curDir, origDir, real);
      }else if(type == R){
				// 方向を変化させて結果元の方角と逆を向いてしまった場合
        if((curDir+1)%4 == (origDir+2)%4){
          //fprintf(stderr,"y = %d, x = %d, change R -> L\n", ny, nx)
					
					// セルが変更可能であれば修正する
          if(canChangedCell(ny,nx)){
            g_changedOnePath[ny][nx] = 1;
						g_tempMaze[ny][nx] = L;
            walk(ny, nx, (curDir+3)%4, origDir, real);
          }
        }else{
          walk(ny, nx, (curDir+1)%4, origDir, real);
        }
      }else if(type == U){
        //fprintf(stderr,"y = %d, x = %d, change U -> S\n", ny, nx);
				
        if(canChangedCell(ny,nx)){
          g_changedOnePath[ny][nx] = 1;
					g_tempMaze[ny][nx] = S;
          walk(ny, nx, curDir, origDir, real);
        }
      }else if(type == L){
        if((curDir+3)%4 == (origDir+2)%4){
          //fprintf(stderr,"y = %d, x = %d, change L -> R\n", ny, nx);
					
          if(canChangedCell(ny,nx)){
            g_changedOnePath[ny][nx] = 1;
						g_tempMaze[ny][nx] = R;
            walk(ny, nx, (curDir+1)%4, origDir, real);
          }
        }else{
          walk(ny, nx, (curDir+3)%4, origDir, real);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          if(i != 2){
            walk(ny, nx, (curDir+i)%4, (curDir+i)%4, real);
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
			memset(g_visitedCount, 0, sizeof(g_visitedCount));

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
        }
      }


      fprintf(stderr,"%d/%d\n", nvis, g_N);
      return nvis / (double)g_N;
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
						if(g_visitedOnePath[y][x]){
							g_visitedCount[y][x] += 1;
						}
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
          fprintf(stderr,"%c",g_cellType[type]);
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
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
