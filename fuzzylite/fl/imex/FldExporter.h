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
 * File:   FldExporter.h
 * Author: jcrada
 *
 * Created on 30 November 2013, 9:52 AM
 */

#ifndef FL_FLDEXPORTER_H
#define	FL_FLDEXPORTER_H

#include "fl/fuzzylite.h"
#include "fl/imex/Exporter.h"

#include <vector>

namespace fl {
    class Engine;
    class InputVariable;
    class OutputVariable;

    class FL_EXPORT FldExporter : public Exporter {
    protected:
        std::string _separator;
    public:
        FldExporter(const std::string& separator = " ");
        virtual ~FldExporter();

        virtual std::string name() const;

        virtual void setSeparator(const std::string& separator);
        virtual std::string getSeparator() const;

        virtual std::string header(const Engine* engine) const;
        virtual std::string header(const std::vector<InputVariable*>& inputVariables) const;
        virtual std::string header(const std::vector<OutputVariable*>& outputVariables) const;


        //WARNING: The engine will be const_casted in order to be processed!
        virtual std::string toString(const Engine* mutableEngine) const;
        virtual std::string toString(const Engine* mutableEngine, int maximumNumberOfResults,
                bool includeHeaders = true, bool includeInputValues = true) const;
        virtual std::string toString(const Engine* mutableEngine, const std::string& inputData,
                bool includeHeaders = true, bool includeInputValues = true) const;

        template<typename T>
        void toWriter(Engine* engine, T& writer, int maximumNumberOfResults,
                const std::string& separator, bool includeInputValues = true) const;

        virtual void parse(const std::string& x, std::vector<scalar>& inputValues) const;

        template<typename T>
        void toWriter(Engine* engine, T& writer, const std::vector<scalar>& inputValues,
                const std::string& separator, bool includeInputValues = true) const;


    };
}

#endif	/* FL_FLDEXPORTER_H */

