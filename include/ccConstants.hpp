

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

/// \file ccConstants.hpp
/// \brief constant variables (costs, velocity, padding)

#ifndef CC_CONSTANTS_HEADER_INCLUDED
#define CC_CONSTANTS_HEADER_INCLUDED

namespace cc{

  /// Costs
  struct Cost{
    /// Cost per unit area
    static constexpr double PerUnitArea = .75;
    /// Cost per second
    static constexpr double PerSecond = .07;
    /// For setting precision $xxx.00 when pretty printing
    static const int Decimals( double val, int nd ){
      int i = floor(val/10);
      int ct = 1;
      while (i>0){
        ct++; i = i >> 1;
      }
      return ct+2;
    }
  };

  /// Velocity in inches pers second
  struct Velocity{
    /// Max Velocity
    static constexpr double Max = .5;
    /// Max Velocity weighted by exp(-(1/radius))
    static double Radius( double radius ){
      return Max * exp(-(1.0/radius));
    }
  };

  /// Material variables
  struct Material{
    /// Amount of padding on width and height
    static constexpr double Padding = .1;
  };

} //c::


#endif /* end of include guard: CC_CONSTANTS_HEADER_INCLUDED */
