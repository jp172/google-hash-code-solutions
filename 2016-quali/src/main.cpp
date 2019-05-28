/*
  Solution to Drone Delivery Problem (GHC 2016).
  The algorithm works simulation based by assigning drones a new route upon
  availability. For an available drone the customer is determined that can be
  served (approximatively) the most in the fastest time (see get_score for
  details). Together with the customer the best warehouse is determined.
  If there is some space left in the drone, additional customers are
  chose that can also be served by items from the same warehouse. Again,
  get_score is used for determining additional customers. Once all customers are
  chosen, the drone flies on its route in will be put into the queue until
  availability.

  Usage: - make
         - ./solve <input file>
*/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>
#include <cmath>
#include <unordered_set>


using namespace std;
typedef vector<int> vi;
typedef pair<int, int> pi;



class warehouse{
public:
  int id; // warehouse id
  pi location; // location (x-coordinate, y-coordinate)
  vi items; // list of all items (by product id) andhow many items it is stored
};

class customer{
public:
  int id; // customer id
  pi location; // location (x-coordinate, y-coordinate)
  multiset<int> order;// product ids the customer needs
};
class drone{
public:
  int id; // drone id
  pi location; // start location
  int available_at = 0; // time drone is available
  int current_load_weight = 0; // weight of items the drone carries
  multiset<int> store; // list of items the drone has in store
};

template <class T> T sq (T a) { return (a * a);}

int dis(pi a, pi b){ // ceiled distance between two locations
  return ceil(sqrt(sq(a.first - b.first) + sq(a.second - b.second)));
}

class instance{
public:
  void read(){
    cin >> R >> C >> D >> T >> M;
    // read products
    cin >> P;
    product_weight.resize(P,0);
    for (int i = 0; i < P; i++) cin >> product_weight[i];
    // read in warehouses, locations, and stored items
    cin >> W;
    warehouses.resize(W);
    for (int i = 0; i < W; i++){
      warehouses[i].id = i;
      cin >> warehouses[i].location.first >> warehouses[i].location.second;
      warehouses[i].items.resize(P);
      for (int j = 0; j < P; j++) cin >> warehouses[i].items[j];
    }
    // read in customer data
    cin >> O;
    customers.resize(O);
    for (int i = 0; i < O; i++){
      customers[i].id = i;
      cin >> customers[i].location.first >> customers[i].location.second;
      int n; cin >> n;
      for (int j = 0; j < n; j++){
        int id; cin >> id;
        customers[i].order.insert(id);
      }
    }
    // read in drones
    drones.resize(D);
    for (int i = 0; i < D; i++){
      drones[i].id = i;
      drones[i].location = warehouses[0].location;
    }
  }

  // writes output as described in problem specs
  // output was stored in out object
  void write(){
    cout << out.size() << endl;
    for (auto v: out){
      cout << v[0] << " ";
      if (v[1] == 3) cout << "W " << v[2];
      else{
        if (v[1] == 0) cout << "L ";
        else if (v[1] == 1) cout << "D ";
        else if (v[1] == 2) cout << "U ";
        cout << v[2] << " " << v[3] << " " << v[4];
      }
      cout << endl;
    }
  }

  // scoring function for (if customer can be fully served) * (serving customer at time t)
  // note that the binary decision if the customer can be served or not is
  // smoothed by a polynomial f(x) = x^10
  double get_score(int weight, int weight_required, int t){
    return pow(double(weight)/double(weight_required), 10) * ((double(T) - double(t))/(1.0 * double(T)) * 100.0);
  }

  // greedily selects all items that
  // - drone d can carry
  // - warehouse w has in stock
  // - customer c needs
  vi get_items(drone &d, warehouse w, customer &c){
    int capacity_left = M - d.current_load_weight; // drone capacity
    vi ret;
    // iterates all items in the customers order
    for (int item: c.order){
      if(w.items[item] > 0 && product_weight[item] < capacity_left){
        capacity_left -= product_weight[item];
        ret.push_back(item);
        w.items[item]--;
      }
    }
    return ret;
  }

  // loads all items to the drone obtained by get_items(d,w,c)
  void load(drone &d, warehouse &w, customer &c){
    vi items = get_items(d, w, c);
    for (int i: items){
      d.current_load_weight += product_weight[i];
      d.store.insert(i);
      w.items[i]--;
      d.available_at++;

      // writing operation to output
      out.push_back({d.id, 0, w.id, i, 1});
    }
    d.available_at += dis(d.location, w.location);
    d.location = w.location;
  }

  // delivers all items customer c needs and drone d has
  void deliver(drone &d, customer &c){
    unordered_multiset<int> to_delete;
    for (int i: c.order){
      if (d.store.find(i) == d.store.end()) continue;
      d.store.erase(d.store.find(i));
      to_delete.insert(i);
      d.available_at++;
      d.current_load_weight -= product_weight[i];
      // writing operation to output
      out.push_back({d.id, 1, c.id, i, 1});
    }
    for (int i: to_delete) c.order.erase(c.order.find(i));
    d.available_at += dis(d.location, c.location);
    d.location = c.location;
  }

  // if the drone has some capacity left load it with items for some other customer
  int get_items_for_other_customers(drone &d, warehouse &w, customer &c, int &current_time_spent){
    double max_score = 0.00001;
    int max_id = -1;
    int new_time_required = 0;

    // iterate among all other customers and get customer with highest score
    for (customer &c2: customers){
      if (c2.id == c.id) continue;
      vi items = get_items(d, w, c2);
      if (items.empty()) continue;

      int weight_required = 0;
      for (int i: c2.order) weight_required += product_weight[i];
      if (weight_required == 0) continue;
      int weights_possible = 0;
      for (int i: items) weights_possible += product_weight[i];
      int time_required = dis(c.location, c2.location) + items.size() * 2;
      double score = get_score(weights_possible, weight_required, current_time_spent + time_required + d.available_at);
      if (score > max_score){
        max_score = score;
        max_id = c2.id;
        new_time_required = dis(c.location, c2.location) + items.size(); //
      }
    }
    // if some new customer is found, load all goods to the drone
    if (max_id != -1){
      load(d, w, customers[max_id]);
      current_time_spent += new_time_required;
    }
    return max_id;
  }

  // executes the process of drone d serving customer c via warehouse w
  void execute(drone &d, customer &c, warehouse &w){
    // loading all products for customer c, which are available at warehouse w
    load(d, w, c);

    // additionally, since the drone is already at a warehouse it is checked
    // which other customer can also be satisfied fast via drone d and by
    // picking up goods from warehouse w
    vector<int> ids = {c.id};
    int current_customer_id = c.id;
    int current_time_spent = dis(w.location, c.location) + d.store.size(); // time required to fly to customer and deliver
    while (1){
      current_customer_id = get_items_for_other_customers(d, w, customers[current_customer_id], current_time_spent);
      if (current_customer_id == -1) break;
      else ids.push_back(current_customer_id);
    }

    // after all customers are picked and the goods are loaded
    // the drone delivers to all customers
    for (int id: ids) deliver(d, customers[id]);

  }

  // returns the score of drone d serving customer c next equipped with the best
  // suited warehouse for serving customer c
  pair<int, double> myscore(drone &d, customer &c){
    if (c.order.empty()) return {0,0};
    double max_score = 0;
    int max_warehouse_id = 0;
    int weight_required = 0;
    for (int i: c.order) weight_required += product_weight[i];
    for (warehouse &w: warehouses){
      int weight = 0;
      vi items = get_items(d, w, c);
      for (int i: items) weight += product_weight[i];

      // time required to execute operation
      int tics = dis(d.location, w.location) + dis(w.location, c.location) + items.size() * 2;
      double score = get_score(weight, weight_required, tics + d.available_at);
      if (items.empty()) score = 0;
      if (score > max_score){
        max_score = score;
        max_warehouse_id = w.id;
      }
    }
    return {max_warehouse_id, max_score};
  }

  // searches for customer and warehouse such that the drone d can deliver as
  // many items the customer needs and the warehouse has in stock
  bool greedy(drone &d){
    int max_customer_id = -1;
    int max_warehouse_id = 0;
    double max_score = 0;
    for (int i = 0; i < O; i++){
      customer &c = customers[i];
      if (c.order.empty()) continue;
      pair<int, double> p = myscore(d, c);
      if (p.second > max_score){
        max_score = p.second;
        max_customer_id = i;
        max_warehouse_id = p.first;
      }
    }
    if (max_customer_id == -1) return false;
    execute(d, customers[max_customer_id], warehouses[max_warehouse_id]);
    return true;
  }

  void solve(){
    set<pi> queue;
    for (int i = 0; i < D; i++) queue.insert({0, i});
    // time-based simulation of assigning delivery routes to drones
    while (!queue.empty()){
      pi cur = *queue.begin(); queue.erase(*queue.begin());
      if (cur.first > T) break;
      if (greedy(drones[cur.second])) queue.insert({drones[cur.second].available_at, cur.second});
    }
  }

  // input variables
  int R, C, D, T, M, W, P, O;
  // R - number rows
  // C - number columns
  // D - number drones
  // T - maximum time
  // M - maximal load drones can carry
  // W - number of warehouses
  // P - number of different products
  // O - number of customers

  vector<drone> drones; // stores drone objects
  vector<warehouse> warehouses; // stores warehouse objects
  vector<customer> customers; // stores customer objects
  vi product_weight; // weight of each product
  vector<vi> out; // stores drone instructions in output format

};


int main(int argc, char* argv[]){
  instance I;
  ios::sync_with_stdio(false);
  cin.tie(0);
  string input_file = argv[1];
  string output_file = "out";
  freopen(input_file.c_str(), "r", stdin); // redirects standard input
  I.read();
  I.solve();
  freopen(output_file.c_str(), "w", stdout); // redirects standard output
  I.write();
}
