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
 * File:   Term.cpp
 * Author: jcrada
 *
 * Created on 18 December 2013, 9:55 AM
 */

#include "fl/term/Term.h"
#include "fl/imex/FllExporter.h"

namespace fl {

    Term::Term(const std::string& name) : _name(name) {

    }

    Term::~Term() {

    }

    void Term::setName(const std::string& name) {
        this->_name = name;
    }

    std::string Term::getName() const {
        return this->_name;
    }

    std::string Term::toString() const {
        return FllExporter("", "; ").toString(this);
    }
}