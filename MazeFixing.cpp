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

ll timeLimit = 9000;
ll middleLimit = 4000;

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
bool g_commit;
bool g_faster;

const int DY[4] = {-1, 0, 1, 0};
const int DX[4] = { 0, 1, 0,-1};

const int MAX_WIDTH = 80;
const int MAX_HEIGHT = 80;

int g_F;
int g_FO;

int g_ID;

int g_N;

int g_width;

int g_height;

double g_fixCount;
int g_pathLen;
int g_vPathLen;
double g_changeValue;
ll g_turn;
map<int, bool> g_bestIdList;

int g_maze[MAX_HEIGHT*MAX_WIDTH];
int g_mazeOrigin[MAX_HEIGHT*MAX_WIDTH];
int g_tempMaze[MAX_HEIGHT*MAX_WIDTH];

int g_bestMaze[MAX_HEIGHT*MAX_WIDTH];
int g_goodMaze[MAX_HEIGHT*MAX_WIDTH];
int g_copyMaze[MAX_HEIGHT*MAX_WIDTH];

ll g_visitedOnePath[MAX_HEIGHT*MAX_WIDTH];

int g_mazeDirection[MAX_HEIGHT*MAX_WIDTH];

ll g_visitedOverall[MAX_HEIGHT*MAX_WIDTH];
ll g_visitedOnceall[MAX_HEIGHT*MAX_WIDTH];

int g_visitedCount[MAX_HEIGHT*MAX_WIDTH];

ll g_changedOnePath[MAX_HEIGHT*MAX_WIDTH];
ll g_changedCheck[MAX_HEIGHT*MAX_WIDTH];
ll g_notChangedPath[MAX_HEIGHT*MAX_WIDTH];

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

double g_beta = 10.0;

typedef struct EVAL{
  int pathLen;
  int vPathLen;
  int fixCount;

  EVAL(){
    this->pathLen = 0;
    this->fixCount = 0;
    this->vPathLen = 0;
  }

  double score(){
    return 3.5 * pathLen + 1.3 * vPathLen - g_beta * fixCount;
  }
} eval;

vector<string> g_query;
vector<EXPLORER> g_explorerList;
vector<int> g_cellCoordList;

int g_cellCount;

class MazeFixing{
  public:

    void init(vector<string> maze, int F){
      g_F = F;
      g_FO = F;
      g_ID = 0;
      g_turn = 0;
      g_N = 0;
      g_height = maze.size();
      g_width = maze[0].size();
      memset(g_maze, W, sizeof(g_maze));
      memset(g_tempMaze, W, sizeof(g_tempMaze));
      memset(g_notChangedPath, 0, sizeof(g_notChangedPath));
      memset(g_visitedOverall, -1, sizeof(g_visitedOverall));
      memset(g_visitedOnePath, -1, sizeof(g_visitedOnePath));
      memset(g_changedCheck, -1, sizeof(g_changedCheck));
      memset(g_changedOnePath, -1, sizeof(g_changedOnePath));

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          char type = maze[y][x];
          int z = getZ(y,x);

          if(type == 'R'){
            g_maze[z] = R;
          }else if(type == 'L'){
            g_maze[z] = L;
          }else if(type == 'U'){
            g_maze[z] = U;
          }else if(type == 'S'){
            g_maze[z] = S;
          }else if(type == 'E'){
            g_maze[z] = E;
            g_notChangedPath[z] = 9999;
          }else{
            g_maze[z] = W;
          }

          if(type != '.'){
            g_N++;
          }
        }
      }

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);

          if(g_maze[z] == W){
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
    }

    EXPLORER createExplorer(int y, int x, int curDir){
      EXPLORER exp(g_ID, y, x, curDir);
      g_ID++;

      return exp;
    }

    void initCoordList(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);
          int type = g_mazeOrigin[z];

          if(type != W && type != S && type != E){
            g_cellCoordList.push_back(z);
            g_cellCount += 1;
          }
        }
      }
    }

    inline int getZ(int y, int x){
      return y * MAX_WIDTH + x;
    }

    inline EXPLORER *getExplorer(int id){
      return &g_explorerList[id];
    }

    void createQuery(){
      fprintf(stderr,"createQuery =>\n");

      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);

          if(g_maze[z] != g_mazeOrigin[z]){
            string query = "";
            query += int2string(y) + " ";
            query += int2string(x) + " ";
            query += g_cellType[g_maze[z]];
            g_query.push_back(query);
          }
        }
      }
    }

    void resetMaze(){
      memcpy(g_maze, g_mazeOrigin, sizeof(g_mazeOrigin));
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

      const ll startTime = getTime();
      ll endTime = startTime + timeLimit;
      ll middleTime = startTime + middleLimit;
      ll currentTime = getTime();
      g_faster = true;
      int tryCount = 0;

      while(g_F > 0 && currentTime < endTime){
        tryCount += 1;
        double maxValue = -100.0;
        bool update = false;
        int bestId = -1;

        beforeProc();

        for(int id = 0; id < g_ID; id++){
          if(g_bestIdList[id]) continue;

          EXPLORER *exp = getExplorer(id);

          double value = 0.0;
          value = calcWalkValue(exp->y, exp->x, exp->curDir, exp->curDir);

          if(maxValue < value && !g_bestIdList[id]){
            maxValue = value;
            bestId = id;
            update = true;
          }
        }

        if(update){
          EXPLORER *exp = getExplorer(bestId);

          commitWalk(exp);

          g_bestIdList[bestId] = true;

          if(tryCount % 2 == 0){
            currentTime = getTime();
          }

          if(currentTime > middleTime){
            g_faster = false;
          }
        }else{
          break;
        }
      }
      
      saveMaze();
      keepMaze();

      ll ct = currentTime-startTime;

      if(g_F > g_FO * 0.01){
        g_beta = (g_N/(double)g_FO);
      }else{
        g_beta = 2.0 * (g_N/(double)g_FO);
      }
      double alpha = 0.999;

      if(g_faster){
        currentTime = getTime();

        eval e = calcScore();
        double bestScore = e.score();
        double goodScore = e.pathLen / (double)g_N;

        initCoordList();
        int span = 50;

        if(ct < 100){
          endTime = startTime + 9500;
          span = 250;
        }

        fprintf(stderr,"span = %d, currentTime = %lld\n", span, ct);

        while(currentTime < endTime){
          int z = g_cellCoordList[xor128()%g_cellCount];
          int z2 = g_cellCoordList[xor128()%g_cellCount];

          tryCount += 1;

          changeCell(z);
          changeCell(z2);

          eval e = calcScore();
          double score = e.score();
          double realScore = e.pathLen / (double)g_N;

          if(goodScore < realScore && e.fixCount <= g_FO){
            goodScore = realScore;
            keepMaze();
          }

          if(bestScore < score && e.fixCount <= g_FO){
            //fprintf(stderr,"beta = %f, fixCount: (%d/%d), update score %f, (%d/%d)\n", g_beta, e.fixCount, g_FO, realScore, e.pathLen, g_N);
            bestScore = score;
            saveMaze();
            g_beta = max(1.0, g_beta * alpha);
          }else{
            restoreCell(z);
            restoreCell(z2);
          }

          if(tryCount % span == 0){
            currentTime = getTime();
          }
        }
      }

      rollback();

      fprintf(stderr,"tryCount = %d\n", tryCount);

      if(g_debug){
        eval e = calcScore();
        fprintf(stderr,"Current = %f\n", e.pathLen / (double)g_N);
        fprintf(stderr,"final remain f count = %d\n", g_F);
      }

      createQuery();
      fprintf(stderr,"query size = %lu\n", g_query.size());

      return g_query;
    }

    inline bool canChangedCell(int y, int x){
      int z = getZ(y,x);
      return !(g_notChangedPath[z] > 0 || g_visitedOverall[z] == g_turn);
    }

    inline void changeCell(int z){
      int cList[3] = {S, R, L};
      g_tempMaze[z] = g_maze[z];

      if(g_maze[z] == g_mazeOrigin[z]){
        int type = cList[xor128()%3];
        g_maze[z] = type;
      }else{
        g_maze[z] = g_mazeOrigin[z];
      }
    }

    inline void restoreCell(int z){
      g_maze[z] = g_bestMaze[z];
    }

    inline void resetCell(int z){
      g_maze[z] = g_mazeOrigin[z];
    }

    inline bool inside(int z){
      return g_maze[z] != W;
    }

    inline bool outside(int y, int x){
      int z = getZ(y,x);
      if(y < 0 || x < 0 || g_height <= y || g_width <= x) return false;
      if(g_maze[z] == W) return false;

      for(int i = 0; i < 4; i++){
        int ny = y + DY[i];
        int nx = x + DX[i];
        int nz = getZ(ny,nx);

        if(g_maze[nz] == W){
          return true;
        }
      }

      return false;
    }

    void commitWalk(explorer *exp){
      g_commit = true;
      ++g_turn;
      walk(exp->y, exp->x, exp->curDir, exp->curDir, 0);
    }

    double calcWalkValue(int y, int x, int curDir, int origDir){
      g_fixCount = 0.0;
      g_pathLen = 0;
      g_vPathLen = 0;
      g_changeValue = 0.0;
      ++g_turn;

      g_commit = false;
      walk(y, x, curDir, origDir, 0);

      if(g_fixCount <= 0 || g_fixCount > g_F){
        return -10000.0;
      }else if(!g_faster){
        return g_pathLen;
      }else{
        return (g_pathLen - 0.01 * g_vPathLen + g_changeValue) / (double)(g_fixCount);
      }
    }

    void walk(int curY, int curX, int curDir, int origDir, int fixCount = 0){
      int ny = curY + DY[curDir];
      int nx = curX + DX[curDir];
      int nz = getZ(ny,nx);

      int sy = ny + DY[curDir];
      int sx = nx + DX[curDir];
      int sz = getZ(sy,sx);

      int ly = ny + DY[(curDir+3)%4];
      int lx = nx + DX[(curDir+3)%4];
      int lz = getZ(ly,lx);

      int ry = ny + DY[(curDir+1)%4];
      int rx = nx + DX[(curDir+1)%4];
      int rz = getZ(ry,rx);

      if(g_visitedOnePath[nz] == g_turn || fixCount > g_F){
        return;
      }

      int type = (g_changedCheck[nz] == g_turn)? g_tempMaze[nz] : g_maze[nz];

      if(g_visitedOnceall[nz] != g_turn){
        g_visitedOnceall[nz] = g_turn;
        g_vPathLen += 1;
      }

      g_visitedOnePath[nz] = g_turn;
      g_mazeDirection[nz] = curDir;

      if(type == W){
        int curZ = getZ(curY,curX);

        while(g_maze[curZ] != W){
          int vCnt = g_visitedCount[curZ];

          if(g_changedOnePath[curZ] == g_turn && g_changedCheck[curZ] != g_turn){
            g_fixCount += 1;

            if(g_maze[curZ] == U){
              g_changeValue += 1.1;
            }
            if(vCnt <= 2){
              g_changeValue += 1.0;
            }
            g_changedCheck[curZ] = g_turn;

            if(g_commit){
              g_maze[curZ] = g_tempMaze[curZ];
              g_F -= 1;
            }
          }

          if(g_commit && g_visitedOverall[curZ] != g_turn){
            g_notChangedPath[curZ] += 1;
          }

          if(g_visitedOverall[curZ] != g_turn){
            if(vCnt == 0){
              g_pathLen += 1;
            }
            g_visitedOverall[curZ] = g_turn;
          }

          curDir = g_mazeDirection[curZ];
          curY += DY[(curDir+2)%4];
          curX += DX[(curDir+2)%4];
          curZ = getZ(curY,curX);
        }
      }else if(type == S){
        walk(ny, nx, curDir, origDir, fixCount);
      }else if(type == R){
        if(turnRight(curDir) == g_nextDirect[origDir][2] && canChangedCell(ny,nx)){

          g_changedOnePath[nz] = g_turn;

          if(g_visitedCount[lz] == 0){
            g_tempMaze[nz] = L;
            walk(ny, nx, turnLeft(curDir), origDir, fixCount+1);
          }else if(g_visitedCount[sz] == 0){
            g_tempMaze[nz] = S;
            walk(ny, nx, curDir, origDir, fixCount+1);
          }else{
            g_tempMaze[nz] = L;
            walk(ny, nx, turnLeft(curDir), origDir, fixCount+1);
          }
        }else{
          walk(ny, nx, turnRight(curDir), origDir, fixCount);
        }
      }else if(type == U && canChangedCell(ny, nx)){

        g_changedOnePath[nz] = g_turn;
        g_tempMaze[nz] = S;
        walk(ny, nx, curDir, origDir, fixCount+1);
      }else if(type == L){
        if(turnLeft(curDir) == g_nextDirect[origDir][2] && featureDir(ny,nx,curDir) != R && canChangedCell(ny,nx)){

          g_changedOnePath[nz] = g_turn;

          if(g_visitedCount[rz] == 0){
            g_tempMaze[nz] = R;
            walk(ny, nx, turnRight(curDir), origDir, fixCount+1);
          }else if(g_visitedCount[sz] == 0){
            g_tempMaze[nz] = S;
            walk(ny, nx, curDir, origDir, fixCount+1);
          }else{
            g_tempMaze[nz] = R;
            walk(ny, nx, turnRight(curDir), origDir, fixCount+1);
          }
        }else{
          walk(ny, nx, turnLeft(curDir), origDir, fixCount);
        }
      }else if(type == E){
        for(int i = 0; i < 4; i++){
          walk(ny, nx, (curDir+i)%4, (curDir+i)%4, fixCount);
        }
      }

      g_visitedOnePath[nz] = -1;
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

    int featureDir(int y, int x, int curDir){
      int ny = y + DY[curDir];
      int nx = x + DX[curDir];
      int nz = getZ(ny,nx);
      int type = (g_changedCheck[nz] == g_turn)? g_tempMaze[nz] : g_maze[nz];

      while(type != S && type != W){
        ny = ny + DY[curDir];
        nx = nx + DX[curDir];
        nz = getZ(ny,nx);
        type = (g_changedCheck[nz] == g_turn)? g_tempMaze[nz] : g_maze[nz];
      }

      return type;
    }

    eval calcScore(){
      eval e;
      g_vPathLen = 0;
      memset(g_visitedCount, 0, sizeof(g_visitedCount));
      ++g_turn;

      for(int id = 0; id < g_ID; id++){
        EXPLORER *exp = getExplorer(id);

        search(exp->y, exp->x, exp->curDir);
      }

      for(int y = 0; y < g_height; ++y){
        for(int x = 0; x < g_width; ++x){
          int z = getZ(y,x);

          if(inside(z) && g_visitedOverall[z] == g_turn){
            ++e.pathLen;
          }
          e.fixCount += (g_maze[z] != g_mazeOrigin[z]);
        }
      }

      e.vPathLen = g_vPathLen - e.pathLen;
      g_F = g_FO - e.fixCount;

      return e;
    }

    void beforeProc(){
      memset(g_visitedCount, 0, sizeof(g_visitedCount));
      ++g_turn;

      for(int id = 0; id < g_ID; ++id){
        EXPLORER *exp = getExplorer(id);

        search(exp->y, exp->x, exp->curDir);
      }
    }

    void search(int curY, int curX, int curDir){
      int ny = curY + DY[curDir];
      int nx = curX + DX[curDir];
      int nz = getZ(ny,nx);

      if(g_visitedOnePath[nz] == g_turn){
        return;
      }
      if(g_visitedOnceall[nz] != g_turn){
        g_visitedOnceall[nz] = g_turn;
        g_vPathLen += 1;
      }

      g_visitedOnePath[nz] = g_turn;
      g_mazeDirection[nz] = curDir;

      int type = g_maze[nz];

      if(type == W){
        int curZ = getZ(curY,curX);

        while(g_maze[curZ] != W){
          g_visitedCount[curZ] += 1;

          if(g_visitedOverall[curZ] != g_turn){
            g_visitedOverall[curZ] = g_turn;
          }

          curDir = g_mazeDirection[curZ];
          curY += DY[(curDir+2)%4];
          curX += DX[(curDir+2)%4];
          curZ = getZ(curY,curX);
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

      g_visitedOnePath[nz] = -1;
    }

    void showMaze(){
      for(int y = 0; y < g_height; y++){
        for(int x = 0; x < g_width; x++){
          int z = getZ(y,x);
          int type = g_maze[z];
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
  timeLimit = 3000;
  middleLimit = 10000;
  g_debug = true;
  vector<string> ret = mf.improve(maze, f);
  cout << ret.size() << endl;
  for(int i = 0; i < ret.size(); i++){
    cout << ret[i] << endl;
  }
  return 0;
}
