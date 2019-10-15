#include <vector>
#include <iomanip>
#include <sstream>
#include "dijkstra.h"
#include "jps.h"
#include "order.h"
#include "loader.h"
#include "centroid.h"
#include "cpd_base.h"
#include "cpd_rect.h"
using namespace std;

vector<bool> mapData;
const string header = "x,y,type,mask";
int height, width;

void print_obstacle(const Mapper& mapper, vector<vector<bool>>& flag, ofstream& output) {
  for (int y=0; y<height; y++)
  for (int x=0; x<width; x++) {
    if (flag[y][x]) continue;
    if (mapper(xyLoc{(int16_t)x, (int16_t)y}) != -1) continue;
    flag[y][x] = true;
    output << x << "," << y << "," << 1 << "," << 0 << endl;
  }
}


void gen_visual() {
  string mpath;
  cin >> mpath;
  ofstream output("data.csv");
  LoadMap(mpath.c_str(), mapData, width, height);
  cout << "mpath: " << mpath << endl;
  cout << "height: " << height << "," << "width: " << width << endl;
  Mapper mapper(mapData, width, height);

  vector<vector<bool>> flag(height+1, vector<bool>(width+1, false));
  output << header << endl;
  print_obstacle(mapper, flag, output);
}

int main() {
  return 0;
}
