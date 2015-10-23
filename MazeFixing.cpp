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

vector<int> g_path;
vector<int> g_cellTypes;

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
int g_ECount;

// 探索者のID
int g_ID;

// 壁ではないセルの数
int g_N;

// 迷路の横幅
int g_width;

// 迷路の縦幅
int g_height;

double g_fixCount;
double g_fixRate;
int g_pathLen;
int g_vPathLen;
int g_vFixCount;
double alpha = 1.5;
double beta = 0.0;
double g_changeValue;
ll g_turn;
map<int, bool> g_bestIdList;

int g_maze[MAX_HEIGHT*MAX_WIDTH];
int g_mazeOrigin[MAX_HEIGHT*MAX_WIDTH];
int g_tempMaze[MAX_HEIGHT*MAX_WIDTH];

int g_bestMaze[MAX_HEIGHT*MAX_WIDTH];
int g_goodMaze[MAX_HEIGHT*MAX_WIDTH];
int g_copyMaze[MAX_HEIGHT*MAX_WIDTH];

int g_outSideDist[MAX_HEIGHT*MAX_WIDTH];

// 探索中に訪れた部分にチェックを付ける
ll g_visitedOnePath[MAX_HEIGHT*MAX_WIDTH];

int g_mazeDirection[MAX_HEIGHT*MAX_WIDTH];

// 探索済みの箇所についてチェックをつける
ll g_visitedOverall[MAX_HEIGHT*MAX_WIDTH];
ll g_visitedOnceall[MAX_HEIGHT*MAX_WIDTH];

int g_visitedCount[MAX_HEIGHT*MAX_WIDTH];

double g_effectMap[MAX_HEIGHT*MAX_WIDTH];

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
  double value;

	EVAL(){
		this->pathLen = 0;
		this->fixCount = 0;
		this->vPathLen = 0;
		this->unreachedCellCount = 0;
    this->value = 0.0;
	}

	double score(){
		return 4.0 * pathLen + alpha * vPathLen + value - beta * fixCount;
	}
} eval;

vector<EXPLORER> g_explorerList;
vector<string> g_query;
vector<int> g_cellCoordList;

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
			g_ECount = 0;
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
						g_RCount += 1;
					}else if(type == 'L'){
						g_maze[z] = L;
						g_LCount += 1;
					}else if(type == 'U'){
						g_maze[z] = U;
						g_UCount += 1;
					}else if(type == 'S'){
						g_maze[z] = S;
						g_SCount += 1;
					}else if(type == 'E'){
						g_maze[z] = E;
						g_notChangedPath[z] = 9999;
						g_ECount += 1;
					}else{
						g_maze[z] = W;
					}

					if(type != '.'){
						g_N++;

					}
					g_outSideDist[z] = 99;
				}
			}

			g_fixRate = g_F / (double)g_N;

			for(int y = 0; y < g_height; y++){
				for(int x = 0; x < g_width; x++){
					int z = getZ(y,x);
					g_effectMap[z] = 1.0;

					if(g_maze[z] == W){
						for(int i = 0; i < 4; i++){
							int ny = y + DY[i];
							int nx = x + DX[i];

							if(outside(ny, nx)){
								EXPLORER exp = createExplorer(y,x,i);
								g_explorerList.push_back(exp);
								//setOutSideDist(ny, nx);
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

		void setOutSideDist(int y, int x){
			queue<coord> que;
			que.push(coord(y,x));
			map<int, bool> checkList;

			while(!que.empty()){
				coord c = que.front(); que.pop();

				int z = getZ(c.y, c.x);
				if(checkList[z] || g_maze[z] == W || g_outSideDist[z] < c.dist) continue;
				checkList[z] = true;
				g_outSideDist[z] = c.dist;

				for(int i = 0; i < 4; i++){
					int ny = c.y + DY[i];
					int nx = c.x + DX[i];

					que.push(coord(ny,nx,c.dist+1));
				}
			}
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

			if(g_debug){
				fprintf(stderr,"Before: R: %d, L: %d, S: %d, U: %d, E: %d\n", g_RCount, g_LCount, g_SCount, g_UCount, g_ECount);
			}

			const ll startTime = getTime();
			ll endTime = startTime + timeLimit;
			ll middleTime = startTime + middleLimit;
			ll currentTime = getTime();
			int tryCount = 0;
      double T = 100.0;
      double k = 10.0;

			// ---------------------- sa -----------------------

			currentTime = getTime();

			eval e = calcScore();
			double bestScore = e.score();
      double goodScore = bestScore;
      int notChangeCount = 0;

			saveMaze();
      keepMaze();
			initCoordList();

			int span = (currentTime < middleTime)? 1000 : 30;
			fprintf(stderr,"span = %d, currentTime = %lld\n", span, currentTime-startTime);

			while(currentTime < endTime){
				int z = g_cellCoordList[xor128()%g_cellCount];
				int z2 = g_cellCoordList[xor128()%g_cellCount];
				int z3 = g_cellCoordList[xor128()%g_cellCount];

				tryCount += 1;

				changeCell(z);
				changeCell(z2);
				changeCell(z3);

				eval e = calcScore();
				double score = e.score();

        if(e.fixCount > g_FO){
          score *= (1 - 0.001 * (e.fixCount - g_FO));
        }

				if(bestScore < score){
          fprintf(stderr,"alpha = %f, beta = %f, update score = %f : (%d/%d)\n", alpha, beta, e.pathLen/(double)g_N, e.pathLen, g_N);
          fprintf(stderr,"fixCount = %d, g_FO = %d\n", e.fixCount, g_FO);
					bestScore = score;
          beta += 0.7 / (double)g_FO;
          alpha = min(2.0, max(0.0, alpha - 0.1));
					saveMaze();
				}else{
          notChangeCount += 1;
					restoreCell(z);
					restoreCell(z2);
					restoreCell(z3);
					//restore();

          if(notChangeCount >= 10000){
            alpha = min(2.0, alpha + 0.1);
            notChangeCount = 0;
          }
				}

        /*
        if(goodScore < score){
          goodScore = score;
          keepMaze();
        }else if(xor128()%100 < 100.0 * rate){
          goodScore = score;
          keepMaze();
        }

        rollback();
        */

				if(tryCount % span == 0){
					currentTime = getTime();
				}
			}

			restore();

			// -------------------- sa end -----------------------

			fprintf(stderr,"tryCount = %d\n", tryCount);

			if(g_debug){
				eval e = calcScore();
				fprintf(stderr,"Current = %f\n", e.pathLen / (double)g_N);
				fprintf(stderr,"final remain f count = %d\n", g_F);
				fprintf(stderr,"Result: R: %d, L: %d, S: %d, U: %d, E: %d\n", g_RCount, g_LCount, g_SCount, g_UCount, g_ECount);
			}

			createQuery();
			fprintf(stderr,"query size = %lu\n", g_query.size());

			return g_query;
		}

		void swapCell(coord &c1, coord &c2){
			int cz = getZ(c1.y, c1.x);
			int cz2 = getZ(c2.y, c2.x);
			int temp = g_maze[cz];
			g_maze[cz] = g_maze[cz2];
			g_maze[cz2] = temp;
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

		bool outside(int y, int x){
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
			memset(g_visitedCount, 0, sizeof(g_visitedCount));
			g_vPathLen = 0;
			g_vFixCount = 0;
			++g_turn;

			for(int id = 0; id < g_ID; id++){
				EXPLORER *exp = getExplorer(id);

				search(exp->y, exp->x, exp->curDir);
			}

			for(int y = 0; y < g_height; ++y){
				for(int x = 0; x < g_width; ++x){
					int z = getZ(y,x);

					if(inside(z) && g_visitedOverall[z] == g_turn){
            e.pathLen += 1;
					}
					e.fixCount += (g_maze[z] != g_mazeOrigin[z]);
				}
			}

      e.vPathLen = g_vPathLen - e.pathLen;
      g_F = g_FO - e.fixCount;

			return e;
		}

		bool search(int curY, int curX, int curDir){
			int ny = curY + DY[curDir];
			int nx = curX + DX[curDir];
			int nz = getZ(ny,nx);
      bool success = false;

			if(g_visitedOnePath[nz] == g_turn){
				return false;
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

        return true;
			}else if(type == S){
				success = search(ny, nx, curDir) || success;
			}else if(type == R){
				success = search(ny, nx, turnRight(curDir)) || success;
			}else if(type == U){
				success = search(ny, nx, u_turn(curDir)) || success;
			}else if(type == L){
				success = search(ny, nx, turnLeft(curDir)) || success;
			}else if(type == E){
				for(int i = 0; i < 4; i++){
					success = search(ny, nx, i) || success;
				}
			}

			g_visitedOnePath[nz] = -1;
      return success;
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
	timeLimit = 600000;
	middleLimit = 10000;
	g_debug = true;
	vector<string> ret = mf.improve(maze, f);
	cout << ret.size() << endl;
	for(int i = 0; i < ret.size(); i++){
		cout << ret[i] << endl;
	}
	return 0;
}
