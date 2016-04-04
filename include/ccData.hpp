/*
 * =============================================================================
 * Copyright (C) 2010  Pablo Colapinto
 * All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * =============================================================================
*/

/// \file ccData.hpp
/// \brief Data Loading and Analysis


#ifndef CC_DATA_HEADER_INCLUDED
#define CC_DATA_HEADER_INCLUDED


#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <memory>


#include "json/json.h"      //< Parsing Library

#include "ccFile.hpp"       //< File Loading

#include "ccGeometry.hpp"   //< Data Representation

#include "ccConstants.hpp"      //< Costs information

namespace cc{

  using std::string;
  using std::map;
  using std::cout;
  using std::endl;
  using std::stringstream;
  using std::vector;

  /// \class Data
  /// \brief loads a json file into memory and runs analysis
  ///
  /// Stored data is structured as a std::vector<Vec2>
  /// Edges point to this memory, as do Circular Arcs
  /// \todo template data type (e.g. Data<Vec2>)

  class Data {

    private:

      /// Vertices
      vector<Vec2> mVec;
      /// Map of Vertex ID to Vec2 Pointers
      map<int,std::shared_ptr<Vec2>> mMap;
      /// Straight Edges
      vector<Edge> mEdge;
      /// Circular Arcs
      vector<CircularArc> mCircularArc;

    public:

      ///Empty Constructor
      Data(){}

      ///Construct and load string
      Data(std::string filename){ load(filename); }

      /// Initialize containers, clearing all members
      void init();

      /// Load json file following format of files/Schema.json
      void load(std::string filename);

      /// Discetize Circular Arc data
      /// \param res number of steps
      /// \returns point cloud std::vector
      vector<Vec2> discretize(int res=100);

      /// Area of Minimal Bounding Box
      /// \param res number of discretization steps
      /// \returns area in squared inches
      double area(int res=100);

      /// Time in seconds it will take to machine
      /// \returns integrated time calculation
      double seconds();

      /// Estimated Cost to Manufacture
      /// \param res number of discretization steps
      /// \returns cost in dollars
      double cost(int res = 100);

      /// Print out stored data
      void print();

      /// \todo print out in post-script format
      void printPS(){}

};


    inline void Data::init(){
      mMap.clear();
      mEdge.clear();
      mCircularArc.clear();
      mVec.clear();
    }

    //--------------------------------------------------------------------------
    /// Load json implementation:
    inline void Data::load(std::string filename){

        init();

        printf("Loading Data from %s...\n", filename.c_str());
        ///1. Load file into root
        Json::Value root;
        File::Load(filename) >> root;

        ///2. Store Vertex information first
        auto vertices = root["Vertices"];
        // Resize std::vector of Vec2
        mVec.resize(vertices.size());
        // Schema stores vertex ids as strings
        int iter = 0;
        for (auto& i : vertices.getMemberNames()){
          // save vertex name as int
          stringstream os; os << i; int id; os >> id;
          // copy in data as doubles
          mVec[iter] =
            { vertices[i]["Position"]["X"].asDouble(),
              vertices[i]["Position"]["Y"].asDouble() };
          ///3. Store vertex id as map to pointer in memory:
          mMap[id] = std::make_shared<Vec2>(mVec[iter]); iter++;
        }

        ///4. Now store edge data as pointer into memory
        auto edges = root["Edges"];
        for (auto& i : edges.getMemberNames()){
          // save edge name as int
          stringstream os; os << i; int id; os >> id;
          // check for edge type
          if (edges[i]["Type"].asString() == "LineSegment"){
            mEdge.push_back( Edge() );
            auto& e = mEdge.back();
            // set id
            e.id = id;
            // Use Vertex ID to look into Map of Pointers
            for (auto& j : edges[i]["Vertices"]){
              e.mVec.push_back( mMap[j.asInt()] );
            }
          }

          ///5. if edge type is a circle, store center and bClockwise boolean.
          if (edges[i]["Type"].asString() == "CircularArc"){
            mCircularArc.push_back( CircularArc() );
            auto& a = mCircularArc.back();
            // set id
            a.id = id;
            // Use Vertex ID to look into Map of Pointers
            for (auto& j : edges[i]["Vertices"]){
              a.mVec.push_back( mMap[j.asInt()] );
            }
            // copy in center point
            a.mCenter =
              { edges[i]["Center"]["X"].asDouble(),
                edges[i]["Center"]["Y"].asDouble() };
            // Do we move clockwise from first (compare first Vertex ID to CWFrom VertexID)
            a.bClockwise =
              edges[i]["Vertices"][0].asInt() == edges[i]["ClockwiseFrom"].asInt();

          }
        }

    }

    //--------------------------------------------------------------------------
    inline void Data::print(){
      cout << mVec.size() << " Vertices: " << endl;
      for (auto& i : mVec) cout << "X: "<< i.x << "\tY: " << i.y << endl;

      if (!mEdge.empty()){
        cout << mEdge.size() << " Straight Edges: " << endl;
        for (auto& i : mEdge ) {
          cout << "id: " << i.id << endl;
          for (auto& j : i.mVec){
            cout << j -> x << " " << j -> y << endl;
          }
        }
      }
      if (!mCircularArc.empty()){
        cout << mCircularArc.size() << " Circular Arc Edges: " << endl;
        for (auto& i : mCircularArc ) {
          cout << "id: " << i.id << endl;
          cout << "cw: " << i.bClockwise << endl;
          cout << "center: " << i.mCenter.x << " " << i.mCenter.y << endl;
          for (auto& j : i.mVec){
            cout << j -> x << " " << j -> y << endl;
          }
        }
      }
    }

    //--------------------------------------------------------------------------
    inline vector<Vec2> Data::discretize(int res){
      vector< Vec2 > points = mVec;
      //Additional vertices from arc discretization
      for (auto& i : mCircularArc){
        auto v = i.discretize(res);
        for (auto& j : v) points.push_back(j);
      }
      return points;
    };

    //--------------------------------------------------------------------------
    inline double Data::seconds(){
      double secs = 0;
      // tally length of each straight edge, divided by max speed
      for (auto& i : mEdge){
        if (i.mVec.size()<2) return 0;
        secs += i.length() / Velocity::Max;
      }
      // tally length of each arc, divided by (maxspeed * exp(-1/radius))
      for (auto& i : mCircularArc){
        if (i.mVec.size()<2) return 0;
        secs += i.length() / Velocity::Radius(i.radius());
      }
      return secs;
    }

    //--------------------------------------------------------------------------
    inline double Data::area(int res){
      //Point cloud with discretized curves
      vector< Vec2 > points = discretize();
      //Convex hull of point cloud
      auto hull = Hull::Convex(points);
      auto box = Hull::MinimumBox(hull);
      //multiply padded width and height
      return (box.width + Material::Padding ) * (box.height+Material::Padding);
    }

    //--------------------------------------------------------------------------
    inline double Data::cost(int res){
      return seconds() * Cost::PerSecond + area(res) * Cost::PerUnitArea;
    }

} //cc::


#endif /* end of include guard: CC_DATA_HEADER_INCLUDED */
