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

int g_nextDirect[4][5] = {
  {-1, 1, 2, 3, 0},
  {-1, 2, 3, 0, 1},
  {-1, 3, 0, 1, 2},
  {-1, 0, 1, 2, 3}
};

ll timeLimit = 8000;
ll middleLimit = 5000;

ll getTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ll result =  tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
  return result;
}

const int W = 0;
const int R = 1;
const int U = 2;
const int L = 3;
const int S = 4;
const int E = 5;

bool g_debug = false;
bool g_real;

const int DY[4] = {-1, 0, 1, 0};
const int DX[4] = { 0, 1, 0,-1};

const int MAX_WIDTH = 80;
const int MAX_HEIGHT = 80;

// 修正可能なセルの個数
int g_F;
int g_FO;

int g_RCount;
int g_LCount;
int g_SCount;
int g_UCount;

// 探索者のID
int g_ID;

// 壁ではないセルの数
int g_N;

int g_beforeNvis;
int g_currentNvis;

// 迷路の横幅
int g_width;

// 迷路の縦幅
int g_height;

int g_fixCount;
int g_pathLen;
int g_changeValue;
int g_turn;
bool g_success;
map<int, bool> g_bestIdList;

int g_maze[MAX_HEIGHT][MAX_WIDTH];
int g_mazeOrigin[MAX_HEIGHT][MAX_WIDTH];
int g_tempMaze[MAX_HEIGHT][MAX_WIDTH];

int g_bestMaze[MAX_HEIGHT][MAX_WIDTH];
int g_goodMaze[MAX_HEIGHT][MAX_WIDTH];
int g_copyMaze[MAX_HEIGHT][MAX_WIDTH];

// 探索中に訪れた部分にチェックを付ける
int g_visitedOnePath[MAX_HEIGHT][MAX_WIDTH];

// 探索済みの箇所についてチェックをつける
int g_visitedOverall[MAX_HEIGHT][MAX_WIDTH];

int g_visitedCount[MAX_HEIGHT][MAX_WIDTH];

int g_changedOnePath[MAX_HEIGHT][MAX_WIDTH];
int g_changedCheck[MAX_HEIGHT][MAX_WIDTH];
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

typedef struct EVAL{
  int pathLen;
  int vPathLen;
  int unreachedCellCount;
  int fixCount;

  EVAL(){
    this->pathLen = 0;
    this->fixCount = 0;
    this->vPathLen = 0;
    this->unreachedCellCount = 0;
  }

  double score(){
    return pathLen;
  }
} eval;

typedef struct CellData{
  int tCnt[6];

} cellData;

vector<string> g_query;
vector<EXPLORER> g_explorerList;
vector<coord> g_cellCoordList;

int g_cellCount;

class MazeFixing{
  public:

    void init(vector<string> maze, int F){
      g_F = F;
      g_FO = F;
      g_ID = 0;
      g_LCount = 0;
      g_RCount = 0;
      g_SCount = 0;
      g_UCount = 0;
      g_turn = 0;
      g_N = 0;
      g_height = maze.size();
      g_width = maze[0].size();
      memset(g_maze, W, sizeof(g_maze));
      memset(g_notChangedPath, 0, sizeof(g_notChangedPath));
      memset(g_visitedOnePath, -1, sizeof(g_visitedOnePath));
      // 今回の探索で変更したセルの確認
      memset(g_changedCheck, -1, sizeof(g_changedCheck));
      // 今回の探索で変更したセルを記録する
      memset(g_changedOnePath, -1, sizeof(g_changedOnePath));

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
            g_notChangedPath[y][x] = 1;
          }else{
            g_maze[y][x] = W;
          }

          if(type != '.'){
            g_N++;

            if(type != 'S' && type != 'E'){
              coord c(y,x);
              g_cellCoordList.push_back(c);
              g_cellCount += 1;
            }
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

      fprintf(stderr,"Explorer Count = %d\n", g_ID);

      memcpy(g_mazeOrigin, g_maze, sizeof(g_maze));
      memcpy(g_tempMaze, g_maze, sizeof(g_maze));
    }

    EXPLORER createExplorer(int y, int x, int curDir){
      EXPLORER exp(g_ID, y, x, curDir);
      g_ID++;

      return exp;
    }

    inline EXPLORER *getExplorer(int id){
      return &g_explorerList[id];
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
          }
        }
      }
    }

    inline void resetMazeData(){
      // 探索済みの経路
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));
    }

    inline void saveMaze(){
      memcpy(g_bestMaze, g_maze, sizeof(g_maze));
    }

    inline void keepMaze(){
      memcpy(g_goodMaze, g_maze, sizeof(g_maze));
    }

    inline void rollback(){
      memcpy(g_maze, g_goodMaze, sizeof(g_goodMaze));
    }

    inline void restore(){
      memcpy(g_maze, g_bestMaze, sizeof(g_bestMaze));
    }

    vector<string> improve(vector<string> maze, int F){
      init(maze, F);

      //showMaze();

      fprintf(stderr,"R count = %d\n", g_RCount);
      fprintf(stderr,"L count = %d\n", g_LCount);
      fprintf(stderr,"U count = %d\n", g_UCount);
      fprintf(stderr,"S count = %d\n", g_SCount);

      const ll startTime = getTime();
      ll endTime = startTime + timeLimit;
      ll currentTime = getTime();

      while(g_F > 0 && currentTime < endTime){
        double maxValue = 0.0;
        bool update = false;
        int bestId = -1;

        beforeProc();

        for(int id = 0; id < g_ID; id++){
          if(g_bestIdList[id]) continue;

          EXPLORER *exp = getExplorer(id);

          double value = 0.0;

          value = calcWalkValue(exp->id, exp->y, exp->x, exp->curDir, exp->curDir);

          //fprintf(stderr,"y = %d, x = %d, d = %d, value = %4.2f\n", exp->y, exp->x, exp->curDir, value);

          if(maxValue < value && !g_bestIdList[id]){
            maxValue = value;
            bestId = id;
            update = true;
          }
        }

        if(!update) break;

        EXPLORER *exp = getExplorer(bestId);

        resetMazeData();

        g_real = true;
        ++g_turn;
        walk(exp->y, exp->x, exp->curDir, exp->curDir);

        if(g_debug){
          fprintf(stderr,"bestId = %d\n", bestId);
          fprintf(stderr,"remain f count = %d\n", g_F);
        }
        g_bestIdList[bestId] = true;
        currentTime = getTime();
      }

      // ---------------------- sa -----------------------
      
      double T = 1000.0;
      double k = 0.1;
      double alpha = 0.999;

      ll middleTime = startTime + middleLimit;
      currentTime = getTime();
      int tryCount = 0;

      resetMazeData();
      eval e = calcScore();
      double bestScore = e.score();
      double goodScore = e.score();

      saveMaze();
      keepMaze();

      int span = (currentTime < middleTime)? 100 : 10;
      fprintf(stderr,"span = %d, currentTime = %lld\n", span, currentTime-startTime);

      if(!g_debug && currentTime < middleTime){
        timeLimit = 9000;
        endTime = startTime + timeLimit;
      }

      while(currentTime < endTime){
        tryCount += 1;

        coord c = g_cellCoordList[xor128()%g_cellCount];
        coord c2 = g_cellCoordList[xor128()%g_cellCount];

        changeCell(c.y, c.x);
        changeCell(c2.y, c2.x);

        resetMazeData();
        eval e = calcScore();
        double score = e.score();

        //double rate = exp(-(goodScore-score) / (k*T));

        if(bestScore < score && e.fixCount <= g_FO){
          bestScore = score;
          saveMaze();
        }else{
          restoreCell(c.y, c.x);
          restoreCell(c2.y, c2.x);
          //restore();
        }

        /*
        if(goodScore < score){
          goodScore = score;
          keepMaze();
        }else if(T > 0 && (xor128() % 100) < 100.0 * rate){
          goodScore = score;
          keepMaze();
        }

        rollback();
        */

        T *= alpha;

        if(tryCount % span == 0){
          currentTime = getTime();
        }
      }

      restore();

      // -------------------- sa end -----------------------

      fprintf(stderr,"tryCount = %d\n", tryCount);

      if(g_debug){
        eval e = calcScore();
        fprintf(stderr,"Current = %d\n", e.pathLen);
        fprintf(stderr,"final remain f count = %d\n", g_F);
      }

      createQuery();
      fprintf(stderr,"query size = %lu\n", g_query.size());

      return g_query;
    }

    // 経路として確定している部分は変更が出来ない
    inline bool canChangedCell(int y, int x){
      return !(g_visitedOverall[y][x] || g_notChangedPath[y][x]);
    }

    inline void changeCell(int y, int x){
      int cList[3] = {S, R, L};
      g_tempMaze[y][x] = g_maze[y][x];

      cellData cd = getCellData(y, x);

      if(cd.tCnt[R] == 4){
        g_maze[y][x] = L;
      }else if(cd.tCnt[L] == 4){
        g_maze[y][x] = R;
      }else if(g_maze[y][x] == g_mazeOrigin[y][x]){
        int type = cList[xor128()%3];
        g_maze[y][x] = type;
      }else{
        g_maze[y][x] = g_mazeOrigin[y][x];
      }
    }

    inline void restoreCell(int y, int x){
      g_maze[y][x] = g_bestMaze[y][x];
    }

    inline bool inside(int y, int x){
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
      ++g_turn;

      resetMazeData();
      g_real = false;
      walk(y, x, curDir, origDir);

      //fprintf(stderr,"id = %d, fixCount = %d, pathLen = %d\n", id, g_fixCount, g_pathLen);

      if(g_fixCount == 0 || g_fixCount > g_F){
        return -10000.0;
      }else if(!g_success){
        return g_fixCount;
      }else{
        return (g_pathLen + g_changeValue) / (double)(g_fixCount);
      }
    }

    void walk(int y, int x, int curDir, int origDir){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];

      int sy = ny + DY[curDir];
      int sx = nx + DX[curDir];
      int ly = ny + DY[(curDir+3)%4];
      int lx = nx + DX[(curDir+3)%4];
      int ry = ny + DY[(curDir+1)%4];
      int rx = nx + DX[(curDir+1)%4];

      // 既に探索されていた場合は探索を抜ける
      if(g_visitedOnePath[ny][nx] == g_turn){
        return;
      }

      /**
       * 今回の探索で既に書き換えが発生していた場合はそれに置き換える
       */
      int type = (g_changedCheck[ny][nx] == g_turn)? g_tempMaze[ny][nx] : g_maze[ny][nx];

      g_visitedOnePath[ny][nx] = g_turn;

      //fprintf(stderr,"y = %d, x = %d, type = %c\n", ny, nx, g_cellType[type]);

      if(type == W){
        // 経路作成に成功
        g_success = true;

        for(int dy = 0; dy < g_height; dy++){
          for(int dx = 0; dx < g_width; dx++){
            int vCnt = g_visitedCount[dy][dx];

            if(g_visitedOnePath[dy][dx] == g_turn){
              // 今回生成した経路で変更が確定していない部分があれば追加する
              if(g_changedOnePath[dy][dx] == g_turn && g_changedCheck[dy][dx] != g_turn){
                if(g_maze[dy][dx] != U){
                  g_fixCount += 1;
                }else{
                  if(g_tempMaze[dy][dx] == S){
                    g_changeValue += 1;
                  }
                  g_fixCount += 1;
                }

                if(vCnt < 2){
                  g_changeValue += 1;
                }
                g_changedCheck[dy][dx] = g_turn;

                if(g_real){
                  g_maze[dy][dx] = g_tempMaze[dy][dx];
                  g_F -= 1;
                }
              }

              // 今回生成した経路でまだ未チェックの部分がある場合は経路長を伸ばす
              if(!g_visitedOverall[dy][dx]){
                g_pathLen += (vCnt < 1);
                g_visitedOverall[dy][dx] = 1;	
              }

              // 探索経路上のセルは変更出来ない
              if(g_real && !g_notChangedPath[dy][dx]){
                g_notChangedPath[dy][dx] = 1;
              }
            }
          }
        }
      }else if(type == S){
        walk(ny, nx, curDir, origDir);
      }else if(type == R){
        // 方向を変化させて結果元の方角と逆を向いてしまった場合
        if(turnRight(curDir) == g_nextDirect[origDir][2]){

          // セルが変更可能であれば修正する
          if(canChangedCell(ny,nx)){
            if(g_visitedCount[ly][lx] == 0){
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = L;
              walk(ny, nx, turnLeft(curDir), origDir);
            }else if(g_visitedCount[sy][sx] == 0){
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = S;
              walk(ny, nx, curDir, origDir);
            }else{
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = L;
              walk(ny, nx, turnLeft(curDir), origDir);
            }
          }
        }else{
          walk(ny, nx, turnRight(curDir), origDir);
        }
      }else if(type == U){
        //fprintf(stderr,"y = %d, x = %d, change U -> S\n", ny, nx);

        if(canChangedCell(ny,nx)){
          g_changedOnePath[ny][nx] = g_turn;
          g_tempMaze[ny][nx] = S;
          walk(ny, nx, curDir, origDir);
        }
      }else if(type == L){
        if(turnLeft(curDir) == g_nextDirect[origDir][2]){
          //fprintf(stderr,"y = %d, x = %d, change L -> R\n", ny, nx);

          if(canChangedCell(ny,nx)){
            if(g_visitedCount[ry][rx] == 0){
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = R;
              walk(ny, nx, turnRight(curDir), origDir);
            }else if(g_visitedCount[sy][sx] == 0){
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = S;
              walk(ny, nx, curDir, origDir);
            }else{
              g_changedOnePath[ny][nx] = g_turn;
              g_tempMaze[ny][nx] = R;
              walk(ny, nx, turnRight(curDir), origDir);
            }
          }
        }else{
          walk(ny, nx, turnLeft(curDir), origDir);
        }
      }else{
        for(int i = 0; i < 4; i++){
          walk(ny, nx, (curDir+i)%4, (curDir+i)%4);
        }
      }

      g_visitedOnePath[ny][nx] = -1;
    }

    inline int turnRight(int curDir){
      return g_nextDirect[curDir][1];
    }

    inline int turnLeft(int curDir){
      return g_nextDirect[curDir][3];
    }

    inline int u_turn(int curDir){
      return g_nextDirect[curDir][2];
    }

    eval calcScore(){
      eval e;
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));
      memset(g_visitedCount, 0, sizeof(g_visitedCount));
      ++g_turn;

      g_beforeNvis = 0;
      g_currentNvis = 0;

      for(int id = 0; id < g_ID; id++){
        EXPLORER *exp = getExplorer(id);

        search(exp->y, exp->x, exp->curDir);
        exp->pathLen = g_currentNvis - g_beforeNvis;
        g_beforeNvis = g_currentNvis;
      }

      int nvis = 0;

      for(int y = 0; y < g_height; ++y){
        for(int x = 0; x < g_width; ++x){
          if(inside(y, x) && g_visitedOverall[y][x]){
            ++nvis;
            ++e.pathLen;
          }
          e.fixCount += (g_maze[y][x] != g_mazeOrigin[y][x]);
        }
      }

      return e;
    }

    void beforeProc(){
      memset(g_visitedOverall, 0, sizeof(g_visitedOverall));
      memset(g_visitedCount, 0, sizeof(g_visitedCount));
      ++g_turn;

      for(int id = 0; id < g_ID; id++){
        EXPLORER *exp = getExplorer(id);

        search(exp->y, exp->x, exp->curDir);
        exp->pathLen = g_currentNvis - g_beforeNvis;
        g_beforeNvis = g_currentNvis;
      }
    }

    cellData getCellData(int y, int x){
      cellData cd;

      for(int i = 0; i < 4; i++){
        int ny = y + DY[i];
        int nx = x + DX[i];

        cd.tCnt[g_maze[ny][nx]] += 1;
      }

      return cd;
    }

    void search(int curY, int curX, int curDir){
      int ny = curY + DY[curDir];
      int nx = curX + DX[curDir];

      if(g_visitedOnePath[ny][nx] == g_turn){
        return;
      }

      g_visitedOnePath[ny][nx] = g_turn;

      int type = g_maze[ny][nx];

      if(type == W){
        for(int y = 0; y < g_height; y++){
          for(int x = 0; x < g_width; x++){
            if(g_visitedOnePath[y][x] == g_turn){
              g_visitedCount[y][x] += 1;

              if(!g_visitedOverall[y][x]){
                g_currentNvis += 1;
                g_visitedOverall[y][x] = 1;
              }
            }
          }
        }
      }else if(type == S){
        search(ny, nx, curDir);
      }else if(type == R){
        search(ny, nx, g_nextDirect[curDir][1]);
      }else if(type == U){
        search(ny, nx, g_nextDirect[curDir][2]);
      }else if(type == L){
        search(ny, nx, g_nextDirect[curDir][3]);
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          search(ny, nx, i);
        }
      }

      g_visitedOnePath[ny][nx] = -1;
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
  timeLimit = 2000;
  middleLimit = 1000;
  g_debug = true;
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
