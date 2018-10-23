#pragma once
#include "mapper.h"
using namespace std;

class Visualizer {
  public:
    const vector<bool>& bits;
    const Mapper& mapper;
    Visualizer(const vector<bool>& mapData, const Mapper& mapper): bits(mapData), mapper(mapper) {};

    vector<string> to_strings() {
      vector<string> res;
      for (int y=0; y<mapper.height(); y++) {
        string row = "";
        for (int x=0; x<mapper.width(); x++) {
          int idx = y * mapper.width() + x;
          if (bits[idx]) row.push_back('.');
          else row.push_back('T');
        }
        res.push_back(row);
      }
      return res;
    }

    vector<string> to_strings(int source, const vector<unsigned short>& allowed) {
      vector<string> res = to_strings();
      for (int i=0; i<(int)allowed.size(); i++) {
        xyLoc cur = mapper(i);
        if (i == source) {
          res[cur.y][cur.x] = 'S';
        } else if (allowed[i] & warthog::HMASK) {
          res[cur.y][cur.x] = 'H';
        } 
      }
      return res;
    }

    static uint32_t str2tiles(const vector<string>& map) {
      uint32_t tiles = 0;
      for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) if (map[i][j] != '#') {
          tiles |= (1 << j) << (i * 8);
        }
      }
      return tiles;
    }

    static vector<string> tiles2str(uint32_t tiles) {
      vector<string> map = {
        "...",
        ".x.",
        "..."
      };
      for (int i=0; i<3; i++, tiles >>= 8) {
        for (int j=0; j<3; j++) {
          if (tiles & (1 << j)) map[i][j] = '.';
          else map[i][j] = '#';
        }
      }
      map[1][1] = 'x';
      return map;
    }

    static void set2direct(uint32_t moveset) {
      if (moveset & warthog::jps::NORTH) cout << "north ";
      if (moveset & warthog::jps::SOUTH) cout << "south ";
      if (moveset & warthog::jps::EAST) cout << "east ";
      if (moveset & warthog::jps::WEST) cout << "west ";
      if (moveset & warthog::jps::NORTHEAST) cout << "northeast ";
      if (moveset & warthog::jps::NORTHWEST) cout << "northwest ";
      if (moveset & warthog::jps::SOUTHEAST) cout << "southeast ";
      if (moveset & warthog::jps::SOUTHWEST) cout << "southwest ";
      cout << endl;
    }
};
