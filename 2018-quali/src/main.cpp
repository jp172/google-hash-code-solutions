/*
  Algorithm for Streaming Videos (Google Hash Code 2017, Qualification)
  Videos are added to the cache server greedily. The video that has the highest
  ratio of increase in objective value divided by video size is assigned to
  the cache server that yields this highest increase.

  Usage: - make
         - ./solve <input_file>
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
#include <tuple>
#include <functional>
#include <unordered_set>

using namespace std;

#define rep(i, from, to) for (ll i = from; i < ll(to); ++i)
#define trav(a, x) for (auto& a : x)
#define all(x) x.begin(), x.end()
#define sz(x) (ll)(x).size()
#define pb push_back
#define fi first
#define se second

typedef long long ll;
typedef pair<ll, ll> pi;
typedef vector<ll> vi;
typedef vector<vector<ll> > vvi;


struct pt{
  ll r, c;
  pt(){}
  pt(ll r, ll c){
    this->r = r;
    this->c = c;
  }
  ll sum(){
    return (this->r + this-> c);
  }
  pt operator -(const pt &other){
    return pt(r - other.r, c - other.c);
  }
};

ll dis(pt a, pt b){
  return abs(a.c - b.c) + abs(a.r - b.r);
}

struct Ride{
  int id;
  pt start;
  pt finish;
  ll earliest_start, latest_finish;

  bool done = false;
};
struct Vehicle{
  int id, availability;
  pt position = pt(0,0);
  vector<int> rides;

  Vehicle(){}
  Vehicle(int id){
    this->id = id;
  }
};

struct mycmp {
    bool operator() (const Vehicle* lhs, const Vehicle* rhs) const {
        if (lhs->availability == rhs->availability) return lhs->id < rhs->id;
        else return lhs->availability < rhs->availability;
        //return lhs->id < rhs->id;
    }
};

struct instance{
  ll R, C, F, N, B, T; // from problem description
  ll instance_score = 0;
  // stores the respective objects
  vector<Ride> Rides;
  vector<Vehicle> Vehicles;

  // read instance
  void read(){
    cin >> R >> C >> F >> N >> B >> T;
    Vehicles.resize(F);
    rep(i, 0, F) Vehicles[i].id = i;
    Rides.resize(N);
    rep(i, 0, N){
      cin >> Rides[i].start.r >> Rides[i].start.c >>
      Rides[i].finish.r >> Rides[i].finish.c >> Rides[i].earliest_start >> Rides[i].latest_finish;
      Rides[i].id = i;
    }
  }

  void get_upper_bound(){
    ll UB = B * N;
    for (Ride& r: Rides){
      UB += dis(r.start, r.finish);
    }
    cerr << UB << endl;
  }

  double get_score(Vehicle* v, Ride& r){
    double d = dis(v->position, r.start);
    if (max(v->availability + (ll) d, r.earliest_start) + dis(r.start, r.finish) > min(T, r.latest_finish)) return 1e8;
    return d + max(0.0, (double) r.earliest_start - (v->availability + d)) - B * (int) (v->availability + d <= r.earliest_start);
  }

  Ride* get_next_ride(Vehicle* v){
    double best_score = 1e9;
    Ride* best_ride = nullptr;
    for (Ride& r: Rides){
      if (r.done) continue;
      double sc = get_score(v, r);
      if (sc < best_score){
        best_score = sc;
        best_ride = &r;
      }
    }
    return best_ride;
  }

  bool assign(Vehicle* v, Ride* r){
    if (r == nullptr) return false;

    // car avail is end of ride
    ll d = dis(v->position, r->start);
    ll d2 = dis(r->start, r->finish);
    ll end_time = max(r->earliest_start, v->availability + d) + d2;
    if (end_time > min(T, r->latest_finish)) return false;

    // update score
    this->instance_score += (v->availability + d <= r->earliest_start ? B : 0) + d2;

    // car avail
    v->availability = end_time;

    // car goes to endpoint
    v->position = r->finish;

    v->rides.pb(r->id);

    r->done = true;
    return true;
  }

  void solve(int j){
      set<Vehicle*, mycmp> queue;
      vector<Vehicle*> queue2;


      //rep(i, 0, F) queue.insert(&Vehicles[i]);
      rep(i, 0, F) queue2.pb(&Vehicles[i]);

      rep(i, 0, j) random_shuffle(all(queue2));

      while (!queue2.empty()){
          random_shuffle(all(queue2));
          //Vehicle* cur_veh = *queue.begin();
          //queue.erase(queue.begin());
          int cv = rand() % queue2.size();
          Vehicle* cur_veh = queue2[cv];
          Ride* r = get_next_ride(cur_veh);
          if (assign(cur_veh, r) && cur_veh->availability <= T){
            //queue.insert(cur_veh);
          }
          else{
            queue2.erase(queue2.begin() + cv);
          }
      }

  }

  ll get_solution_score(){
    return instance_score;
  }

  void write() {
    rep(i, 0, F){
      cout << Vehicles[i].rides.size();
      for (int i: Vehicles[i].rides) cout << " " << i;
      cout << endl;
    }
  }

  void evaluate(){
    ll real_score = 0;
    for (Vehicle& v: Vehicles){
      ll time = 0;
      pt pos(0,0);
      for (int i : v.rides){
        Ride r = Rides[i];
        time += dis(pos, r.start);
        if (time <= r.earliest_start) real_score += B;
        time = max(time, r.earliest_start);
        time += dis(r.start, r.finish);
        pos = r.finish;
        if (time <= r.latest_finish) real_score += dis(r.start, r.finish);
      }
      if (time > T){
        cerr << "error in veh " << v.id << endl;
        return;
      }
    }
    cerr << "real score: " << real_score << endl;
  }

};

instance I;
int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);
  string input_file = argv[1];

  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  I.get_upper_bound();
  ll best = 0;
  rep(i, 0, 10000){
    for (Vehicle &v: I.Vehicles) {
      v.rides.clear();
      v.position = pt(0,0);
      v.availability = 0;
    }
    for (Ride& r: I.Rides) r.done = false;
    srand(time(NULL));
    I.instance_score = 0;
    I.solve(i);
    if (I.instance_score > best){
      best = I.instance_score;
      cerr << best << endl;
      I.evaluate();
      string output_file = input_file + "out" + to_string(I.instance_score);
      freopen(output_file.c_str(), "w", stdout); // redirects standard output
      I.write();
    }
  }

  ll score = I.get_solution_score();
  I.evaluate();
  string output_file = input_file + "out" + to_string(score);
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();
}
