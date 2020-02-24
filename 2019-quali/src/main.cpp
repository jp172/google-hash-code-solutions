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
#include <unordered_map>
#include <random>

using namespace std;

#define rep(i, from, to) for (ll i = from; i < ll(to); ++i)
#define trav(a, x) for (auto& a : x)
#define all(x) x.begin(), x.end()
#define sz(x) (ll)(x).size()
#define print(x) trav(a, x) cout << a << " "; cout << endl;
#define pb push_back
#define fi first
#define se second

typedef long long ll;
typedef pair<ll, ll> pi;
typedef vector<ll> vi;
typedef vector<vector<ll> > vvi;

const int MAXN = 100001;

int parent[MAXN];
int rang[MAXN];

void make_set(int v) {
    parent[v] = v;
    rang[v] = 1;
}

int find_set(int v) {
    if (v == parent[v]) {
        return v;
    }
    return parent[v] = find_set(parent[v]);
}

bool union_sets(int a, int b) {
    a = find_set(a);
    b = find_set(b);

    if (a != b) {
        if (rang[a] < rang[b]) {
            swap(a, b);
        }

        parent[b] = a;
        if (rang[a] == rang[b]) {
            rang[a]++;
        }
        return true;
    }

    return false;
}

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

struct Photo{
    int id;
    bool horizontal = false;
    vector<string> tags;
};

struct instance{
  ll N; // from problem description
  vector<Photo> photos;

  // slideshow
  vector<vector<int> > slideshow;

  ll instance_score = 0;
  // stores the respective objects

  // read instance
  void read(){
    cin >> N;
    photos.resize(N);
    int nbr_tags;
    char c;
    rep(i, 0, N){
      photos[i].id = i;
      cin >> c;
      if (c == 'H') photos[i].horizontal = true;
      cin >> nbr_tags;
      photos[i].tags.resize(nbr_tags);
      rep(j, 0, nbr_tags) cin >> photos[i].tags[j];
    }
  }

  ll get_score(vector<int> &a, vector<int> &b){
    set<string> first_string_set, second_string_set;
    for(int el: a) for (string s: photos[el].tags) first_string_set.insert(s);
    for(int el: b) for (string s: photos[el].tags) second_string_set.insert(s);

    int inter = 0;
    for (string s : first_string_set) if (second_string_set.find(s) != second_string_set.end()) inter++;

    ll ret = min(min((int)first_string_set.size(),(int) second_string_set.size()) - inter, inter);
    // print(first_string_set);
    // print(second_string_set);
    // cout << ret << endl;
    return ret;
  }

  int get_fast_score(set<string>& first_string_set, set<string>& second_string_set){
    int inter = 0;
    for (string s : first_string_set) if (second_string_set.find(s) != second_string_set.end()) inter++;

    return min(min((int)first_string_set.size(),(int) second_string_set.size()) - inter, inter);

  }

  void assign(int cur, vector<vector<int>>& edges, vector<vector<int>>& preslideshow,
        vector<bool>& used, int n){

    int prev = cur;
    slideshow.pb(preslideshow[cur]);
    int next = edges[cur][0];
    used[cur] = true;
    while(true){
      prev = cur;
      cur = next;
      used[cur] = true;
      slideshow.pb(preslideshow[cur]);
      if (edges[cur].size() == 1) break;
      if (edges[cur][0] == prev) next = edges[cur][1];
      else next = edges[cur][0];
    }
  }


  void solve(){
    vector<int> verticals, horizontals;
    vector<vector<int>> preslideshow;

    trav(p, photos) {
      if (!p.horizontal) verticals.pb(p.id);
      else horizontals.pb(p.id);
    }
    shuffle(all(horizontals), default_random_engine(time(NULL)));
    // join verticals
    for (int i = 0; i + 1 < (int) verticals.size(); i = i + 2){
      preslideshow.pb({verticals[i], verticals[i+1]});
    }

    // match horizontals
    rep(i, 0, horizontals.size()) preslideshow.pb({horizontals[i]});

    unordered_map<string, vector<int> > mapping;
    rep(i, 0, preslideshow.size()) trav(pid, preslideshow[i]) trav(s, photos[pid].tags) mapping[s].pb(i);

    map<int, vector<pi>> score; // score of slide i to slide j
    map<pair<int, int>,int> inter;

    vector<set<string> > string_sets(preslideshow.size());
    rep(i, 0, preslideshow.size()){
      trav(el, preslideshow[i]){
        trav(s, photos[el].tags) string_sets[i].insert(s);
      }
    }

    cout << mapping.size() << endl;

    trav(p, mapping){
      auto cands = p.second;
      cout << cands.size() << endl;
      for (int i = 0; i < cands.size(); i++){
          for (int j = 0; j < i; j = j + 5){
            if (cands[i] < cands[j]) inter[{cands[i],cands[j]}]++;
            else inter[{cands[j],cands[i]}]++;
          }
      }
    }

    cout << "got score" << endl;
    cout << inter.size() << endl;
    vector<pair<pair<int,int>,int>> vecscores;
    trav(p, inter){
      vecscores.pb({p.fi, min(min((int)string_sets[p.fi.fi].size(),(int)string_sets[p.fi.se].size()) - p.se, p.se)});
    }
    sort(all(vecscores), [](const auto& a, const auto& b){if (a.se == b.se) return a.fi < b.fi; else return a.se > b.se;});


    int n = preslideshow.size();
    cout << n << endl;

    vector<int> cnt(n, 0);
    vector<vector<int> > edges(n);
    rep(i, 0, n) make_set(i);
    // smart connection
    int ii = 0;
    trav(pp, vecscores){
      int a = pp.fi.fi; int b = pp.fi.se;
      if (cnt[a] < 2 && cnt[b] < 2 && find_set(a) != find_set(b)){

        cnt[a]++;
        cnt[b]++;
        edges[a].pb(b);
        edges[b].pb(a);
        union_sets(a,b);
        ii++;
      }
      if (ii == n - 1) break;
    }

    vector<bool> used(n, false);

    rep(i, 0, n){
      if (cnt[i] == 1 && !used[i]){
        assign(i, edges, preslideshow, used, n);
      }
    }


    // vector<bool> used(n);
    // int i = 0;
    // slideshow.pb(preslideshow[i]);
    // used[i] = true;
    // rep(k, 0, n - 1){
    //   bool found = false;
    //   trav(p, score[i]){
    //     if (!used[p.se]){
    //       i = p.se;
    //       used[i] = true;
    //       slideshow.pb(preslideshow[i]);
    //       found = true;
    //       break;
    //     }
    //   }
    //   if (!found){
    //     rep(j, 0, n){
    //       if (!used[j]){
    //         i = j;
    //         used[i] = true;
    //         slideshow.pb(preslideshow[i]);
    //         break;
    //       }
    //     }
    //   }
    // }

  }


  ll get_solution_score(){
    ll ret = 0;
    rep(i, 0, slideshow.size() - 1) ret += get_score(slideshow[i], slideshow[i+1]);
    return ret;
  }


  void write() {
    cout << slideshow.size() << endl;
    for (vector<int> v: slideshow){
      for (int el : v) cout << el << " ";
      cout << endl;
    }
  }

};

instance I;
int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);
  string input_file = argv[1];

  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  //I.get_upper_bound();
  I.solve();

  ll score = I.get_solution_score();
  cerr << score << endl;
  //I.evaluate();
  string output_file = input_file + "out" + to_string(score);
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();
}
