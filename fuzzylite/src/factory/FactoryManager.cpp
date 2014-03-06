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
 * File:   Factory.cpp
 * Author: jcrada
 * 
 * Created on 8 January 2013, 11:10 PM
 */

#include "fl/factory/FactoryManager.h"
#include "fl/factory/DefuzzifierFactory.h"
#include "fl/factory/SNormFactory.h"
#include "fl/factory/TNormFactory.h"
#include "fl/factory/TermFactory.h"
#include "fl/factory/HedgeFactory.h"

namespace fl {

    FactoryManager FactoryManager::_instance; 

    FactoryManager* FactoryManager::instance() {
        static bool initialized = false;
        if (not initialized) {
            _instance.setTnorm(new TNormFactory);
            _instance.setSnorm(new SNormFactory);
            _instance.setDefuzzifier(new DefuzzifierFactory);
            _instance.setTerm(new TermFactory);
            _instance.setHedge(new HedgeFactory);
            initialized = true;
        }
        return &_instance;
    }

    FactoryManager::FactoryManager(TNormFactory* tnorm, SNormFactory* snorm,
            DefuzzifierFactory* defuzzifier, TermFactory* term,
            HedgeFactory* hedge) :
    _tnorm(tnorm), _snorm(snorm), _defuzzifier(defuzzifier), _term(term), _hedge(hedge) {
    }

    FactoryManager::~FactoryManager() {
        if (_hedge) delete _hedge;
        if (_term) delete _term;
        if (_defuzzifier) delete _defuzzifier;
        if (_snorm) delete _snorm;
        if (_tnorm) delete _tnorm;
    }

    void FactoryManager::setTnorm(TNormFactory* tnorm) {
        if (this->_tnorm) delete this->_tnorm;
        this->_tnorm = tnorm;
    }

    TNormFactory* FactoryManager::tnorm() const {
        return this->_tnorm;
    }

    void FactoryManager::setSnorm(SNormFactory* snorm) {
        if (this->_snorm) delete this->_snorm;
        this->_snorm = snorm;
    }

    SNormFactory* FactoryManager::snorm() const {
        return this->_snorm;
    }

    void FactoryManager::setDefuzzifier(DefuzzifierFactory* defuzzifier) {
        if (this->_defuzzifier) delete this->_defuzzifier;
        this->_defuzzifier = defuzzifier;
    }

    DefuzzifierFactory* FactoryManager::defuzzifier() const {
        return this->_defuzzifier;
    }

    void FactoryManager::setTerm(TermFactory* term) {
        if (this->_term) delete this->_term;
        this->_term = term;
    }

    TermFactory* FactoryManager::term() const {
        return this->_term;
    }

    void FactoryManager::setHedge(HedgeFactory* hedge) {
        if (this->_hedge) delete this->_hedge;
        this->_hedge = hedge;
    }

    HedgeFactory* FactoryManager::hedge() const {
        return this->_hedge;
    }

}
