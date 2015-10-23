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
bool g_faster;

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

typedef struct PATH_DATA{
	int id;
	int length;
	vector<int> route;
	vector<int> types;
	double value;

	bool operator >(const PATH_DATA &p) const{
		return value > p.value;
	}
} path_data;

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

	CellData(){
		memset(tCnt, 0, sizeof(tCnt));
	}

} cellData;

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

		bool isCentral(int y, int x){
			double span = 0.33;
			double dy = g_height * span;
			double dx = g_width * span;

			return (dy < y && y < (g_height-dy) && dx < x && x < (g_width-dx));
		}

		void mazeClean(){
			for(int y = 0; y < g_height; y++){
				for(int x = 0; x < g_width; x++){
					int z = getZ(y,x);

					if(g_F > 0 && isCentral(y,x) && g_maze[z] == U){
						g_F--;
						g_maze[z] = S;
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
			//mazeClean();

			if(g_debug){
				fprintf(stderr,"Before: R: %d, L: %d, S: %d, U: %d, E: %d\n", g_RCount, g_LCount, g_SCount, g_UCount, g_ECount);
			}

			const ll startTime = getTime();
			ll endTime = startTime + timeLimit;
			ll middleTime = startTime + middleLimit;
			ll currentTime = getTime();
			g_faster = false;
			int tryCount = 0;
			int bestFixCount;
			int bestLength;
			double minValue = INT_MAX;
			priority_queue<path_data, vector<path_data>, greater<path_data> > pque;

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

					//fprintf(stderr,"y = %d, x = %d, d = %d, value = %4.2f\n", exp->y, exp->x, exp->curDir, value);

					if(maxValue < value && !g_bestIdList[id]){
						maxValue = value;
						bestId = id;
						bestFixCount = g_vFixCount;
						bestLength = g_vPathLen;
						update = true;
					}
				}

				if(update){
					EXPLORER *exp = getExplorer(bestId);

					commitWalk(exp);

					path_data pd = createPathData(bestId, maxValue);
					pque.push(pd);

					g_bestIdList[bestId] = true;

					if(tryCount % 2 == 0){
						currentTime = getTime();
					}

					if(false && minValue < maxValue){
					//if(tryCount % 10 == 0){
						pd = pque.top(); pque.pop();
						g_bestIdList[pd.id] = false;
						restorePath(pd.route);
						fprintf(stderr,"length = %d, value = %f\n", pd.length, pd.value);
						minValue = INT_MAX;
					}else{
						minValue = maxValue;
					}

					if(currentTime > middleTime){
						g_faster = true;
					}
				}else{
					break;
				}
			}

			// ---------------------- sa -----------------------

			currentTime = getTime();

			eval e = calcScore();
			double bestScore = e.score();

			saveMaze();
			initCoordList();

			int span = (currentTime < middleTime)? 1000 : 30;
			fprintf(stderr,"span = %d, currentTime = %lld\n", span, currentTime-startTime);

			while(currentTime < endTime && !g_faster){
				int z = g_cellCoordList[xor128()%g_cellCount];
				int z2 = g_cellCoordList[xor128()%g_cellCount];

				tryCount += 1;

				changeCell(z);
				changeCell(z2);

				eval e = calcScore();
				double score = e.score();

				//double rate = exp(-(goodScore-score) / (k*T));

				if(bestScore < score && e.fixCount <= g_FO){
					bestScore = score;
					saveMaze();
				}else{
					restoreCell(z);
					restoreCell(z2);
					//restore();
				}

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

		inline bool canChangedCell(int y, int x){
			int z = getZ(y,x);
			return !(g_notChangedPath[z] > 0 || g_visitedOverall[z] == g_turn);
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

		void restorePath(vector<int> &path){
			int length = path.size();

			for(int i = 0; i < length; i++){
				int z = path[i];

				if(g_maze[z] != g_mazeOrigin[z] && g_notChangedPath[z] == 1){
					g_notChangedPath[z] = 0;
					resetCell(z);
					g_F++;
				}
			}
		}

		void commitPath(path_data &pd){
			for(int i = 0; i < pd.length; i++){
				int z = pd.route[i];
				int type = pd.types[i];

				g_maze[z] = type;
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

		path_data createPathData(int id, double value){
			path_data pd;
			pd.id = id;
			pd.route = g_path;
			pd.types = g_cellTypes;
			pd.length = g_path.size();
			pd.value = value;

			return pd;
		}

		void commitWalk(explorer *exp){
			g_real = true;
			g_path = vector<int>();
			g_cellTypes = vector<int>();
			++g_turn;
			walk(exp->y, exp->x, exp->curDir, exp->curDir, 0);
		}

		double calcWalkValue(int y, int x, int curDir, int origDir){
			g_fixCount = 0.0;
			g_pathLen = 0;
			g_vPathLen = 0;
			g_vFixCount = 0;
			g_changeValue = 0.0;
			++g_turn;

			g_real = false;
			walk(y, x, curDir, origDir, 0);

			if(g_fixCount <= 0 || g_fixCount > g_F){
				return -10000.0;
			}else if(g_faster){
				return g_pathLen;
			}else{
				return (g_pathLen - 0.01 * g_vPathLen + g_changeValue) / (double)(g_fixCount);
			}
		}

		void walk(int curY, int curX, int curDir, int origDir, int fixCount = 0, int pathLen = 0){
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
							g_changeValue += 1.0;
						}
						if(vCnt <= 2){
							g_changeValue += 1.0;
						}
						g_changedCheck[curZ] = g_turn;

						if(g_real){
							g_maze[curZ] = g_tempMaze[curZ];
							g_F -= 1;
						}
					}

					if(g_changedOnePath[curZ] == g_turn){
						g_vFixCount += 1;
					}

					if(g_real && g_visitedOverall[curZ] != g_turn){
						g_path.push_back(curZ);
						g_cellTypes.push_back(g_maze[curZ]);

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
				walk(ny, nx, curDir, origDir, fixCount, pathLen+1);
			}else if(type == R){
				if(turnRight(curDir) == g_nextDirect[origDir][2] && canChangedCell(ny,nx)){

					g_changedOnePath[nz] = g_turn;

					if(g_visitedCount[lz] == 0){
						g_tempMaze[nz] = L;
						walk(ny, nx, turnLeft(curDir), origDir, fixCount+1, pathLen+1);
					}else if(g_visitedCount[sz] == 0){
						g_tempMaze[nz] = S;
						walk(ny, nx, curDir, origDir, fixCount+1, pathLen+1);
					}else{
						g_tempMaze[nz] = L;
						walk(ny, nx, turnLeft(curDir), origDir, fixCount+1, pathLen+1);
					}
				}else{
					walk(ny, nx, turnRight(curDir), origDir, fixCount, pathLen+1);
				}
			}else if(type == U && canChangedCell(ny, nx)){

				g_changedOnePath[nz] = g_turn;
				g_tempMaze[nz] = S;
				walk(ny, nx, curDir, origDir, fixCount+1, pathLen+1);
			}else if(type == L){
				if(turnLeft(curDir) == g_nextDirect[origDir][2] && featureDir(ny,nx,curDir) != R && canChangedCell(ny,nx)){

					g_changedOnePath[nz] = g_turn;

					if(g_visitedCount[rz] == 0){
						g_tempMaze[nz] = R;
						walk(ny, nx, turnRight(curDir), origDir, fixCount+1, pathLen+1);
					}else if(g_visitedCount[sz] == 0){
						g_tempMaze[nz] = S;
						walk(ny, nx, curDir, origDir, fixCount+1, pathLen+1);
					}else{
						g_tempMaze[nz] = R;
						walk(ny, nx, turnRight(curDir), origDir, fixCount+1, pathLen+1);
					}
				}else{
					walk(ny, nx, turnLeft(curDir), origDir, fixCount, pathLen+1);
				}
			}else if(type == E){
				for(int i = 0; i < 4; i++){
					walk(ny, nx, (curDir+i)%4, (curDir+i)%4, fixCount, pathLen+1);
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

		cellData getCellData(int y, int x){
			cellData cd;
			int z = getZ(y,x);

			if(g_maze[z] == W) return cd;

			for(int i = 0; i < 4; i++){
				int ny = y + DY[i];
				int nx = x + DX[i];
				int nz = getZ(ny,nx);

				cd.tCnt[g_maze[nz]] += 1;
			}

			return cd;
		}

		void search(int curY, int curX, int curDir){
			int ny = curY + DY[curDir];
			int nx = curX + DX[curDir];
			int nz = getZ(ny,nx);

			if(g_visitedOnePath[nz] == g_turn){
				return;
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
