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

// custom comparator for priority queue
struct mycmp {
    bool operator() (const tuple<double, ll, ll>& lhs, const tuple<double, ll, ll>& rhs) const {
        return lhs > rhs;
    }
};


struct video{
  ll size; // video size
  vi requests; // ids of all requests requesting this video
};

struct endpoint{
  vi cache_time; // latency to cache server. if not connected, big M latency assumed
  ll latency_to_data_center; // latency to data center
};

// cache server object
struct cache{
  ll id;
  ll capacity; // size of all available videos needs to be less or equal this
  unordered_set<ll> available_videos; // ids of all videos put to this server
};

struct request{
  ll endpoint_id, video_id, number_requests, current_latency;
};


struct instance{
  ll V, E, R, C, X; // from problem description

  // stores the respective objects
  vector<video> videos;
  vector<endpoint> endpoints;
  vector<request> requests;
  vector<cache> caches;

  // read instance
  void read(){
    cin >> V >> E >> R >> C >> X;
    videos.resize(V);
    rep(i, 0, V) cin >> videos[i].size;
    caches.resize(C);
    rep(i, 0, C){
      caches[i].id = i;
      caches[i].capacity = X;
    }
    endpoints.resize(E);
    ll nbr_connected_cache_servers, cache_id;
    trav(e, endpoints){
      e.cache_time.resize(C, 10000); // if no connection from e to c, use big M
      cin >> e.latency_to_data_center >> nbr_connected_cache_servers;
      rep(j, 0, nbr_connected_cache_servers){
        cin >> cache_id;
        cin >> e.cache_time[cache_id];
      }
    }
    requests.resize(R);
    rep(i, 0, R){
      auto &r = requests[i];
      cin >> r.video_id >> r.endpoint_id >> r.number_requests;
      r.current_latency = endpoints[r.endpoint_id].latency_to_data_center;
      videos[r.video_id].requests.pb(i);
    }
  }

  // write instance
  void write(){
    cout << C << endl;
    trav(c, caches){
      cout << c.id << " ";
      trav(video_id, c.available_videos) cout << video_id << " ";
      cout << endl;
    }
  }

  // evaluate the solution.
  void eval(){
    // initialize all request latencies with their latency to the data center
    trav(r, requests) r.current_latency = endpoints[r.endpoint_id].latency_to_data_center;
    // update all request latencies according to video availability on cache servers
    trav(c, caches){
      ll capacity = 0;
      trav(video_id, c.available_videos) capacity += videos[video_id].size;
      if (capacity > X) {
        cout << "Too much space used on cache server " << c.id << endl;
        return;
      }
      trav(r, requests){
          if (c.available_videos.find(r.video_id) == c.available_videos.end()) continue;
          r.current_latency = min(r.current_latency, endpoints[r.endpoint_id].cache_time[c.id]);
      }
    }
    // sum all latency improvements
    ll objval = 0;
    trav(r, requests) objval += r.number_requests * (endpoints[r.endpoint_id].latency_to_data_center - r.current_latency);
    // sum all request numbers
    ll sum_of_all_request_numbers = 0;
    trav(r, requests) sum_of_all_request_numbers += r.number_requests;
    cout << int((1000.0)* double(objval)/double(sum_of_all_request_numbers)) << endl;
  }

  // assign video to cache server
  void assign(ll video_id, ll cache_id){ // video cache
    video &v = videos[video_id];
    cache &c = caches[cache_id];

    // update cache server
    c.available_videos.insert(video_id);
    c.capacity -= v.size;

    // update reduced latency for all affected requests
    trav(request_id, v.requests){
      request &r = requests[request_id];
      r.current_latency = min(r.current_latency, endpoints[r.endpoint_id].cache_time[cache_id]);
    }
  }

  // checks if video fits on cache server
  bool feas(ll video_id, ll cache_id){
    return (caches[cache_id].capacity >= videos[video_id].size);
  }

  // estimates benefit of adding video video_id to cache server cache_id
  // benefit is measured in (estimated added) objective value divided by video size
  double get_value(ll video_id, ll cache_id){
    // if video does not fit or is already on server, return 0
    if (!feas(video_id, cache_id) || caches[cache_id].available_videos.find(video_id) != caches[cache_id].available_videos.end()) return 0.0;
    ll objective_value_increase = 0;
    trav(rid, videos[video_id].requests){
      request &r = requests[rid];
      objective_value_increase += ll(r.number_requests) * max(0LL, ll(r.current_latency - endpoints[r.endpoint_id].cache_time[cache_id]));
    }
    return double(objective_value_increase) / double(videos[video_id].size);
  }

  // solves problem by greedy video to cache server assignment
  // all possible video->server assignments are stored in a priority queue.
  // they are prioritised decreasingly by benefit divided by video size.
  // after a video is added the score of all other items is updated lazily
  void solve(){
    set<tuple<double, ll, ll > , mycmp > video_queue;
    rep(v, 0, V) rep(c, 0, C) video_queue.insert({get_value(v,c), v, c});

    double old_value, new_value;
    ll video_id, cache_id;
    while (!video_queue.empty()){
      tie(old_value, video_id, cache_id) = *video_queue.begin();
      video_queue.erase(video_queue.begin());
      if (old_value < 1e-9) break;
      if (!feas(video_id, cache_id)) continue;
      // lazy update
      new_value = get_value(video_id, cache_id);
      // if the value was (approximately) right, assign it
      if (abs(new_value - old_value) < 1e-9){
        assign(video_id, cache_id);
      }
      else if (new_value > 1e-9) video_queue.insert({new_value, video_id, cache_id});
    }
  }
};

instance I;
int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);
  string input_file = argv[1];
  string output_file = "out";
  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  I.solve();
  I.eval();
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();
}

// trending today: 499966
// me_at_the_zoo:  507906
// videos worth spreading: 608303
// kittens: 1021680
