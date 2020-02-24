#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>
#include <queue>
#include <tuple>
#include <functional>
#include <unordered_set>
#include <numeric>

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

struct pizza{
  ll index, pieces;
};

bool mycmp (const pizza &a, const pizza &b){
  if (a.pieces == b.pieces) return a.index < b.index;
  else return a.pieces > b.pieces;
}

struct instance{
  ll N, M; // from problem description
  ll init_M;

  // stores the respective objects
  vector<pizza> pizzas;
  vector<bool> taken;
  // read instance
  void read(){
    cin >> M >> N;
    pizzas.resize(N);
    taken.resize(N);
    rep(i, 0, N){
      cin >> pizzas[i].pieces;
      pizzas[i].index = i;
    }
  }

  void assign_taken(vector<pizza> &piz){
    ll my_M = M;
    for (pizza &p: piz){
      if (my_M >= p.pieces){
        my_M -= p.pieces;
        taken[p.index] = true;
      }
    }
    cout << my_M << endl;
  }

  ll try_assign(vector<pizza> &piz){
    ll my_M = M;
    for (pizza &p: piz){
      if (my_M >= p.pieces){
        my_M -= p.pieces;
      }
    }
    return M - my_M;
  }

  void solve_simple_sorting(){
    //sort(all(pizzas), mycmp);
    auto best = pizzas;
    auto cur = pizzas;
    ll best_score = 0;
    rep(i, 0, 10000){
      random_shuffle(all(cur));
      ll new_score = try_assign(cur);
      if (new_score > best_score){
        best_score = new_score;
        cout << best_score << endl;
        best = cur;
      }
    }
    assign_taken(best);
  }

  void solve_dp(){
    unordered_map<ll, vector<int> > cands;
    cands[0] = {};
    for (int i = N - 1; i >= 0; i--){
      unordered_map<ll, vector<int> > new_cands = cands;
      for (auto p : cands){
        ll v = p.first + pizzas[i].pieces;
        if (v <= M){
          new_cands[v] = p.second;
          new_cands[v].pb(i);
        }
      }
      swap(cands, new_cands);

    }
    pair<ll, vector<int>>  opt_cands = *max_element(all(cands), [](auto& p, auto& q){return p.first < q.first;});
    cerr << opt_cands.fi << endl;
    for (int c : opt_cands.se) taken[c] = true;
  }

  ll get_score(){
    ll ret = 0;
    rep(i, 0, N) if (taken[i]) ret += pizzas[i].pieces;
    return ret;
  }

  void write() {
    ll nbr = accumulate(taken.begin(), taken.end(), 0LL);
    cout << nbr << endl;
    rep(i, 0, N) if (taken[i]) cout << i << " ";
    cout << endl;
  }
};

instance I;
int main(int argc, char* argv[]){
  ios::sync_with_stdio(false);
  cin.tie(0);

  string input_file = argv[1];

  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  I.solve_simple_sorting();
  ll score = I.get_score();
  string output_file = input_file + "out-" + to_string(score);
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();

}
