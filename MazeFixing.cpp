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

char const g_cellType[6] = {'W','R','U','L','S','E'};

ll timeLimit = 8000;
ll currentTime;

bool g_debug;

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

// 探索者のID
int g_ID;

// 壁ではないセルの数
int g_N;

// 変更可能なセルの数
int g_cellCount;

// 迷路の横幅
int g_width;

// 迷路の縦幅
int g_height;

int g_RCount;
int g_LCount;
int g_UCount;
int g_ECount;
int g_SCount;

int g_fixCount;
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

// 何回訪れられているかをチェック
int g_visitedCount[MAX_HEIGHT][MAX_WIDTH];

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

typedef struct SEARCHS{
  int score;
  int alist[200];

  SEARCHS(){
    this->score = 0;
    memset(alist, -1, sizeof(alist));
  }
} searchs;

SEARCHS dp[320][320];

vector<string> g_query;
vector<EXPLORER> g_explorerList;

class MazeFixing{
  public:

    void init(vector<string> maze, int F){
      g_F = F;
      g_FO = F;
      g_ID = 0;
      g_N = 0;
      g_cellCount = 0;
      g_RCount = 0;
      g_LCount = 0;
      g_ECount = 0;
      g_SCount = 0;
      g_UCount = 0;
      g_height = maze.size();
      g_width = maze[0].size();
      memset(g_maze, W, sizeof(g_maze));

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
            g_UCount += 1;
          }else if(type == 'S'){
            g_maze[y][x] = S;
            g_SCount += 1;
          }else if(type == 'E'){
            g_maze[y][x] = E;
            g_ECount += 1;
          }else{
            g_maze[y][x] = W;
          }

          if(type != '.'){
            g_N++;

            if(type != 'E'){
              g_cellCount += 1;
            }
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

      if(g_debug){
        fprintf(stderr,"R: %d, L: %d, S: %d, U: %d, E: %d\n", g_RCount, g_LCount, g_SCount, g_UCount, g_ECount);
        fprintf(stderr,"change count = %d\n", g_N - (g_SCount + g_ECount));
        fprintf(stderr,"can change cell count = %d\n", g_cellCount);
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
            query += g_cellType[g_maze[y][x]];
            g_query.push_back(query);
            //fprintf(stderr,"%d: query = %s\n", cnt++, query.c_str());
          }
        }
      }
    }

    void resetMazeData(){
      // 現在探索している経路
      memset(g_visitedOnePath, 0, sizeof(g_visitedOnePath));
      // 探索済みの経路
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));
      // セルの訪問回数を保持
      memset(g_visitedCount, 0, sizeof(g_visitedCount));
    }

    void saveMaze(){
      memcpy(g_bestMaze, g_maze, sizeof(g_maze));
    }

    void resetMaze(){
      memcpy(g_maze, g_mazeOrigin, sizeof(g_mazeOrigin));
      memcpy(g_goodMaze, g_mazeOrigin, sizeof(g_mazeOrigin));
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

    void fillMaze(int type){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          if(g_maze[y][x] != W && g_maze[y][x] != E){
            g_maze[y][x] = type;
          }
        }
      }
    }

    vector<string> improve(vector<string> maze, int F){
      init(maze, F);

      int cnt = 0;
      int tryCount = 0;
      int notChangeCount = 0;

      const ll startTime = getTime();
      const ll endTime = startTime + timeLimit;

      double goodScore = -10000.0;
      double bestScore = -10000.0;

      double T = 100.0;
      double k = 0.5;
      double alpha = 0.999;
      currentTime = startTime;

      saveMaze();
      keepMaze();
      fillMaze(S);
      //showMaze();

      while(currentTime < endTime){
        tryCount += 1;

        int id = xor128()%g_ID;
        EXPLORER *e = getExplorer(id);
        resetMazeData();

        g_fixCount = 0;
        double score = calcScore();

        if(g_fixCount > g_FO){
          score -= 100000;
        }

        if(bestScore < score){
          bestScore = score;
          saveMaze();
        }

        double rate = exp(-(goodScore-score)/(k*T));
        //fprintf(stderr,"goodScore = %f, score = %f, rate = %f\n", goodScore, score, rate);

        if(goodScore < score){
          goodScore = score;
          keepMaze();
        }else if(T > 0 && (xor128() % 100) < 100.0 * rate){
          goodScore = score;
          keepMaze();
          notChangeCount = 0;
        }else{
          notChangeCount += 1;

          if(notChangeCount > 100){
            notChangeCount = 0;
            goodScore = 0.0;
            resetMaze();
            //fprintf(stderr,"hello\n");
          }
        }

        rollback();
        currentTime = getTime();
        T *= alpha;
      }

      //restore();

      fprintf(stderr,"Current = %f\n", calcScore());
      fprintf(stderr,"tryCount = %d\n", tryCount);
      fprintf(stderr,"final remain f count = %d\n", g_F);
      createQuery();
      fprintf(stderr,"query size = %lu\n", g_query.size());

      return g_query;
    }

    // 経路として確定している部分は変更が出来ない
    bool canChangeCell(int y, int x){
      return !g_visitedOverall[y][x];
    }

    // セルを変更する
    void changeCell(int y, int x, int type){
      g_maze[y][x] = type;
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
        search(ny, nx, turnRight(curDir));
      }else if(type == U){
        search(ny, nx, (curDir+2)%4);
      }else if(type == L){
        search(ny, nx, turnLeft(curDir));
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          search(ny, nx, i);
        }
      }

      g_visitedOnePath[ny][nx] = 0;
    }

    int turnRight(int curDir){
      return (curDir+1)%4;
    }

    int turnLeft(int curDir){
      return (curDir+3)%4;
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
  g_debug = true;
  timeLimit = 1000;
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){cout << ret[i] << endl;}
  return 0;
}
