#include <iostream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <memory>
#include <algorithm>


using namespace std;

const int MAP_SIZE = 150;

class PMap
{
public:
    int height[MAP_SIZE][MAP_SIZE];

    std::vector<int> playerIds;

    PMap() {
        playerIds.clear();
    }
    PMap(const char *fileName) {
        playerIds.clear();
        playerIds.push_back(-1);
        playerIds.push_back(-1);
        playerIds.push_back(-1);

        ifstream fin(fileName);
        for (int i=0; i<MAP_SIZE; ++i)
            for (int j=0; j<MAP_SIZE; ++j)
                fin >> height[i][j];
        fin.close();
    }
    int* operator[](int width) {return height[width];}
    int getHeight (int x, int y) const {
        if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE)
        {
            cerr << "postion outside map" << endl;
            return -1;
        }
        return height[x][y];
    }
};

struct Pos
{
    int x, y;
    Pos(int x, int y)
        :x(x), y(y)
    {}
};

bool operator==(Pos A, Pos B)
{
    return A.x == B.x && A.y == B.y;
}

int crossProduct(Pos A, Pos B, Pos C)
{
    return (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
}
bool cross(Pos A, Pos B, Pos C, Pos D)
{
    return (long long) crossProduct(A, B, C) * crossProduct(A, B, D) <= 0
        && (long long) crossProduct(C, D, A) * crossProduct(C, D, B) <= 0;
}

bool checkOnLine(Pos A, Pos B, Pos C)
{
    if (A == B)
    return false;
    Pos a(A.x * 2, A.y * 2), b(B.x * 2, B.y * 2), c(C.x * 2, C.y * 2);
    return cross(a, b, Pos(c.x + 1, c.y + 1), Pos(c.x + 1, c.y - 1))
        || cross(a, b, Pos(c.x + 1, c.y + 1), Pos(c.x - 1, c.y + 1))
        || cross(a, b, Pos(c.x - 1, c.y - 1), Pos(c.x + 1, c.y - 1))
        || cross(a, b, Pos(c.x - 1, c.y - 1), Pos(c.x - 1, c.y + 1));
}

// A-star algorithm.
// Depends on: std::queue in <queue>, std::sqrt in <cmath>, std::reverse in <algorithm>, MAP_SIZE in "const.h", checkOnLine() in "Pos.h".
void findPath(const PMap &map, Pos start, Pos dest, const vector<Pos> &blocks, vector<Pos> &path)
{
    struct Node {
        int x, y;
        int level;
        int priority;

        Node() :x(0), y(0), level(0), priority(0) {}
        Node(int xp, int yp, int d, int p) :x(xp), y(yp), level(d), priority(p) {}
        void updatePriority(int xDest, int yDest)
        { priority = level + estimate(xDest, yDest) * 9; }
        void nextLevel()
        { level += 10; }
        int estimate(const int & xDest, const int & yDest) const {
            int xd = xDest - x, yd = yDest - y;
            return static_cast<int>(sqrt(xd * xd + yd * yd));
        }
        bool operator < (const Node &b) const
        { return priority > b.priority; }
    };
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1, 0, -1};
    static int open_nodes_map[MAP_SIZE][MAP_SIZE];
    static unsigned char closed_nodes_map[MAP_SIZE][MAP_SIZE];
    static unsigned char dir_map[MAP_SIZE][MAP_SIZE];
    priority_queue<Node> pq[2];
    int pqi;
    int i, j, x, y, xdx, ydy;

    memset(closed_nodes_map, 0, sizeof(closed_nodes_map));
    memset(open_nodes_map, 0, sizeof(open_nodes_map));
    if (start == dest)
        return;
    for (size_t i = 0; i < blocks.size(); ++i)
        if (blocks[i].x >= 0 && blocks[i].x < MAP_SIZE && blocks[i].y >= 0 && blocks[i].y < MAP_SIZE)
            closed_nodes_map[blocks[i].x][blocks[i].y] = 1;
    if (closed_nodes_map[dest.x][dest.y] == 1) {
        xdx = start.x < dest.x ? -1 : 1;
        ydy = start.y < dest.y ? -1 : 1;
        x = dest.x; y = dest.y;
        while (closed_nodes_map[x][y] == 1) {
            if (checkOnLine(start, dest, Pos(x + xdx, y)))
                x += xdx;
            else
                y += ydy;
        }
        dest = Pos(x, y);
    }
    Node n0(start.x, start.y, 0, 0);
    n0.updatePriority(dest.x, dest.y);
    pqi=0;
    pq[pqi].push(n0);
    open_nodes_map[start.x][start.y] = n0.priority;
    while(!pq[pqi].empty()) {
        n0 = pq[pqi].top();
        pq[pqi].pop();
        x = n0.x;
        y = n0.y;
        open_nodes_map[x][y] = 0;
        closed_nodes_map[x][y] = 1;
        if(x == dest.x && y == dest.y) {
            while(!(x == start.x && y == start.y)) {
                j = dir_map[x][y];
                path.push_back(Pos(x, y));
                x += dx[j];
                y += dy[j];
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return;
        }
        for(i = 0; i < 4; i++) {
            xdx=x+dx[i]; ydy=y+dy[i];
            if(!(xdx < 0 || xdx >= MAP_SIZE || ydy < 0 || ydy >= MAP_SIZE
                    || map.getHeight(xdx, ydy) - map.getHeight(x, y) > 1 || map.getHeight(xdx, ydy) - map.getHeight(x, y) < -1
                    || closed_nodes_map[xdx][ydy] == 1)) {
                Node m0(n0);
                m0.x = xdx;
                m0.y = ydy;
                m0.nextLevel();
                m0.updatePriority(dest.x, dest.y);
                if(open_nodes_map[xdx][ydy] == 0) {
                    open_nodes_map[xdx][ydy] = m0.priority;
                    pq[pqi].push(m0);
                    dir_map[xdx][ydy] = (i + 2) % 4;
                }
                else if(open_nodes_map[xdx][ydy] > m0.priority) {
                    open_nodes_map[xdx][ydy] = m0.priority;
                    dir_map[xdx][ydy] = (i + 2) % 4;
                    while(!(pq[pqi].top().x == xdx && pq[pqi].top().y == ydy)) {
                        pq[1 - pqi].push(pq[pqi].top());
                        pq[pqi].pop();
                    }
                    pq[pqi].pop();
                    if(pq[pqi].size() > pq[1 - pqi].size())
                        pqi=1 - pqi;
                    while(!pq[pqi].empty()) {
                        pq[1 - pqi].push(pq[pqi].top());
                        pq[pqi].pop();
                    }
                    pqi=1 - pqi;
                    pq[pqi].push(m0);
                }
            }
        }
    }
}

int main()
{
    PMap map("map.txt");

    srand(time(NULL));

    int xA, yA, xB, yB;
    vector<Pos> path;

    do {
        xA = rand() % MAP_SIZE;
        xB = rand() % MAP_SIZE;
    } while (map[xA][xB] < 15);
    do {
        yA = rand() % MAP_SIZE;
        yB = rand() % MAP_SIZE;
    } while (map[yA][yB] < 15);
    //xA = 6; yA = 0;

    // get the route
    clock_t start = clock();
    findPath(map, Pos(xA, yA), Pos(xB, yB), vector<Pos>(), path);
    clock_t end = clock();

    // follow the route on the map and display it
    int displayedMap[MAP_SIZE][MAP_SIZE] = {0};
    for(size_t i=0;i<path.size();i++)
        displayedMap[path[i].x][path[i].y]=1;

    // display the map with the route
    for(int y=0;y<MAP_SIZE;y++)
    {
        for(int x=0;x<MAP_SIZE;x++) {
            if (x == xA && y == yA)
                cout << "W";
            else if (x == xB && y == yB)
                cout << "W";
            else if(displayedMap[x][y]==1)
                cout<<"M";
            else if(map[x][y] > 20)
                cout << "z";
            else
                cout<<( char)(map[x][y] + 'a'); //finish
        }
        cout<<endl;
    }

    cout<<"Start: "<<xA<<","<<yA<<endl;
    cout<<"Finish: "<<xB<<","<<yB<<endl;
    if(path.empty()) cout<<"An empty route generated!"<<endl;
    double time_elapsed = double(end - start);
    cout<<"Time to calculate the route (us): "<<time_elapsed<<endl;
    return(0);
}

