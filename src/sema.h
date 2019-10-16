#pragma once

#include <string>
#include <sstream>
#include "nullvisitor.h"

struct CodeEmitterException : public std::runtime_error
{
    CodeEmitterException(const std::string& msg):std::runtime_error(msg) {}
};


struct SemanticChecker : public NullVisitor
{
	void visitPost(const Expression* expr)
    	{
		auto children = expr->getChilds();
        	switch(children.size())
        	{
            		case 3: {
                		auto op = cast<BasicExpression>(children[1]);
                		if (op->value != "=")
                		{
		                	std::string errMessage = "expression is noop operation : ";
                    			std::stringstream outStream;
                    			for(const auto child : expr->getChilds())
                    			{
                        			child->text(outStream);
                    			}
                    			errMessage += outStream.str();
                    			throw CodeEmitterException(errMessage);
	                	}

			}
		}
	}
};

