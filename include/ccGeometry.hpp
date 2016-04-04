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

/// \file ccGeometry.hpp
/// \brief Geometric Representation of 2D Vectors, Edges, Arcs, and Hulls

#ifndef CC_GEOMETRY_HEADER_INCLUDED
#define CC_GEOMETRY_HEADER_INCLUDED

#include <vector>
#include <math.h>
#include <memory>

namespace cc{

  /// 2D Vector which coordinates stored as doubles
  struct Vec2{

    double x,y;

    /// Subtraction of two Vec2s
    Vec2 operator -(const Vec2& b) const {
      return {x-b.x, y-b.y};
    }

    /// Addition
    Vec2 operator +(const Vec2& b) const {
      return {x+b.x, y+b.y};
    }
    /// Length
    double norm() const {
      return sqrt( x*x + y*y );
    }

    /// Compare is less then (for std::sort)
    bool operator < (const Vec2& b) const {
      return x < b.x || (x == b.x && y < b.y);
    }

    /// Test for equality
    bool operator == (const Vec2& b) const{
      return (x == b.x && y == b.y);
    };

    /// Vec2 of Unit Length
    Vec2 unit() const {
      double n = norm();
      if (n != 0) return { x/n, y/n };
      return {0,0};
    }

    /// return a new Vec2 by rotating this one by theta
    Vec2 rotate(double theta){
      return Vec2::Construct( Vec2::Theta(*this)+theta, norm() );
    }

    /// Dot Product
    static double Dot( const Vec2& a, const Vec2& b){
      return a.x*b.x + a.y*b.y;
    }

    /// Euclidean Distance
    static double Dist(const Vec2& a, const Vec2& b){
      return (a-b).norm();
    }

    /// Cross Product
    static double Cross(const Vec2& a, const Vec2& b){
      return a.x*b.y-a.y*b.x;
    }

    /// Radians between Vectors in range [-PI,PI]
    static double Theta(const Vec2& a, const Vec2& b){
      auto ta = a.unit();
      auto tb = b.unit();
      return atan2(Cross(ta,tb),Dot(ta,tb));
    }

    /// Radians from (1,0)
    static double Theta(const Vec2& v){
      return atan2(v.y, v.x);
    }

    /// Construct from center point, theta, and radius
    static Vec2 Construct( const Vec2& mCenter, double theta, double radius ){
      return mCenter + Vec2{cos(theta)*radius, sin(theta)*radius};
    }

    static Vec2 Construct( double theta, double radius ){
      return {cos(theta)*radius, sin(theta)*radius};
    }

  };

  /// shared pointers to Vec2
  struct Edge{
    int id;                                     ///< Unique ID from JSON file
    std::vector< std::shared_ptr<Vec2> > mVec;  ///< Pointers to Vertices

    virtual double length(){
      if (mVec.size()>1){
        return ((*mVec[1]) - (*mVec[0])).norm();
      }
      return 0;
    }
  };

  /// An Edge with a center and orientation
  struct CircularArc : Edge {

    /// Center of circle
    Vec2 mCenter;

    /// Does Arc move Clockwise from First Vec2?
    bool bClockwise;

    /// Radians of arc
    double radians(){                                                                     /// Output Range:
      double t = Vec2::Theta((*mVec[0] - mCenter) ,(*mVec[1] - mCenter));                 ///<-- [-PI,PI]
      t = (t>=0) ? t : PI + (PI-t);                                                       ///<-- [0,2PI]
      if (bClockwise) t = -(2*PI-t);                                                      ///<-- [-2PI,2PI]
      return t;
    }

    /// Radius of curvature of the arc
    double radius(){
      return Vec2::Dist(*mVec[0],mCenter);
    }

    /// Length of Arc
    virtual double length() override {
      return radius() * fabs( radians() );
    }

    /// Discretize Arc into n pieces
    std::vector<Vec2> discretize(int res){
        std::vector<Vec2> result(res);
        double theta = radians();
        double start = Vec2::Theta( (*mVec[0] - mCenter) );
        double r = radius();
        for (int i =0;i<res;++i){
          float t = (float)i/res;
          // construct Vec2 from center point, angle, and radius
          result[i] = Vec2::Construct(mCenter, start+theta*t, r);
        }
        return result;
    }
  };

  /// Analysis of point cloud data
  struct Hull {

      /// \brief Minimum Bounding Box data
      struct Box{
        Vec2 para[4];           ///< directions of parallel support lines
        int idx[4];             ///< index into data of min/max points
        double width, height;   ///< dimensions of data
      };


      /// Create Convex hull with the Monotone Chain Algorithm
      /// \param input reference to std::vector< Vec2 >.
      /// (Passed by reference because these are sorted in place)
      /// \returns std::vector<Vec2> an ordered, convex, closed loop of points
      /// \todo consider parallel sort
      static std::vector<Vec2> Convex( std::vector<Vec2>& input ){

        std::vector<Vec2> result;
        std::vector<Vec2> upper;
        std::vector<Vec2> lower;

        ///1. sort by x and then by y (uses Vec2::operator < (...))
        std::sort( input.begin(), input.end() );

        ///2. calculate lower hull
        for (int i=0;i<input.size();++i){

            while(  lower.size() >= 2 &&
                    Vec2::Cross(lower[lower.size()-1] - lower[lower.size()-2],
                                input[i] - lower[lower.size()-2]) <= 0)
            {
              lower.pop_back();
            }
           lower.push_back(input[i]);
        }

        ///3. calculate upper hull
        for (int i=input.size()-1; i>=0;--i){

            while(  upper.size() >= 2 &&
                    Vec2::Cross(upper[upper.size()-1] - upper[upper.size()-2],
                                input[i] - upper[upper.size()-2]) <= 0)
            {
              upper.pop_back();
            }
           upper.push_back(input[i]);
        }

        ///4. eliminate last of each (redundant)
        lower.pop_back();
        upper.pop_back();
        ///5. concatenate results and return
        result = lower;
        for (auto& i : upper ) result.push_back(i);
        return result;
      }

      /// Find Minimum Bounding Box using "Rotating Calipers"
      /// \param input a convex hull
      /// \return Box coordinates of minimum bounding box
      static Box MinimumBox( const std::vector<Vec2> input){

          Box box; //< here we will store bounding box data
          if (input.size() < 3 ) return box;

          /// 1. Parallel Support Lines start off ccw vertical and horizontal
          //through minimum x , maximum x, minmum y, and maximum y values
          Vec2 para[4]  = { {0,-1}, {0,1}, {1,0}, {-1,0} };
          // Angles between parallel support lines and next edge along hull
          double theta[4];
          double minTheta = PI;

          // idx of Vecs
          int idx[4] = {0,0,0,0};

          /// 2. Get idx of vecs at extremes
          Vec2 tminX, tmaxX, tminY, tmaxY;
          Vec2 minX, maxX, minY, maxY;
          minX = maxX = minY = maxY = input[0];
          for (int i =0;i < input.size(); ++i){
            tminX = tmaxX = tminY = tmaxY = input[i];
            if (tminX.x < minX.x) { minX = tminX; idx[0]=i; }
            if (tmaxX.x > maxX.x) { maxX = tmaxX; idx[1]=i; }
            if (tminY.y < minY.y) { minY = tminY; idx[2]=i; }
            if (tmaxY.y > maxY.y) { maxY = tmaxY; idx[3]=i; }
          }

          // store next idx of minX
          int loopIdx = idx[0]+1;

          /// 3. Calculate starting area
          Vec2 hx = input[idx[1]] - input[idx[0]];
          double width = fabs( Vec2::Cross(para[0],hx) );
          Vec2 hy = input[idx[3]] - input[idx[2]];
          double height = fabs( Vec2::Cross(para[2],hy) );
          double minArea = width*height;

          /// 4. Rotate Calipers in search of minimum area
          do {
            // Find minimum radians we can rotate parallel lines around convex hull
            minTheta = PI;
            for (int i =0;i<4;++i){
               int next = (idx[i] < (input.size() -1) ) ? idx[i]+1 : 0;
               cc::Vec2 edge = (input[next] - input[idx[i]]).unit();
               theta[i] = asin( Vec2::Cross( para[i], edge ) );
               if (theta[i] < minTheta){
                 minTheta = theta[i];
               }
            }
            // rotate all lines by minTheta
            for (int i =0;i<4;++i){
                para[i] = para[i].rotate( minTheta );
                if (theta[i]==minTheta){ // if theta represents minimum, idx next point
                  idx[i] = ( idx[i] < (input.size() - 1) ) ? idx[i]+1 : 0 ;
                }
            }

            // calculate width and height
            Vec2 hx = input[idx[1]] - input[idx[0]];
            double width = fabs(Vec2::Cross(para[0], hx));
            Vec2 hy = input[idx[3]] - input[idx[2]];
            double height = fabs(Vec2::Cross(para[2], hy));
            double area = width*height;
            if (area < minArea) {
              minArea =  area;
              box.width = width;
              box.height = height;
              /// \todo memcpy box data storage for speed boost
              for (int j=0;j<4;++j){
                box.para[j] = para[j];
                box.idx[j] = idx[j];
              }
            }

          } while (idx[3] != loopIdx );

          return box;
      }
  };

} //cc::



#endif /* end of include guard: CC_GEOMETRY_HEADER_INCLUDED */
