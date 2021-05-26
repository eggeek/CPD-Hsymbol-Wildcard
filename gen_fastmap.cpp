#include "loader.h"
#include "FastMapGenerator.h"
#include "list_graph.h"
#include <iomanip>


int main(int argc, char** argv) {
  string mpath = string(argv[1]);
  vector<bool> bits;
  int height, width;
  LoadMap(mpath.c_str(), bits, width, height);
  Mapper mapper(bits, width, height);
  AdjGraph g(extract_graph(mapper));

  FastMapGenerator fgen = FastMapGenerator();
  FastMapHeuristic* fmh = fgen.CalcHeuristic(&g);
  printf("%d %d\n", g.node_count(), fmh->num_coord);
  auto coordinates = fmh->coordinates;
  for (int i=0; i<g.node_count(); i++) {
    for (int j=0; j<fmh->num_coord; j++) {
      printf("%.4f%c", coordinates[i][j], j+1<fmh->num_coord?' ': '\n');
    }
  }
  return 0;
}
