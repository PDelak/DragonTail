#pragma once

#include "ast.h"

auto emitMachineCode(const StatementList& statementList)
{
    X86InstrVector i_vector;
    i_vector.push_function_prolog();

    i_vector.push_function_epilog();
    JitCompiler jit(i_vector);
    return jit.compile();
}

