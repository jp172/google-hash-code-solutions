/*
  This code solves the Google Hash Code 2015 task "Optimize a data center".
  It does so by greedily assigning servers to certain positions and after-
  wards optimizing a mixed-integer program for the server-pool assignment.
  Since this is a max-min problem, a tau-variable is introduced to transform
  it to a maximization problem. Expect solutions for dc to lie around 390.

  Usage:  - make
          - ./solve <instance name>
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include "gurobi_c++.h"

using namespace std;

typedef pair<int, int> pi;
typedef vector<int> vi;

class server{
public:
  pi position; // position of server (row r, column c)
  int size; // size ( = length of a server)
  int cap; // server capacity (higher is better)
  int id; // object id
  int pool_id; // pool that it is assigned to
  bool allocated = false; // if this server is chosen to be used
};

// compares size per capacity for each server
bool sizecmp(server &a, server &b){
  return double(double(a.size)/double(a.cap)) < double(double(b.size)/double(b.cap));
}

// compares server ids
bool idcmp(server &a, server &b){
  return a.id < b.id;
}

class instance{
public:

  // instance variables
  int R, C, U, P, M;
  vector<vector<bool> > unavailable; // true if coordinate (r,c) is unavailable
                                     // or already blocked by some server
  vector<server> servers; // all servers

  // reads instance
  void read(){
    cin >> R >> C >> U >> P >> M;
    unavailable.resize(R);
    for (int r = 0; r < R; r++) unavailable[r].resize(C);
    int r, c;
    for (int i = 0; i < U; i++){
      cin >> r >> c; unavailable[r][c] = true;
    }
    servers.resize(M);
    for (int i = 0; i < M; i++){
      servers[i].id = i;
      cin >> servers[i].size >> servers[i].cap;
    }
  }

  // writes solution
  void write(){
    for (server &s: servers){
      if (!s.allocated) cout << "x" << endl;
      else cout << s.position.first << " " << s.position.second << " " << s.pool_id << endl;
    }
  }

  // models the integer program to assign each allocated server to a pool
  void ip_pool_assignment(){
    GRBEnv env;
    GRBModel m = GRBModel(env);
    m.getEnv().set("OutputFlag", "1");
    m.getEnv().set("TimeLimit", "600.0");

    vi allocated; // index set of all servers that are allocated on the grid
    for (int i = 0; i < M; i++) if (servers[i].allocated) allocated.push_back(i);

    cout << "Number servers allocated: " << allocated.size() << endl;

    vector<vector<GRBVar> > x(M); // x[i][p] == 1 iff server i is in pool p
    for (int i: allocated) x[i].resize(P);
    for (int i: allocated) for (int j = 0; j < P; j++) x[i][j] = m.addVar(0,1,1,GRB_BINARY);
    vector<GRBVar> y(P); // models maximal capacity loss for pool p
    for (int i = 0; i < P; i++) y[i] = m.addVar(0,GRB_INFINITY, 1, GRB_CONTINUOUS);
    m.update();

    // assignment constraits (allocate every server to exactly one pool)
    for (int i: allocated) {
      GRBLinExpr expr = 0;
      for (int p = 0; p < P; p++) expr += x[i][p];
      m.addConstr(expr == 1);
    }

    // determines y-variables (modeling the worst case for each pool)
    for (int r = 0; r < R; r++){
      for (int p = 0; p < P; p++){
        GRBLinExpr expr = 0;
        for (int i: allocated) {
          if (servers[i].position.first != r) continue;
          expr += x[i][p] * servers[i].cap;
        }
        m.addConstr(y[p] >= expr);
      }
    }

    // tau needs to be smaller equal the worst performing pool..
    GRBVar tau = m.addVar(0, GRB_INFINITY, 1, GRB_CONTINUOUS);
    for (int p = 0; p < P; p++){
      GRBLinExpr obj = 0;
      for (int i: allocated) {
        obj += servers[i].cap * x[i][p];
      }
      m.addConstr(tau <= obj - y[p]);
    }

    // .. and then tau is maximized
    m.setObjective(tau * 1.0, GRB_MAXIMIZE);

    // some (inexact) cuts, read: "do not put too many servers of one row into one pool"
    int max_server_per_row_per_pool = 1;
    for (int r = 0; r < R; r++){
      for (int p = 0; p < P; p++){
        GRBLinExpr expr = 0;
        for (int i: allocated) {
          if (servers[i].position.first != r) continue;
          else expr += x[i][p];
        }
        m.addConstr(expr <= max_server_per_row_per_pool);
      }
    }

    m.optimize();

    // retrieve assignment
    for (int i: allocated) {
      for (int p = 0; p < P; p++){
        if (x[i][p].get(GRB_DoubleAttr_X) > 0.5){
          servers[i].pool_id = p;
        }
      }
    }
  }

  // assigns server to position
  void assign_server_to_position(server &s, int r, int c){
    for (int pos = 0; pos < s.size; pos++) unavailable[r][c + pos] = true;
    s.position = {r, c};
    s.allocated = true;
  }

  // checks if server s can be placed at row (r,c) to (r, c + s.size - 1)
  bool check(server &s, int r, int c){
    for (int pos = 0; pos < s.size; pos++) if (unavailable[r][c + pos]) return false;
    if (c == 0) return true;
    else if (unavailable[r][c - 1]) return true;
    return false;
  }

  // checks for a server which position it can be assigned to and then assigns to it.
  void assign_server(server &s){
    for (int r = 0; r < R; r++){
      for (int c = 0; c < C - s.size; c++){
        if (check(s, r, c)){
          assign_server_to_position(s, r, c);
          return;
        }
      }
    }
  }

  void assign(){
    // greedy assignment. sort servers by size
    sort(servers.begin(), servers.end(), sizecmp);
    for (server &s: servers) assign_server(s);
    // sort servers back to their id
    sort(servers.begin(), servers.end(), idcmp);
  }

  void solve(){
    assign();
    ip_pool_assignment();
  }

};

int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);

  if (argc < 2){
    cout << "Usage: ./solve <instance name, e.g. dc>" << endl;
    return 0;
  }

  // file input
  string filename;
  filename = argv[1];
  string input_file = "../data/" + filename + ".in";
  freopen(input_file.c_str(), "r", stdin);

  instance I;
  I.read();
  I.solve();

  // file output
  string output_file = "../out/" + filename + ".out";
  freopen(output_file.c_str(), "w", stdout);
  I.write();
}
