#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
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

const int W = -1;
const int R = 0;
const int L = 1;
const int U = 2;
const int S = 3;
const int E = 4;

const int g_direct[4] = {S, R, U, L};

const int DY[4] = {-1, 0, 1, 0};
const int DX[4] = { 0, 1, 0,-1};

const int MAX_WIDTH = 80;
const int MAX_HEIGHT = 80;
int g_F;
int g_ID;
int g_N;
int g_width;
int g_height;

int g_fixCount;

int g_maze[MAX_WIDTH][MAX_HEIGHT];
int g_mazeOrigin[MAX_WIDTH][MAX_HEIGHT];
int g_outsideDist[MAX_WIDTH][MAX_HEIGHT];
int g_visitedOnePath[MAX_HEIGHT][MAX_WIDTH];
int g_visitedOverall[MAX_HEIGHT][MAX_WIDTH];
int g_visitedOnce[MAX_HEIGHT][MAX_WIDTH];

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
    }

    EXPLORER createExplorer(int y, int x, int curDir){
      EXPLORER exp(g_ID, y, x, curDir);
      g_ID++;

      return exp;
    }

    EXPLORER *getExplorer(int id){
      return &g_explorerList[id];
    }

    void changeRandom(int y, int x){
      int list[3] = {S,R,L};

      int type = list[xor128()%3];
      createQuery(y,x,type);
      g_maze[y][x] = type;
    }

    void changeBest(int y, int x){
      int list[3] = {S,R,L};
      double bestScore = 0.0;
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
      createQuery(y,x,bestType);
    }

    void createQuery(int y, int x, int type){
      string query = "";
      query += int2string(y) + " ";
      query += int2string(x) + " ";
      query += g_cellType[type+1];
      g_query.push_back(query);
    }

    void solve(){
      int f = g_F;

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] != W && g_maze[y][x] != E && g_maze[y][x] != S && outside(y,x) && f > 0){
            g_maze[y][x] = S;
            createQuery(y,x,S);
            f--;
          }
        }
      }

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(inside(y,x) && g_maze[y][x] == U && f > 0){
            //changeRandom(y,x);
            changeBest(y,x);
            f--;
          }
        }
      }

      calcScore();

      int dist = 2;
      while(f > 0){
        for(int y = 0; y < g_height && f > 0; y++){
          for(int x = 0; x < g_width && f > 0; x++){
            if(g_visitedOnce[y][x] && g_maze[y][x] != W && g_maze[y][x] != E && g_maze[y][x] != S && g_outsideDist[y][x] == dist){
              g_maze[y][x] = S;
              createQuery(y,x,S);
              f--;
              calcScore();
            }
          }
        }

        dist += 2;
        if(dist > g_height/2) break;
      }

      fprintf(stderr,"remain f count = %d\n", f);
    }

    vector<string> improve(vector<string> maze, int F){
      init(maze, F);

      solve();
      //showMaze();

      for(int id = 0; id < g_ID; id++){
        EXPLORER *exp = getExplorer(id);

        fprintf(stderr,"y = %d, x = %d, d = %d\n", exp->y, exp->x, exp->curDir);
      }

      fprintf(stderr,"Current = %f\n", calcScore());

      return g_query;
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

    void calcWalkValue(int y, int x, int curDir, int origDir){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];

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
        calcWalkValue(ny, nx, curDir, origDir);
      }else if(type == R){
        if((curDir+1)%4 == origDir){
          g_fixCount += 1;
          calcWalkValue(ny, nx, (curDir+3)%4, origDir);
        }else{
          calcWalkValue(ny, nx, (curDir+1)%4, origDir);
        }
      }else if(type == U){
        g_fixCount += 1;
        calcWalkValue(ny, nx, curDir, origDir);
      }else if(type == L){
        if((curDir+3)%4 == origDir){
          g_fixCount += 1;
          calcWalkValue(ny, nx, (curDir+1)%4, origDir);
        }else{
          calcWalkValue(ny, nx, (curDir+3)%4, origDir);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          if(i != 2){
            calcWalkValue(ny, nx, (curDir+i)%4, (curDir+i)%4);
          }
        }
      }

      g_visitedOnePath[ny][nx] = 0;
    }

    void walk(int y, int x, int curDir, int origDir, int fixCount){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];

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
        walk(ny, nx, curDir, origDir, fixCount);
      }else if(type == R){
        if((curDir+1)%4 == origDir){
          g_maze[ny][nx] = L;
          walk(ny, nx, (curDir+3)%4, origDir, fixCount+1);
        }else{
          walk(ny, nx, (curDir+1)%4, origDir, fixCount);
        }
      }else if(type == U){
        g_maze[ny][nx] = S;
        walk(ny, nx, curDir, origDir, fixCount+1);
      }else if(type == L){
        if((curDir+3)%4 == origDir){
          g_maze[ny][nx] = R;
          walk(ny, nx, (curDir+1)%4, origDir, fixCount+1);
        }else{
          walk(ny, nx, (curDir+3)%4, origDir, fixCount);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          int ny = y + DY[i];
          int nx = x + DX[i];
          walk(ny, nx, i, origDir, fixCount);
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
      memset(g_visitedOnce, 0, sizeof(g_visitedOnce));

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

      //fprintf(stderr,"nvis = %d\n", nvis);

      return nvis / (double)g_N;
    }

    void search(int curY, int curX, int curDir){
      int ny = curY + DY[curDir];
      int nx = curX + DX[curDir];

      if(g_visitedOnePath[ny][nx]){
        return;
      }
      g_visitedOnePath[ny][nx] = 1;
      g_visitedOnce[ny][nx] = 1;

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
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
