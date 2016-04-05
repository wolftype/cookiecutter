# cookiecutter

#### Lightweight Solution to a 2D Geometry Problem.
This repo was created in order to solve a specific "coding challenge" assignment
to estimate the cost of manufacturing of some simple shapes based on time it will take to laser etch the path and the amount of material required to make it.

### A Quote for the Cookie Cutter
In a terminal, typing:
```
git clone https://github.com/wolftype/cookiecutter.git
cd cookiecutter
./run.sh
```
will build with cmake and run the `ccquote` binary in `build/bin/`.

Without arguments the binary will output the cost quotes for the three given `files/*.json` schemas.  
Additional quotes can be generated by passing a filename to the `ccquote` binary:

    ./build/bin/ccquote files/<your_json_file>

After running `./run.sh`, if you have `doxygen` installed you can build the `doxyfile`:

    doxygen doxyfile

which will generate `html` and `tex` files in the `docs/` folder

### Contents of `include/`
* `cc.hpp`:  Includes all headers.
* `ccData.hpp`: Loads data and runs analysis.
* `ccGeometry.hpp`: Representation of 2D vectors, edges, arcs, and hulling.
* `ccConstants.hpp`: Domain constants such as `Cost::PerSecond` and `Cost::PerArea`.
* `ccMacros.hpp`:  Mathematic constants. Just `PI` for now.
* `ccFile.hpp`: Simple file loading


### Basic Usage

```c
#include "cc.hpp"
#include <iostream>

using namespace cc;

int main(){
  Data data("files/Rectangle.json");     //<-- load into memory
  data.resolution( 100 );                //<-- set resolution of discretization
  double cost = data.cost();             //<-- analyze cost
  std::cout << cost << std::endl;
  return 0;
}
```

#### Framing the problem

After importing 2D Coordinate Data and mesh topology according to files/Schema.json (using a library such as [jsoncpp](https://github.com/open-source-parsers/jsoncpp)) the problem can be geometrically constructed as:

1. Integrate the (curvature-weighted) length of a specified path composed of straight lines and arc segments.
2. Find the area of the minimum bounding box of a set of points.

The first problem integrating length is trivial, with the length of arc segments calculated using an atan2 function.

A general solution to the second problem is slightly more involved, as it entails finding the convex hull of a set of points, and then finding the minimum bounding box of that hull. We note that the set of points are not just the vertices listed in the `.json` files, but must also include the results of discretizing arc segments between them, as these arcs may extrude from the straight line path between points and therefore affect the dimensions of the bounding box.

### Outline of Solution

1. Represent geometry in memory:
2. Load data into memory
3. Geo Task 1: find arc lengths in order to integrate length of path
4. Geo Task 2: find the mimimum bounding area of a discretized set of points
    * Use the [Monotone Chain Algorithm](https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain) to create convex hull of discretized data.
    * Use [Rotating Calipers](https://en.wikipedia.org/wiki/Rotating_calipers) method to find area of minimum bounding box of convex hull
5. Calculate the Time Cost based on lengths of edges and arcs and variable velocities, and the Material Cost based on area.

#### 1. Represent geometry
* Vertices:
```c
      struct Vec2 { double x,y; /*... methods ...*/ };
```
* Edges as pointers to Vertices:
```c
      struct Edge{
        std::vector< std::shared_ptr<Vec2> > mVec;
        /*... methods ...*/
      };
```
* CircularArc as Edges with additional data
```c
      struct CircularArc : Edge {
        Vec2 mCenter;
        bool bClockwise;
        /*... methods ...*/
      }
```
* Data to store std::vector of Vec2, Edge, CircularArc
```c
      class Data  {

        private:

          /// Vertices
          vector<Vec2> mVec;
          /// Map of Vertex ID to Vec2 Pointers
          map<int,std::shared_ptr<Vec2>> mMap;
          /// Straight Edges
          vector<Edge> mEdge;
          /// Circular Arcs
          vector<CircularArc> mCircularArc;
          /// Number of discretization steps, default is 20
          int mResolution = 20;

        public:
          /*... methods ...*/
      }
```
#### 2. Load data into memory
* use [jsoncpp](https://github.com/open-source-parsers/jsoncpp) library
* Store Efficiently -- use std::shared_ptrs since there are some wild cookie cutters out there!

#### 3. Geo Task 1: Integrate the (curvature-weighted) length of a specified path composed of straight lines and arc segments
* For Circular Arcs with vertices `ea` and `eb` and `center` vector, find radius.
```cpp
double radius = (ea-center).norm();
```
* Find vectors `va` and `vb` pointing to vertices from center of circle:
```cpp
Vector va = (ea-center);
Vector vb = (eb-center);
```
* Use atan2 function on cross and dot product of vectors to find theta between them:
```cpp
double theta = atan2( Vector::Cross(va,vb), Vector::Dot(va,vb));
```
* Scale theta [-PI,PI] to range [0-2PI]:
```cpp
theta = (theta >= 0) ? theta : PI + (PI-theta);
```
* `theta` now represents counterclockwise radians from `ea` to `eb` -- determine final theta based on input data, in range of [-2PI,2PI]
```cpp
theta =bClockwise ? -(2*PI - theta) : theta;
```
* Calculate Arc length
```cpp
double arclength = fabs(theta)*radius;
```
* Weigh `length` by radius and sum with edge lengths

#### 4. Geo Task 2: Find the Area of the Minimum Bounding Box of a Set of Points
* Discretize arcs using `theta` value calculated above in Task 1.
```cpp
std::vector<Vec2> input = data.discretize();
```
* Convex Hull the n points in O(n log(n)) time using the [Monotone Chain Algorithm](https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain)
  - Sort the n points by x value (by y value if x values are equal)
  - Iterate over the zig-zag sort to find lower and upper hulls.
```cpp
std::vector<Vec2> hull = Hull::Convex(input);
```
* Find the Minimum Bounding Box of the Convex Hull in O(n) time using the [Rotating Calipers](https://en.wikipedia.org/wiki/Rotating_calipers) method:
```cpp
   auto box = Hull::MinimumBox( hull );
```

# Going Further

Some thoughts on what would make productive next steps.

### Computation

For simplicity I have implemented my own geometry algorithms, but these should be compared with
other library solutions to find an optimal representation that includes:

1. Higher precision
  * alternative `atan2` formulas
2. Higher speed hulling computation
  * output-sensitive algorithms (e.g. Chan's)
3. Increased Robustness of code
  * Add exception handling (throw better errors)
  * Check for proper handling of collinear triples of points in `Hull::MinimumBox` method
  * Add exit strategy in `do...while` loop of `Hull::MinimumBox`

### Cost of Manufacturing

We can use some of the algorithms to solve more complicated problems.

1. Minimize Material for multiple projects
  * Group Multiple Forms into a single sheet
2. Minimize material for 3D objects with minimum volumes
  * Rotating calipers method has been used for this

### Code Design

1. Template the topological data on underyling data to enable testing with other CGAL libraries
2. Add in Visualization of algorithms and data for better debugging
3. We might hope that we could encode the arc segments as circles and treat this problem as a minimal bounding box of n-spheres (see for instance [this article](http://www.sciencedirect.com/science/article/pii/0925772195000240)), however that does not precisely simplify the problem, since only _segments_ of circles need to be hulled.  Still, the extent to which the use of such a representation this would speed up calculations or increase precision could be examined with more time.
