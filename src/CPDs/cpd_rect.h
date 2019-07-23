#pragma once
#include "cpd_base.h"
#include "rect_wildcard.h"
using namespace std;


class CPD_RECT: public CPDBASE {
public:
  vector<RectInfo> append_row(int s, const vector<unsigned short>& allowed, const Mapper& mapper, 
      const vector<RectInfo>& rects, const vector<int>& row_ordering,
      const int side);
  vector<int> compress(int s, const vector<unsigned short>& allowed,
      const RectInfo& rect, const Mapper& mapper, const vector<int>& row_ordering,
      const int side);
};
