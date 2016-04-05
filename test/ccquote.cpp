/// If called with a file/*.json, will load data and analyze cost
/// Otherwise, prints cost of
/// Rectangle.json, ExtrudeCircularArc.json, and CutCircularArc.json

#include "cc.hpp"

#include <iostream> //std::cout
#include <iomanip>  //std::setprecision

using namespace cc;
using namespace std;

/// Pretty Printing
void print(double cost){
  int prec = Cost::Decimals(cost,2);
  cout << "Estimated Cost: $" <<  std::setprecision(prec) << cost << " US Dollars."<< endl;
}

/// Pass optional "files/*.json" argument to binary
int main(int argc, char * argv[]) {

  Data data;
  double cost;
  data.resolution( 20 );
  /// process argument if one exists, otherwise process 3 default files
  if (argc > 1) {
    data.load(argv[1]);
    cost = data.cost();
    print(cost);
  } else {
    data.load("files/Rectangle.json");
    cost = data.cost();
    print(cost);

    data.load("files/ExtrudeCircularArc.json");
    cost = data.cost();
    print(cost);

    data.load("files/CutCircularArc.json");
    cost = data.cost();
    print(cost);

  }

  return 0;

}
