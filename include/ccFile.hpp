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

/// \file ccFile.hpp
/// \brief Basic file loader

#ifndef CC_FILE_HEADER_INCLUDED
#define CC_FILE_HEADER_INCLUDED

#include <fstream>
#include <stdexcept>

namespace cc {

    /// Bare bones file loader: call with File::Load(path/to/filename)
    struct File {

      /// Pass in filepath relative to project source directory (e.g. "files/Rectangle.json")
      static void Load(std::string filepath, std::fstream& myfile){
      //  std::fstream myfile;

        // Search for file by going up file directory tree up to 5 times
        int attempts = 0;
        std::string nfilepath = filepath;
        while (!myfile.is_open() && attempts < 5) {
            myfile.open( nfilepath.c_str(), std::ios::in );
            nfilepath = "../" + nfilepath;
            attempts += 1;
        }
        if (!myfile.is_open()) throw std::invalid_argument("Error: File Not Found.");

      }

    };

} //cc::


  #endif /* end of include guard: CC_FILE_HEADER_INCLUDED */
