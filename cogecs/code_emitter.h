#ifndef MACHINE_CODE_EMITTER_H
#define MACHINE_CODE_EMITTER_H

#include "ast.h"

auto emit_machine_code(std::vector<int>* v, const StatementList& statementList)
{
    X86InstrVector i_vector;
    i_vector.push_function_prolog();

    i_vector.push_function_epilog();
    JitCompiler jit(i_vector);
    return jit.compile();
}

#endif
