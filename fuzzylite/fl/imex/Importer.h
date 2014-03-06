// #BEGIN_LICENSE
// fuzzylite: a fuzzy logic control library in C++
// Copyright (C) 2014  Juan Rada-Vilela
// 
// This file is part of fuzzylite.
//
// fuzzylite is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// fuzzylite is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fuzzylite.  If not, see <http://www.gnu.org/licenses/>.
// #END_LICENSE

/* 
 * File:   Importer.h
 * Author: jcrada
 *
 * Created on 25 December 2012, 11:43 PM
 */

#ifndef FL_IMPORTER_H
#define	FL_IMPORTER_H

#include "fl/fuzzylite.h"

#include <string>

namespace fl {
    class Engine;

    class FL_EXPORT Importer {
    public:

        Importer() {
        }

        virtual ~Importer() {
        }
        
        virtual std::string name() const = 0;
        virtual Engine* fromString(const std::string& s) const = 0;
    };

}

#endif	/* IMPORTER_H */

