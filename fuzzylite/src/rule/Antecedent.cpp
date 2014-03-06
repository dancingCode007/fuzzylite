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
 * Antecedent.cpp
 *
 *  Created on: 3/12/2012
 *      Author: jcrada
 */

#include "fl/rule/Antecedent.h"

#include "fl/rule/Expression.h"

#include "fl/Engine.h"
#include "fl/variable/InputVariable.h"
#include "fl/hedge/Hedge.h"
#include "fl/hedge/Any.h"
#include "fl/term/Term.h"

#include "fl/rule/Rule.h"
#include "fl/norm/TNorm.h"
#include "fl/norm/SNorm.h"

#include "fl/term/Function.h"

#include "fl/factory/FactoryManager.h"
#include "fl/factory/HedgeFactory.h"

#include <stack>
#include <algorithm>

namespace fl {

    Antecedent::Antecedent()
    : _text(""), _expression(NULL) {
    }

    Antecedent::~Antecedent() {
        unload();
    }

    void Antecedent::setText(const std::string& text) {
        this->_text = text;
    }

    std::string Antecedent::getText() const {
        return this->_text;
    }

    Expression* Antecedent::getExpression() const {
        return this->_expression;
    }

    bool Antecedent::isLoaded() const {
        return this->_expression != NULL;
    }

    scalar Antecedent::activationDegree(const TNorm* conjunction, const SNorm* disjunction,
            const Expression* node) const {
        if (not isLoaded()) {
            throw fl::Exception("[antecedent error] antecedent <" + _text + "> is not loaded", FL_AT);
        }
        const Proposition* proposition = dynamic_cast<const Proposition*> (node);
        if (proposition) {
            if (not proposition->variable->isEnabled()) {
                return 0.0;
            }


            if (not proposition->hedges.empty()) {
                //if last hedge is "Any", apply hedges in reverse order and return degree
                std::vector<Hedge*>::const_reverse_iterator rit = proposition->hedges.rbegin();
                if (dynamic_cast<Any*> (*rit)) {
                    scalar result = (*rit)->hedge(fl::nan);
                    while (++rit != proposition->hedges.rend()) {
                        result = (*rit)->hedge(result);
                    }
                    return result;
                }
            }

            InputVariable* inputVariable = dynamic_cast<InputVariable*> (proposition->variable);
            scalar result = proposition->term->membership(inputVariable->getInputValue());
            for (std::vector<Hedge*>::const_reverse_iterator rit = proposition->hedges.rbegin();
                    rit != proposition->hedges.rend(); ++rit) {
                result = (*rit)->hedge(result);
            }
            return result;
        }
        //if node is an operatorsk
        const Operator* fuzzyOperator = dynamic_cast<const Operator*> (node);
        if (not (fuzzyOperator->left and fuzzyOperator->right)) {
            std::ostringstream ex;
            ex << "[syntax error] left and right operands must exist";
            throw fl::Exception(ex.str(), FL_AT);
        }
        if (fuzzyOperator->name == Rule::FL_AND)
            return conjunction->compute(
                this->activationDegree(conjunction, disjunction, fuzzyOperator->left),
                this->activationDegree(conjunction, disjunction, fuzzyOperator->right));

        if (fuzzyOperator->name == Rule::FL_OR)
            return disjunction->compute(
                this->activationDegree(conjunction, disjunction, fuzzyOperator->left),
                this->activationDegree(conjunction, disjunction, fuzzyOperator->right));
        std::ostringstream ex;
        ex << "[syntax error] operator <" << fuzzyOperator->name << "> not recognized";
        throw fl::Exception(ex.str(), FL_AT);

    }

    scalar Antecedent::activationDegree(const TNorm* conjunction, const SNorm* disjunction) const {
        return this->activationDegree(conjunction, disjunction, this->_expression);
    }

    void Antecedent::unload() {
        if (_expression) {
            delete _expression;
            _expression = NULL;
        }
    }

    void Antecedent::load(fl::Rule* rule, const Engine* engine) {
        load(_text, rule, engine);
    }

    void Antecedent::load(const std::string& antecedent, fl::Rule* rule, const Engine* engine) {
        unload();
        this->_text = antecedent;
        if (fl::Op::trim(antecedent).empty()){
            throw fl::Exception("[syntax error] antecedent is empty", FL_AT);
        }
        /*
         Builds an proposition tree from the antecedent of a fuzzy rule.
         The rules are:
         1) After a variable comes 'is',
         2) After 'is' comes a hedge or a term
         3) After a hedge comes a hedge or a term
         4) After a term comes a variable or an operator
         */

        Function function;

        std::string postfix = function.toPostfix(antecedent);
        std::stringstream tokenizer(postfix);
        std::string token;

        enum FSM {
            S_VARIABLE = 1, S_IS = 2, S_HEDGE = 4, S_TERM = 8, S_AND_OR = 16
        };
        int state = S_VARIABLE;
        std::stack<Expression*> expressionStack;
        Proposition* proposition = NULL;
        try {
            while (tokenizer >> token) {
                if (state bitand S_VARIABLE) {
                    if (engine->hasInputVariable(token)) {
                        proposition = new Proposition;
                        proposition->variable = engine->getInputVariable(token);
                        expressionStack.push(proposition);

                        state = S_IS;
                        continue;
                    }
                }

                if (state bitand S_IS) {
                    if (token == Rule::FL_IS) {
                        state = S_HEDGE bitor S_TERM;
                        continue;
                    }
                }

                if (state bitand S_HEDGE) {
                    Hedge* hedge = NULL;
                    if (rule->hasHedge(token)) {
                        hedge = rule->getHedge(token);
                    } else {
                        std::vector<std::string> hedges = FactoryManager::instance()->hedge()->available();
                        if (std::find(hedges.begin(), hedges.end(), token) != hedges.end()) {
                            hedge = FactoryManager::instance()->hedge()->createInstance(token);
                            rule->addHedge(hedge);
                        }
                    }
                    if (hedge) {
                        proposition->hedges.push_back(hedge);
                        if (dynamic_cast<Any*> (hedge)) {
                            state = S_VARIABLE bitor S_AND_OR;
                        } else {
                            state = S_HEDGE bitor S_TERM;
                        }
                        continue;
                    }
                }

                if (state bitand S_TERM) {
                    if (proposition->variable->hasTerm(token)) {
                        proposition->term = proposition->variable->getTerm(token);
                        state = S_VARIABLE bitor S_AND_OR;
                        continue;
                    }
                }

                if (state bitand S_AND_OR) {
                    if (token == Rule::FL_AND or token == Rule::FL_OR) {
                        if (expressionStack.size() < 2) {
                            std::ostringstream ex;
                            ex << "[syntax error] logical operator <" << token << "> expects two operands,"
                                    << "but found <" << expressionStack.size() << "> in antecedent";
                            throw fl::Exception(ex.str(), FL_AT);
                        }
                        Operator* fuzzyOperator = new Operator;
                        fuzzyOperator->name = token;
                        fuzzyOperator->right = expressionStack.top();
                        expressionStack.pop();
                        fuzzyOperator->left = expressionStack.top();
                        expressionStack.pop();
                        expressionStack.push(fuzzyOperator);

                        state = S_VARIABLE bitor S_AND_OR;
                        continue;
                    }
                }

                //If reached this point, there was an error
                if ((state bitand S_VARIABLE) or (state bitand S_AND_OR)) {
                    std::ostringstream ex;
                    ex << "[syntax error] antecedent expected input variable or logical operator, but found <" << token << ">";
                    throw fl::Exception(ex.str(), FL_AT);
                }
                if (state bitand S_IS) {
                    std::ostringstream ex;
                    ex << "[syntax error] antecedent expected keyword <" << Rule::FL_IS << ">, but found <" << token << ">";
                    throw fl::Exception(ex.str(), FL_AT);
                }
                if ((state bitand S_HEDGE) or (state bitand S_TERM)) {
                    std::ostringstream ex;
                    ex << "[syntax error] antecedent expected hedge or term, but found <" << token << ">";
                    throw fl::Exception(ex.str(), FL_AT);
                }
                std::ostringstream ex;
                ex << "[syntax error] unexpected token <" << token << "> in antecedent";
                throw fl::Exception(ex.str(), FL_AT);
            }

            if (not ((state bitand S_VARIABLE) or (state bitand S_AND_OR))) { //only acceptable final state
                if (state bitand S_IS) {
                    std::ostringstream ex;
                    ex << "[syntax error] antecedent expected keyword <" << Rule::FL_IS << "> after <" << token << ">";
                    throw fl::Exception(ex.str(), FL_AT);
                }
                if ((state bitand S_HEDGE) or (state bitand S_TERM)) {
                    std::ostringstream ex;
                    ex << "[syntax error] antecedent expected hedge or term after <" << token << ">";
                    throw fl::Exception(ex.str(), FL_AT);
                }
            }

            if (expressionStack.size() != 1) {
                std::vector<std::string> errors;
                while (expressionStack.size() > 1) {
                    Expression* expression = expressionStack.top();
                    expressionStack.pop();
                    errors.push_back(expression->toString());
                    delete expression;
                }
                std::ostringstream ex;
                ex << "[syntax error] unable to parse the following expressions in antecedent <"
                        << Op::join(errors, " ") << ">";
                throw fl::Exception(ex.str(), FL_AT);
            }
        } catch (std::exception& ex) {
			(void)ex;
            for (std::size_t i = 0; i < expressionStack.size(); ++i) {
                delete expressionStack.top();
                expressionStack.pop();
            }
            throw;
        }
        this->_expression = expressionStack.top();
    }

    std::string Antecedent::toString() const {
        return toInfix(this->_expression);
    }

    std::string Antecedent::toPrefix(const Expression* node) const {
        if (not isLoaded()) {
            throw fl::Exception("[antecedent error] antecedent <" + _text + "> is not loaded", FL_AT);
        }
        if (not node) node = this->_expression;

        if (dynamic_cast<const Proposition*> (node)) {
            return node->toString();
        }
        const Operator* fuzzyOperator =
                dynamic_cast<const Operator*> (node);
        std::stringstream ss;
        ss << fuzzyOperator->toString() << " "
                << toPrefix(fuzzyOperator->left) << " "
                << toPrefix(fuzzyOperator->right) << " ";
        return ss.str();
    }

    std::string Antecedent::toInfix(const Expression* node) const {
        if (not isLoaded()) {
            throw fl::Exception("[antecedent error] antecedent <" + _text + "> is not loaded", FL_AT);
        }
        if (not node) node = this->_expression;
        if (dynamic_cast<const Proposition*> (node)) {
            return node->toString();
        }
        const Operator* fuzzyOperator =
                dynamic_cast<const Operator*> (node);
        std::stringstream ss;
        ss << toInfix(fuzzyOperator->left) << " "
                << fuzzyOperator->toString() << " "
                << toInfix(fuzzyOperator->right) << " ";
        return ss.str();
    }

    std::string Antecedent::toPostfix(const Expression* node) const {
        if (not isLoaded()) {
            throw fl::Exception("[antecedent error] antecedent <" + _text + "> is not loaded", FL_AT);
        }
        if (not node) node = this->_expression;
        if (dynamic_cast<const Proposition*> (node)) {
            return node->toString();
        }
        const Operator* fuzzyOperator =
                dynamic_cast<const Operator*> (node);
        std::stringstream ss;
        ss << toPostfix(fuzzyOperator->left) << " "
                << toPostfix(fuzzyOperator->right) << " "
                << fuzzyOperator->toString() << " ";
        return ss.str();
    }


}
