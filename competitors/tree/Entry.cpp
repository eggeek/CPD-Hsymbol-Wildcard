/**
 * Copyright (c) 2012, Ken Anderson
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "Entry.h"
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string>
using namespace std;

//////////////// Optional Compiler Defines /////////////////

// Pass in these optional definitions to the compiler, if wanted.
// DEBUG                      -- prints debug data and outputs relevant info to a file.
// TEST_FOR_PATH_CONNECTIVITY -- checks paths for connectivity.  Leaving this option out may improve speed a miniscule amount, but is not safe.
// DISABLE_DIAGONALS          -- Disable diagonal moves.  Useful for debugging.

//////////////// Defines  /////////////////

#ifdef DEBUG
#define PRINT(format,...) printf(format,__VA_ARGS__)
#else
#define PRINT(format,...) ;
#endif

//////////////// Constants /////////////////

#define NO_OP      0x00
#define	UP         0x01
#define DOWN       0x02
#define LEFT       0x03
#define RIGHT      0x04
#define UP_LEFT    0x05
#define UP_RIGHT   0x06
#define DOWN_LEFT  0x07
#define DOWN_RIGHT 0x08

const float SQRT2 = sqrt(2.0);

//////////////// Structures /////////////////

typedef char Operation;

//////////////// Variables /////////////////
int width, height;

// Array structures used when creating the cached search tree.
// Only the last two arrays are actually saved/loaded from disk.
// The size of the file will be width*height*2.
// In the worst case scenario (2048*2048*2), we'll be under 10 Megs.
std::vector<bool> map;              // Stores the location of the walls
std::vector<bool> visited;          // Stores whether we visited this node while making the search tree
std::vector<float> costF;           // Accurate costs for initial calculation
std::vector<unsigned char> costI;   // Compacted costs for storage.
std::vector<Operation> opToParent;  // Store only the operator because it takes less space.

//////////////// Definitions /////////////////

inline int GetIndex(xyLoc s)
{
	return s.y*width+s.x;
}

template <typename T>
inline void free_memory(std::vector<T>& to_clear)
{
    std::vector<T> v;
    v.swap(to_clear);
}

xyLoc GetNeighbour( xyLoc loc, Operation op)
{
	switch(op)
	{
		case UP:
			loc.y++;
			break;
		case DOWN:
			loc.y--;
			break;
		case LEFT:
			loc.x--;
			break;
		case RIGHT:
			loc.x++;
			break;
		case UP_LEFT:
			loc.x--;
			loc.y++;
			break;
		case UP_RIGHT:
			loc.x++;
			loc.y++;
			break;
		case DOWN_LEFT:
			loc.x--;
			loc.y--;
			break;
		case DOWN_RIGHT:
			loc.x++;
			loc.y--;
			break;
	}
	return loc;
}

Operation reverse(Operation op)
{
	switch(op)
	{
		case UP:
			op = DOWN;
			break;
		case DOWN:
			op = UP;
			break;
		case LEFT:
			op = RIGHT;
			break;
		case RIGHT:
			op = LEFT;
			break;
		case UP_LEFT:
			op = DOWN_RIGHT;
			break;
		case UP_RIGHT:
			op = DOWN_LEFT;
			break;
		case DOWN_LEFT:
			op = UP_RIGHT;
			break;
		case DOWN_RIGHT:
			op = UP_LEFT;
			break;
	}
	return op;
}

float GetCost( Operation op)
{
	switch(op)
	{
		case UP:
		case DOWN:
		case LEFT:
		case RIGHT:
			return 1.0;
		default:
			return SQRT2;
	}
}

// Debug check
void verify()
{
/*	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			xyLoc loc;
			loc.x = x;
			loc.y = y;
			int index = GetIndex(loc);
			tree_entry &entry = tree[index];
			if(entry.visited==true && 
				( entry.parent.x<0 || entry.parent.y<0 || entry.parent.x>=width || entry.parent.y >= height ) 
			)
			{
				PRINT("error");
			}
		}
	}
*/
}

void WriteTree( const char * filename, const std::vector<xyLoc>* path = NULL )
{
	// Writes a Human-readible file for debugging
	FILE * fp = fopen(filename, "w");
	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			char c = '@';
			xyLoc loc;
			loc.x = x;
			loc.y = y;
			int index = GetIndex(loc);

			if( opToParent[index] != NO_OP )
			{
				xyLoc parent = GetNeighbour(loc,opToParent[index]);
				if( parent.x==loc.x && (parent.y==loc.y+1 || parent.y==loc.y-1) )
				{
					c = '|';
				}
				else if( (parent.x==loc.x+1 || parent.x==loc.x-1) && parent.y==loc.y )
				{
					c = '-';
				}
				else if( (parent.x==loc.x+1 && parent.y==loc.y+1) || (parent.x==loc.x-1 && parent.y==loc.y-1) )
				{
					c = '\\';
				}
				else if( (parent.x==loc.x+1 && parent.y==loc.y-1) || (parent.x==loc.x-1 && parent.y==loc.y+1) )
				{
					c = '/';
				}
				else
				{
					c = '.';
				}
			}
			// Check if in solution (inefficient, but this is just for debugging)
			if( path )
			{
				for( unsigned int i=0; i<path->size(); i++)
				{
					if( (*path)[i].x==loc.x && (*path)[i].y==loc.y )
					{
						c = '*';
					}
				}
			}

			fprintf(fp, "%c", c);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}


// generates 8-connected neighbors
// a diagonal move must have both cardinal neighbors free to be legal
void GetSuccessors(xyLoc s, std::vector<Operation> &ops)
{
	ops.resize(0);
	int opFlags = 0;
	xyLoc next = s;

	next.x++;
	if (next.x < width && map[GetIndex(next)])
	{
		ops.push_back(RIGHT);
		opFlags |= 0x01<<RIGHT;
	}

	next = s;
	next.x--;
	if (next.x >= 0 && map[GetIndex(next)])
	{
		ops.push_back(LEFT);
		opFlags |= 0x01<<LEFT;
	}

	next = s;
	next.y--;
	if (next.y >= 0 && map[GetIndex(next)])
	{
		ops.push_back(DOWN);
		opFlags |= 0x01<<DOWN;
	}

	next = s;
	next.y++;
	if (next.y < height && map[GetIndex(next)])
	{
		ops.push_back(UP);
		opFlags |= 0x01<<UP;
	}
#ifndef DISABLE_DIAGONALS
	next = s;
	next.y++;
	next.x++;
	if((opFlags&(0x01<<UP)) && (opFlags&(0x01<<RIGHT)) && map[GetIndex(next)])
	{
		ops.push_back(UP_RIGHT);
	}
	
	next = s;
	next.y++;
	next.x--;
	if((opFlags&(0x01<<UP)) && (opFlags&(0x01<<LEFT)) && map[GetIndex(next)])
	{
		ops.push_back(UP_LEFT);
	}
	
	next = s;
	next.y--;
	next.x++;
	if((opFlags&(0x01<<DOWN)) && (opFlags&(0x01<<RIGHT)) && map[GetIndex(next)])
	{
		ops.push_back(DOWN_RIGHT);
	}
	
	next = s;
	next.y--;
	next.x--;
	if((opFlags&(0x01<<DOWN)) && (opFlags&(0x01<<LEFT)) && map[GetIndex(next)] )
	{
			ops.push_back(DOWN_LEFT);
	}
#endif
}

// Basicaly does a Djikstra's search to create a search tree
// anchored at the given location.
void CreateTree(xyLoc loc)
{
	PRINT("CreateTree(%i %i)\n",loc.x,loc.y);
	std::vector<Operation> ops;
	std::deque<xyLoc> q;

	int index = GetIndex(loc);
	q.push_back(loc);
	visited[index] = true;
	opToParent[index] = NO_OP;
	costF[index] = 0;

	while( q.size() > 0 )
	{
		loc = q.front();
		q.pop_front();
		index = GetIndex(loc);

		GetSuccessors(loc, ops);
		for( unsigned int i=0; i<ops.size(); i++ )
		{
			Operation op = ops[i];
			xyLoc successor = GetNeighbour(loc,op);
			float edge_cost = GetCost(op);
			float succ_cost = costF[index] + edge_cost;
			int index2 = GetIndex(successor);

			if( succ_cost < costF[index2] )
			{
				opToParent[index2] = reverse(op);
				costF[index2] = succ_cost;
				if( visited[index2] == false )
				{
					visited[index2] = true;
					q.push_back(successor);
				}
			}
		}
	}
}

/////////////// Public functions //////////////////

const char *GetName()
{
	return "TreeCache";
}

string getMapName(string filename) {
  auto pos = filename.find_last_of('/');
  const string mapfile = filename.substr(pos + 1);
  auto suff = mapfile.find('.');
  return mapfile.substr(0, suff);
}


void PreprocessMap(std::vector<bool> &bits, int w, int h, const char *filename)
{
	map = bits;
	width = w;
	height = h;

	PRINT("PreprocessMap: width=%i height=%i \n", width, height);

	// init
	unsigned int size = bits.size();
	opToParent.resize(size);
	visited.resize(size);
	costF.resize(size);
	costI.resize(size);
	for( unsigned int i=0; i<size; i++ )
	{
		opToParent[i] = NO_OP;
		visited[i]    = false;
		costF[i]      = FLT_MAX;
		costI[i]      = (unsigned char)255;
	}

	// Create Trees (multiple roots possible)
	for( int x=0; x<width; x++) {
		for( int y=0; y<height; y++) {
			xyLoc loc = {(int16_t)x,(int16_t)y};
			int index = GetIndex(loc);
			if( map[index]==true && visited[index]==false )
			{
				CreateTree(loc);
			}
		}
	}		

	// Compact
	for( unsigned int i=0; i<size; i++ )
	{
		// Round up to ensure that only the root node gets a cost of 0.
		const double scaledCostF = std::min( 254.0, ceil(costF[i]/5.0));
		const unsigned int scaledCostI = scaledCostF;
		costI[i] = (unsigned char) scaledCostI;
	}

	// Write file
	PRINT("PreprocessMap: Writing to file '%s'\n", filename);
	FILE* fp = fopen(filename, "wb");
	fwrite(&costI[0], sizeof(unsigned char), costI.size(), fp);
	fwrite(&opToParent[0], sizeof(Operation), opToParent.size(), fp);
	fclose(fp);

#ifdef DEBUG
	verify();
	WriteTree("debug_map.txt");
#endif

	map.clear();
	costI.clear();
	visited.clear();

	// Force memory freeing
	free_memory(map);
	free_memory(costI);
	free_memory(visited);
	free_memory(costI);
	free_memory(opToParent);
}

void *PrepareForSearch(std::vector<bool> &bits, int w, int h, const char *filename)
{
	map = bits;
	width = w;
	height = h;

	int size = w*h;
	costI.resize(size);
	opToParent.resize(size);

	PRINT("PrepareForSearch: Reading from file '%s'\n", filename);
	FILE* fp = fopen(filename, "rb");
	fread(&costI[0], sizeof(unsigned char), costI.size(), fp);
	fread(&opToParent[0], sizeof(Operation), opToParent.size(), fp);
	fclose(fp);

#ifdef DEBUG
	verify();
#endif

	return NULL;//(void *)13182;
}

void PrintPath(std::vector<xyLoc> &path)
{
	PRINT("Len=%u \n", (unsigned int)path.size());
	for(unsigned int i=0; i<path.size(); i++)
	{
		PRINT("%i,%i \n", path[i].x, path[i].y );
	}
	PRINT("%c\n", ' ');
}

bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path)
{
	PRINT("PrepareForSearch: %c\n", ' ');

	//assert((long)data == 13182);
	std::deque<xyLoc> q;
	xyLoc loc, loc2;
	int index, index2;

#ifdef DEBUG
	verify();
#endif

	// Get both halves of path
	//path.clear();
	//q.clear();
	loc = s;
	loc2 = g;
	index = GetIndex(loc);
	index2 = GetIndex(loc2);
	path.push_back(loc);
	q.push_back(loc2);
	while( loc.x != loc2.x || loc.y != loc2.y )
	{
		if( costI[index] > costI[index2] )
		{
#ifdef TEST_FOR_PATH_CONNECTIVITY
			if( opToParent[index] == NO_OP )
			{
				path.clear();
				return true; // Two points not connected
			}
#endif
			loc = GetNeighbour(loc, opToParent[index]);
			index = GetIndex(loc);
			path.push_back(loc);
		}
		else
		{
#ifdef TEST_FOR_PATH_CONNECTIVITY
			if( opToParent[index2] == NO_OP )
			{
				path.clear();
				return true; // Two points not connected
			}
#endif
			loc2 = GetNeighbour(loc2, opToParent[index2]);
			index2 = GetIndex(loc2);
			q.push_back(loc2);
		}
	}

	// Skip the root of the tree (don't want to include it twice)
	path.pop_back();	

	// Reverse order and add to solution
	std::deque<xyLoc>::reverse_iterator it;
	for( it = q.rbegin(); it != q.rend(); ++it )
	{
		path.push_back(*it);
	}
	
#ifdef DEBUG
	PrintPath(path);
	fflush(stdout);
	WriteTree("debug_map_path.txt", &path);
#endif

	return true;
}
