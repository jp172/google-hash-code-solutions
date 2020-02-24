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
#include <numeric>
#include "gurobi_c++.h"

using namespace std;

#define rep(i, from, to) for (ll i = from; i < ll(to); ++i)
#define trav(a, x) for (auto& a : x)
#define all(x) x.begin(), x.end()
#define sz(x) (int)(x).size()
#define print(x) trav(a, x) cout << a << " "; cout << endl;
#define pb push_back
#define fi first
#define se second

typedef long long ll;
typedef pair<ll, ll> pi;
typedef vector<ll> vi;
typedef vector<vector<ll> > vvi;


struct Book{
    ll id;
    ll score;
};

struct Library{
    ll id;
    ll sign_up_time;
    ll books_per_day;
    bool signed_up = false;
    vector<int> books;
    vector<int> books_for_scan;
    double cur_score = 0;
};
ll B, L, D; // from problem description
vector<Book> books;
vector<Library> libraries;

//
vector<int> library_queue;
vector<bool> used_book;

struct instance{


  ll instance_score = 0;
  // stores the respective objects

  // read instance
  void read(){
    cin >> B >> L >> D;
    books.resize(B);
    used_book.resize(B);
    rep(i, 0, B){
      books[i].id = i;
      cin >> books[i].score;
    }
    libraries.resize(L);
    int b;
    rep(i, 0, L) {
      libraries[i].id = i;
      cin >> b;
      libraries[i].books.resize(b);
      cin >> libraries[i].sign_up_time >> libraries[i].books_per_day;
      rep(j, 0, b) cin >> libraries[i].books[j];
    }

  }

  void get_upper_bound(){
    ll ret = 0;
    trav(b, books) ret += b.score;
    cerr << "upper bound: " << ret << endl;
  }

  int get_best_library(int day){

    int ret = -1;
    double best_score = 0;
    trav(l, libraries){
      if (l.signed_up) continue;
      sort(all(l.books), [](const auto& a, const auto& b){if (used_book[a] == used_book[b]) return books[a].score > books[b].score; else return used_book[a] < used_book[b]; });
      int rem_books = (D - day - l.sign_up_time) * l.books_per_day;

      l.cur_score = 0;

      rep(i, 0, min(sz(l.books), rem_books)){
        if (!used_book[l.books[i]]) {
          l.cur_score += books[l.books[i]].score;
        }
      }

      l.cur_score = l.cur_score / l.sign_up_time;

      if (l.cur_score > best_score){
        best_score = l.cur_score;
        ret = l.id;
      }

    }
    return ret;
  }

  bool assign_library_books(int lid, int day){
    Library& l = libraries[lid];

    sort(all(l.books), [](const auto& a, const auto& b){if (used_book[a] == used_book[b]) return books[a].score > books[b].score; else return used_book[a] < used_book[b]; });

    int nbr_books = (D - day - l.sign_up_time) * l.books_per_day;

    vector<int>& tmpbooks = l.books;
    rep(i, 0, min(nbr_books, sz(tmpbooks))){
      if (used_book[tmpbooks[i]]) break;
      used_book[tmpbooks[i]] = true;
      l.books_for_scan.pb(tmpbooks[i]);
      instance_score += books[tmpbooks[i]].score;
    }
    if (sz(libraries[lid].books_for_scan) > 0) return true;
    else return false;
  }

  int get_ip_solution(){
    GRBEnv env;
    GRBModel m = GRBModel(env);
    m.getEnv().set("OutputFlag", "1");
    m.getEnv().set("TimeLimit", "10.0");
    m.getEnv().set("MIPgap", "0");
    //m.getEnv().set("ModelSense","-1");

    vector<map<int, GRBVar> > x(L); //

    trav(l, libraries){
        trav(bid, l.books){
          x[l.id][bid] = m.addVar(0, 1, 0, GRB_BINARY);
        }
    }

    m.update();

    // add library constraints
    int rem_days = D;
    for (int lid: library_queue){

      Library& l = libraries[lid];
      rem_days -= l.sign_up_time;
      GRBLinExpr expr = 0;
      trav(bid, l.books){
        expr += x[lid][bid];
      }

      m.addConstr(expr <= rem_days * l.books_per_day);
      m.addConstr(expr <= sz(l.books));
    }

    // add objective constraints
    vector<vector<int> > books_to_libraries(B);

    trav(lid, library_queue){
      Library& l = libraries[lid];
      trav(bid, l.books) books_to_libraries[bid].pb(l.id);
    }

    GRBLinExpr obj = 0;

    rep(bid, 0, B){
      GRBLinExpr expr2 = 0;
      trav(lid, books_to_libraries[bid]) {
        expr2 += x[lid][bid];
        obj += x[lid][bid] * books[bid].score;
      }
      m.addConstr(expr2 <= 1);
    }

    m.setObjective(obj, GRB_MAXIMIZE);

    m.optimize();

    // retrieve assignment
    int score = 0;

    trav(lid, library_queue){
      libraries[lid].books_for_scan.clear();
      trav(bid, libraries[lid].books){
        if (x[lid][bid].get(GRB_DoubleAttr_X) > 0.5){
          libraries[lid].books_for_scan.pb(bid);
          score += books[bid].score;
        }
      }
    }

    return score;

  }

  void solve(){

    int day = 0;

    while (day <= D){
      int best = get_best_library(day);
      if (best == -1) break;
      libraries[best].signed_up = true;
      if (assign_library_books(best, day)){
        library_queue.pb(best);
      }
      day += libraries[best].sign_up_time;
    }

  }

  void write() {
    int cnt = 0;
    for (int lid : library_queue){
      if (libraries[lid].books_for_scan.size() > 0) cnt++;
    }
    cout << cnt << endl;

    trav(lid, library_queue){
      if (libraries[lid].books_for_scan.size() == 0) continue;
      cout << lid << " " << libraries[lid].books_for_scan.size() << endl;
      print(libraries[lid].books_for_scan);
    }
  }

};

instance I;
int main(int argc, char* argv[]){

  ios::sync_with_stdio(false);
  cin.tie(0);
  string input_file = argv[1];
  srand(time(NULL));
  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  I.get_upper_bound();
  I.solve();
  int best_score = I.get_ip_solution();
  string output_file = input_file + "out" + to_string(best_score);
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();

}
