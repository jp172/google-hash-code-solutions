/*
  This file contains the source code for Google Hash Codes Pizza Exercise.
  It uses a divide-and-conquer approach. To this end, the whole area R x C is
  divided into smaller areas of size split x split. Then for each small area
  all possible pizza cuts are determined and stored as vertices. For each pair
  of vertices (i.e., pizza cuts) it is determined if they overlap. If so, an
  edge is added between them. For this graph of vertices and edges a mixed-
  integer program via Gurobi is run with the objective of maximizing the area
  of the chosen vertices (pizza cuts). All pizza cuts are then added and the
  next area is considered.

  Usage:  - compile with make
          - "./solve <instance name, e.g., medium> ""
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "gurobi_c++.h"

using namespace std;

typedef pair<int, int> pi;

int split = 12; // size of divide-and-conquer squares. 15 is still okay in speed

class vertex{
  // each possible pizza cut is stored in a vertex
public:
  int r, c; // top left start position of pizza cut
  pair<int, int> shape;
  // pizza cut covers [r, r + shape.first - 1] x [c, c + shape.second - 1]

  vertex(int the_r, int the_c, pi the_shape){
    r = the_r; c = the_c; shape = the_shape;
  }
  int get_cost(){ // cost = size of area
    return shape.first * shape.second;
  }
};


class instance{
public:
    void read(){
      // reads data
      cin >> R >> C >> min_items >> max_cells;
      pizza.resize(R);
      for (int r = 0; r < R; r++) pizza[r].resize(C);
      used = pizza;
      char cr;
      for (int r = 0; r < R; r++) for (int c = 0; c < C; c++){
        cin >> cr;
        if (cr == 'M') pizza[r][c] = 1;
      }
    }

    void write_output(){
      // writes output
      cout << out.size() << endl;
      for (auto o: out){
        for (int i: o) cout << i << " ";
        cout << endl;
      }
    }

    void get_shapes(){
      // determines all possible pizza shapes according to problem definition
      for (int i = 2 * min_items; i <= max_cells; i++){
        for (int j = 1; j <= i; j++){
          if (i % j == 0) shapes.push_back({j, i/j});
        }
      }
    }

    bool overlap(vertex &v1, vertex &v2){
      if (v1.r == v2.r && v1.c == v2.c) return false;
      if (v2.r >= v1.r + v1.shape.first || v2.c >= v1.c + v1.shape.second ) return true;
      if (v1.r >= v2.r + v2.shape.first  || v1.c >= v2.c + v2.shape.second ) return true;
      return false;
    }

    bool check_area(int i, int j, pi &f, int r, int c){
      // checks if some area of the curent candidate has already been used
      if (!(i + f.first <= r && j + f.second <= c)) return false;
      int tcount = 0;
      for (int d1 = i; d1 < i + f.first; d1++){
        for (int d2 = j; d2 < j + f.second; d2++){
          if (used[d1][d2]) return false;
          if (!pizza[d1][d2]) tcount++;
        }
      }
      if ((tcount < min_items) || (f.first * f.second - tcount < min_items)) return false;
      return true;
    }

    void get_vertices(vector<vertex> &vertices, int r, int c){
      // determines all vertices, i.e. all possible pizza cuts in the specified area.
      // hence all (top left) starting positions (r,c) are iterated and all possible shapes are tested
      r = min(r, R); c = min(c, C);
      for (int i = 0; i < r; i++){
        for (int j = 0; j < c; j++){
          if (used[i][j]) continue;
          for (auto f: shapes){
            if (!check_area(i, j, f, r, c)) continue;
            else {
              vertex v = vertex(i, j, f);
              vertices.push_back(v);
            }
          }
        }
      }
    }

    void get_edges(vector<pi> &edges, vector<vertex> &vertices){
      // determines all edges. between the vertices. an edge is added if two pizza cuts (i.e. vertices) overlap
      int n = vertices.size();
      for (int v1 = 0; v1 < n; v1++){
        for (int v2 = 0; v2 < v1; v2++){
          if (!overlap(vertices[v1], vertices[v2])) edges.push_back({v1, v2});
        }
      }
    }

    void max_independent_set(vector<vertex> &vertices, vector<pi> &edges, vector<int> &indices){
      // sets up a mixed integer program to solve maximal independent set problem
      int v = vertices.size();
      int e = edges.size();
      GRBEnv env;
      GRBModel m = GRBModel(env);
      m.getEnv().set("OutputFlag", "0");
      vector<GRBVar> x(v); // x[i] = 1 <=> vertex i is chosen
      for (long long i = 0; i < v; i++) x[i] = m.addVar(0, 1, 1,GRB_BINARY);
      m.update();
      for (int i = 0; i < e; i++){
        m.addConstr(x[edges[i].first] + x[edges[i].second] <= 1);
      }
      m.update();
      GRBLinExpr objective_function = 0;
      for (long long i = 0; i < v; i++){
          objective_function += vertices[i].get_cost() * x[i];
      }
      m.setObjective(objective_function, GRB_MAXIMIZE);
      m.optimize();

      for (long long i = 0; i < v; i++){
          if (x[i].get(GRB_DoubleAttr_X) > 0.5){
            indices.push_back(i);
          }
      }
      // cerr << m.get(GRB_DoubleAttr_ObjVal) << endl;

    }
    void cover_area(vertex &v){
      // this area is already subject to some pizza cut
      for (int i = v.r; i < v.r + v.shape.first; i++){
        for (int j = v.c; j < v.c + v.shape.second; j++){
          used[i][j] = true;
        }
      }
    }

    void solve_area(int r, int c){
      // for each considered area at position (r,c) to (r+split, c+split) we determine pizza cuts
      // first all vertices = possible pizza cuts are determined and then a maximal independend set problem is solved
      vector<vertex> vertices;
      vector<pi> edges;
      get_vertices(vertices, r, c);
      vector<int> indices; // index set of all vertices (pizza cuts) that shall be executed
      get_edges(edges, vertices);
      max_independent_set(vertices, edges, indices);
      for (int i: indices){
        cover_area(vertices[i]);
        vertex &v = vertices[i];
        score += v.get_cost();
        out.push_back({v.r, v.c, v.r + v.shape.first - 1, v.c + v.shape.second - 1});
      }
    }

    // instance variables
    int C, R;
    int min_items, max_cells;
    vector<vector<bool> > pizza; // 0 = tomato, 1 = mushroom
    vector<vector<bool> > used; // area that was already cut

    vector<pair<int, int> > shapes; // all possible shapes
    int score = 0; // current score
    vector<vector<int> > out; // all done cuts stored in output format
};

int main(int argc, char** argv){
  ios::sync_with_stdio(false);
  cin.tie(0);

  // file input
  string filename;
  if (argc < 2) filename = "example";
  else filename = argv[1];
  string input_file = "../data/" + filename + ".in";
  freopen(input_file.c_str(), "r", stdin);

  // preprocessing
  instance I;
  I.read();
  I.get_shapes();

  // solve by divide-and-conquer
  cerr << "size of pizza is: " << I.R << " x " << I.C << endl;
  cerr << "divide-and-conquer size is: " << split << " x " <<  split << endl;
  cerr << "start cutting from top left to bottom right.." << endl;
  for (int r = split; r <= max(split, I.R ); r += split){
    for (int c = split; c <= max(split, I.C); c += split){
      cerr << r << " x " << c << endl;
      I.solve_area(r, c);
    }
    I.solve_area(r, I.C);
  }
  for (int c = split; c <= max(split, I.C); c += split) I.solve_area(I.R, c);
  I.solve_area(I.R, I.C);

  cerr << "finished. score is: " << I.score << endl;
  string output_file = "../out/" + filename;
  freopen(output_file.c_str(), "w", stdout);
  I.write_output();
}
