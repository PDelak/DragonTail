#pragma once

#include "nullvisitor.h"

struct SemanticChecker : public NullVisitor
{
	void visitPost(const Expression* expr)
    	{
	}
};

