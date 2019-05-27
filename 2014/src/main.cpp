/*
  Solution algorithm for Google Hash Code 2014 - Street View routing
  The solution works simulation-based by assigning each car upon availability
  (by storing the cars in a priority queue sorted by time of availability) to
  their next edges to drive along. These next edges are determined by a
  branch-and-bound algorithm, which is bounded by capping the height of the
  branch-and-bound tree. Hence, for an available car the procedure "bfs"
  (approximately) calculates a path from the car's current node with highest
  length/time_required ratio (respecting edges that have been covered as such).
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

using namespace std;

typedef pair<int, int> pi;
typedef vector<int> vi;

class car{
public:
  int id; // car id
  vi moves = {}; // stores car movements as set of nodes
  int time = 0; // time car is available again
};

class edge{
public:
  pi node_ids; // stores ids of nodes this edge connects
  int one_directional; // = 2 if street is bidirectional
  int length, time; // length and time required to drive this edge
  bool covered; // true if edge is covered by some car
  int id; // edge id
};

class instance{
public:
  vector<car> cars; // stores all car objects
  vector<edge> edges; // stores all edge objects
  map<pi, int> edge_id; // maps node ids to edge id it connects
  vector<vector<int> > neighbors; // stores all neighbor nodes of a node id
  vector<pair<double, double> > cities; // stores all cities' coordinates (not really used)
  int N, M, T, C, S; // number of nodes, streets, time limit, number of cars and start ID of all cars

  // read file accordding to problem definition
  void read(){
    cin >> N >> M >> T >> C >> S;
    cars.resize(C);
    for (int i = 0; i < C; i++){ cars[i].id = i; cars[i].moves = {4516};}
    cities.resize(N);
    for (int i = 0; i < N; i++) cin >> cities[i].first >> cities[i].second;
    edges.resize(M);
    neighbors.resize(N);
    for (int i = 0; i < M; i++){
      edges[i].id = i;
      cin >> edges[i].node_ids.first >> edges[i].node_ids.second >> edges[i].one_directional >> edges[i].time >> edges[i].length;
      neighbors[edges[i].node_ids.first].push_back(edges[i].node_ids.second);
      if (edges[i].one_directional == 2){
        neighbors[edges[i].node_ids.second].push_back(edges[i].node_ids.first);
        edge_id[{edges[i].node_ids.second, edges[i].node_ids.first}] = i; // maps (a, b) and (b,a) to the same edge. For bi-directional roads
      }
      edge_id[edges[i].node_ids] = i;
    }
  }

  // write out answer in specified format
  void write(){
    cout << C << endl;
    for (int i = 0; i < C; i++){
      cout << cars[i].moves.size() << endl;
      for (int m: cars[i].moves) cout << m << endl;
    }
  }

  // bfs for the next path for car c
  // returns a path of integers corresponding to the cities it drives along
  vi bfs(car &c){
    queue<tuple<vi, double, double> > path_candidates; // queue of all still considered paths
    double max_value = 0; // current maximal value (length per time)
    vi return_path = {*c.moves.rbegin()}; // chosen path, starts with the car's current position
    path_candidates.push({return_path, 0, 0});

    // start bfs
    while (!path_candidates.empty()){
      // retrieve current node
      tuple<vi, double, double> p = path_candidates.front(); path_candidates.pop();
      vi current_path = get<0>(p);
      double path_length = get<1>(p);
      double path_time = get<2>(p);

      if (current_path.size() >= 20) continue; // bounds the length of all paths
      int cur_position = *current_path.rbegin();

      // iterate among all neighbors
      for (int v: neighbors[cur_position]){
        if (find(current_path.begin(), current_path.end(), v) != current_path.end()) continue;
        edge &cur_edge = edges[edge_id[{cur_position, v}]];
        if (c.time + path_time + cur_edge.time > T) continue;
        current_path.push_back(v);

        // distinction in value calculation if cur_edge was already covered or not
        if (!cur_edge.covered){
          double cur_value = double(path_length + cur_edge.length)/(path_time + cur_edge.time);
          if (cur_value > max_value){
            max_value = cur_value;
            return_path = current_path;
            path_candidates.push({current_path, path_length + cur_edge.length, path_time + cur_edge.time});
          }
          // consider only paths with reasonable high value
          else if (cur_value > 0.5 * max_value) path_candidates.push({current_path, path_length + cur_edge.length, path_time + cur_edge.time});
        }
        else path_candidates.push({current_path, path_length, path_time + cur_edge.time});
        current_path.pop_back();
      }
    }
    return return_path;
  }

  // assigns the next edges to the car with id i
  bool assign_next_edges(car &c){
    vi new_path;
    new_path = bfs(c);

    // checks if bfs returns a new path
    if (new_path.size() == 1) return false;

    // assign new path to car's movements
    int cur, next;
    for (int j = 1; j < int(new_path.size()); j++){
      cur = new_path[j-1];
      next = new_path[j];
      c.moves.push_back(next);
      c.time += edges[edge_id[{cur, next}]].time;
      edges[edge_id[{cur, next}]].covered = true;
    }
    return true;
  }

  void solve(){
    set<pi> queue;
    for (int i = 0; i < C; i++) queue.insert({0, i});
    // assign each car to a new road once it finishes its assigned road
    while (!queue.empty()){
      pi cur = *queue.begin(); queue.erase(*queue.begin());
      // cout << cur.first << " " << cur.second << endl;
      if (cars[cur.second].time > T) continue;

      // only insert into queue if new edges are added
      if (assign_next_edges(cars[cur.second])) queue.insert({cars[cur.second].time, cur.second});
    }
  }
};


instance I;
int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);

  if (argc < 2){
    cout << "Usage: ./solve <instance name, e.g. paris_54000>" << endl;
    return 0;
  }

  // file input
  string filename = argv[1];
  string input_file = "../data/" + filename + ".in";
  freopen(input_file.c_str(), "r", stdin);

  // solve problem
  I.read();
  cout << "instance read in. start solving problem" << endl;
  I.solve();
  cout << "assignment done! writing output!" << endl;
  // file output
  string output_file = "../out/" + filename + ".out";
  freopen(output_file.c_str(), "w", stdout);
  I.write();
}
