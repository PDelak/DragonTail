#pragma once

// Control Flow Flattening is first transformation phase 
// where compound statements are flattening to be used
// for low level or machine code generation

#include <stack>
#include <vector>
#include <cassert>
#include "astvisitor.h"
#include "ast.h"
#include "tools.h"

struct CFGFlattener : public AstVisitor
{   
    CFGFlattener()  
    {
        std::string label = getNextLabel();     
        statements.push_back(makeNode(Expression(scope, { makeNode(BasicExpression(scope,"__alloc__")) })));
    }
    ~CFGFlattener()  
    { 
        assert(nodesStack.empty());
    }

    void visitPre(const BasicStatement*) {}
    void visitPre(const BasicExpression*) {}
    void visitPre(const FunctionCall*) {}
    void visitPost(const BasicStatement*) {}
    void visitPost(const BasicExpression*) {}
    void visitPost(const FunctionCall*) {}

    void visitPre(const VarDecl* stmt)
    {
        nodesStack.push_back(makeNode(VarDecl(scope, stmt->var_name)));
    }
    
    void visitPre(const Expression* stmt)
    {
        nodesStack.push_back(makeNode(Expression(scope, stmt->getChilds(), stmt->isPartOfCompoundStmt)));

    }
    void visitPre(const IfStatement*)
    {
        nodesStack.push_back(makeNode(IfStatement(scope)));
    }

    void visitPre(const WhileLoop*)
    {
        nodesStack.push_back(makeNode(WhileLoop(scope)));
    }

    void visitPre(const BlockStatement*)
    {
        nodesStack.push_back(makeNode(BlockStatement(scope++)));
    }

    void visitPre(const LabelStatement* stmt)
    {
        nodesStack.push_back(makeNode(LabelStatement(scope, stmt->label)));
    }

    void visitPre(const GotoStatement* stmt) 
    {
        nodesStack.push_back(makeNode(GotoStatement(scope, stmt->label)));
    }

    void visitPre(const FunctionDecl* fdecl) 
    {
        // TODO : this is shallow copy for now
        nodesStack.push_back(makeNode(FunctionDecl(scope, fdecl->name, fdecl->parameters, fdecl->statements)));
    }

    void visitPre(const ReturnStatement* stmt) 
    {
        nodesStack.push_back(makeNode(ReturnStatement(scope, stmt->param)));
    }

    std::string getNextLabel() 
    {
        std::string label = "label__";
        label.append(std::to_string(id));
        id++;
        return label;
    }

    std::string getNextTempVariable()
    {
        std::string label = "temp__";
        label.append(std::to_string(id));
        id++;
        return label;
    }

    void visitPost(const IfStatement*)
    {
        if (nodesStack.empty()) return;
        auto begin = nodesStack.rbegin();
        auto if_statement = *begin;
        nodesStack.erase(std::next(begin).base());

        auto currentStatementIterator = statements.rbegin();
        // get statement that is connected to if
        
        std::vector<StatementPtr> if_children;
        if (is<BlockStatement>(*currentStatementIterator)) {
            auto block = cast<BlockStatement>(*currentStatementIterator);
            std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
            std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(if_children));
        }
        else {
            if_children.push_back(*currentStatementIterator);
        }

        ++currentStatementIterator;
        
        if (!is<Expression>(*currentStatementIterator)) {
            // handle label statement, that's follow case
            // if (a) label_0: {}
            if_children.insert(if_children.begin(), *currentStatementIterator);
            ++currentStatementIterator;
        }

        Expression condition;
        condition.setElements(cast<Expression>(*currentStatementIterator)->getChilds());
        cast<IfStatement>(if_statement)->condition.isPartOfCompoundStmt = cast<Expression>(*currentStatementIterator)->isPartOfCompoundStmt;
        ++currentStatementIterator;
        statements.erase(currentStatementIterator.base(), statements.end());

        std::string temp = getNextTempVariable();

        statements.push_back(makeNode(VarDecl(scope, temp)));

        // create reverse condition expression
        auto reverseCondition = makeNode(Expression(scope, {makeNode(BasicExpression(scope, temp)), 
                                                            makeNode(BasicExpression(scope, "="))}));
        
        std::copy(condition.child_begin(), condition.child_end(), 
                  std::back_inserter(static_cast<Expression*>(reverseCondition.get())->getChilds()));
        statements.push_back(reverseCondition);

        cast<IfStatement>(if_statement)->condition.insertChild(cast<IfStatement>(if_statement)->condition.child_end(), 
                                                               { makeNode(BasicExpression(scope, "!")), 
                                                                 makeNode(BasicExpression(scope, temp)) });

        std::string label = getNextLabel();

        // create goto statement
        // goto statements scope needs to be 0 as everything is flatten
        constexpr size_t gotoStatementScope = 0;
        cast<IfStatement>(if_statement)->statements.push_back(makeNode(GotoStatement(gotoStatementScope, label)));
        statements.push_back(if_statement);

        std::copy(if_children.begin(), if_children.end(), std::back_inserter(statements));

        statements.push_back(makeNode(LabelStatement(scope, label)));
    }

    void visitPost(const WhileLoop*)
    {
        
        // while loop is a kind of a compound node 
        // it can have one children which can be a basic statement
        // or a block 
        // 1) while -> ;
        // 2) while -> {}
        // below while loop 
        // while(x) {}
        // is transformed to if statement as follows
        /*
            var temp__0;
            label__1:
            temp__0=x;
            if (!temp__0) goto label__2;
            goto label__1;
            label__2:
        */
        if (nodesStack.empty()) return;
        auto begin = nodesStack.rbegin();
        nodesStack.erase(std::next(begin).base());

        auto currentStatementIterator = statements.rbegin();
        
        std::vector<StatementPtr> while_children;
        if (is<BlockStatement>(*currentStatementIterator)) {
            auto block = cast<BlockStatement>(*currentStatementIterator);
            // every left bracket '{' starts a new block and increases a scope counter
            // every right bracket '}' closes block and decreases a scope counter
            // reducing while loop means that it is transformed to if statement with reversed condition
            // and few jumps depending on condition value
            // so next for_each statement is responsible for decreasing scope counter of all statements inside a block
            std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
            std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(while_children));
        }
        else { while_children.push_back(*currentStatementIterator); }

        ++currentStatementIterator;
        
        if (!is<Expression>(*currentStatementIterator)) {
            // handle label statement, that's follow case
            // while (a) label_0: {}
            while_children.insert(while_children.begin(), *currentStatementIterator);
            ++currentStatementIterator;         
        }
        
        Expression condition = *cast<Expression>(*currentStatementIterator);
        auto if_statement = makeNode(IfStatement(scope));           
        cast<IfStatement>(if_statement)->scope = cast<Expression>(*currentStatementIterator)->scope;
        cast<IfStatement>(if_statement)->condition.isPartOfCompoundStmt = cast<Expression>(*currentStatementIterator)->isPartOfCompoundStmt;

        ++currentStatementIterator;
        statements.erase(currentStatementIterator.base(), statements.end());
        
        std::string temp = getNextTempVariable();
        statements.push_back(makeNode(VarDecl(scope, temp)));

        // create reverse condition expression      
        auto labelBeforeIfNode = makeNode(LabelStatement(scope));
        cast<LabelStatement>(labelBeforeIfNode)->label = getNextLabel();
        statements.push_back(labelBeforeIfNode);

        auto reverseCondition = makeNode(Expression(scope, { makeNode(BasicExpression(scope, temp)), 
                                                             makeNode(BasicExpression(scope, "=")) }));
        std::copy(condition.child_begin(), condition.child_end(), 
                  std::back_inserter(static_cast<Expression*>(reverseCondition.get())->getChilds()));
        
        statements.push_back(reverseCondition);
        
        cast<IfStatement>(if_statement)->condition.insertChild(cast<IfStatement>(if_statement)->condition.child_end(), 
                                                              { makeNode(BasicExpression(scope, "!")), 
                                                                makeNode(BasicExpression(scope, temp)) });

        std::string label = getNextLabel();
        // goto statements scope needs to be 0 as everything is flatten
        constexpr size_t gotoStatementScope = 0;
        cast<IfStatement>(if_statement)->statements.push_back(makeNode(GotoStatement(gotoStatementScope, label)));
        
        statements.push_back(if_statement);
        std::copy(while_children.begin(), while_children.end(), std::back_inserter(statements));
        statements.push_back(makeNode(GotoStatement(scope, cast<LabelStatement>(labelBeforeIfNode)->label)));
        statements.push_back(makeNode(LabelStatement(scope, label)));
    }
    void visitPost(const BlockStatement*)
    {       
        // reduce all blocks except most outer one
        // otherwise reduce all     
        if (nodesStack.empty()) return;
        auto begin = nodesStack.rbegin();
        auto block = *begin;
        auto previous = nodesStack.erase(std::next(begin).base());
        if (previous != nodesStack.begin()) --previous;
        if (!nodesStack.empty() && (is<IfStatement>(*previous) || is<WhileLoop>(*previous))) {
            StatementList blockStatements;
            auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
                return s->scope != scope;
            });

            // insert label just before body of compound statement (if, while loop)
            blockStatements.push_back(makeNode(Expression(scope, { makeNode(BasicExpression(scope,"__alloc__")) })));

            std::copy(it.base(), statements.end(), std::back_inserter(blockStatements));
            blockStatements.push_back(makeNode(Expression(scope, { makeNode(BasicExpression(scope,"__dealloc__")) })));

            cast<BlockStatement>(block)->statements = blockStatements;
            statements.erase(it.base(), statements.end());
            statements.push_back(block);
        }
        else {
            // if previous is block statement as well
            // update all statements that are part of current block             
            auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
                return s->scope != scope;
            });

            // We can insert a label just before any block
            // but right now it seems to be not neeeded
            // insert label just before beginning of new scope
            // std::string label = getNextLabel();
            // auto first_statement_after_label = statements.insert(it.base(), makeNode(LabelStatement(scope, label)));

            std::for_each(it.base(), statements.end(), [](const StatementPtr& stmt) {
                stmt->scope = stmt->scope - 1;
            });

        }
        --scope;
                                    
    }

    void visitPost(const VarDecl*)
    {
        popFromStackAndPushToStatements();
    }

    void visitPost(const Expression*)
    {
        popFromStackAndPushToStatements();
    }

    void visitPost(const LabelStatement*)
    {
        popFromStackAndPushToStatements();
    }

    void visitPost(const GotoStatement*)
    {
        popFromStackAndPushToStatements();
    }

    void visitPost(const ReturnStatement*) 
    {
        popFromStackAndPushToStatements();
    }

    void visitPost(const FunctionDecl*) 
    {
        if (nodesStack.empty()) return;
        auto node = *nodesStack.rbegin();
        auto load_call_expression = makeNode(Expression(scope));
        auto load_call = makeNode(FunctionCall(scope));
        std::vector<StatementPtr> load_expr_children;
        load_expr_children.push_back(load_call);
        cast<FunctionCall>(load_call)->name = "load";
        cast<Expression>(load_call_expression)->setElements(load_expr_children);
        cast<Expression>(load_call_expression)->isPartOfCompoundStmt = false;

        auto ret_call_expression = makeNode(Expression(scope));
        auto ret_call = makeNode(FunctionCall(scope));
        std::vector<StatementPtr> ret_expr_children;
        ret_expr_children.push_back(ret_call);
        cast<FunctionCall>(ret_call)->name = "ret";
        cast<Expression>(ret_call_expression)->setElements(ret_expr_children);
        cast<Expression>(ret_call_expression)->isPartOfCompoundStmt = false;

        auto label = makeNode(LabelStatement(scope));
        static_cast<LabelStatement*>(label.get())->label = static_cast<FunctionDecl*>(node.get())->name;
        
        statements.push_back(label);
        statements.push_back(load_call_expression);
        
        // traverse function block
        for (const auto& stmt : static_cast<FunctionDecl*>(node.get())->statements)
        {
            stmt->traverse(*this);
        }
        
        statements.push_back(ret_call_expression);

        nodesStack.erase(std::next(nodesStack.rbegin()).base());
    }

    StatementList getStatements() 
    {
        // TODO this is workaround 
        if (!closingDealloc) {
            statements.push_back(makeNode(Expression(scope, { makeNode(BasicExpression(scope,"__dealloc__")) })));
            closingDealloc = true;
        }
        return statements; 
    }
private:

    void popFromStackAndPushToStatements()
    {
        if (nodesStack.empty()) return;
        auto begin = nodesStack.rbegin();
        auto node = *begin;
        statements.push_back(node);
        nodesStack.erase(std::next(begin).base());
    }

    size_t scope = 0;
    StatementList statements;
    std::vector<StatementPtr> nodesStack;
    size_t id = 0;
    bool closingDealloc = false;
};
