
// Generated from ./MySimpleUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MySimpleUIParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by MySimpleUIParser.
 */
class  MySimpleUIVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by MySimpleUIParser.
   */
    virtual std::any visitProgram(MySimpleUIParser::ProgramContext *context) = 0;

    virtual std::any visitStateDecl(MySimpleUIParser::StateDeclContext *context) = 0;

    virtual std::any visitRootComponent(MySimpleUIParser::RootComponentContext *context) = 0;

    virtual std::any visitComponentCall(MySimpleUIParser::ComponentCallContext *context) = 0;

    virtual std::any visitParameterList(MySimpleUIParser::ParameterListContext *context) = 0;

    virtual std::any visitParameter(MySimpleUIParser::ParameterContext *context) = 0;

    virtual std::any visitElement(MySimpleUIParser::ElementContext *context) = 0;

    virtual std::any visitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext *context) = 0;

    virtual std::any visitExpression(MySimpleUIParser::ExpressionContext *context) = 0;

    virtual std::any visitLiteral(MySimpleUIParser::LiteralContext *context) = 0;

    virtual std::any visitIdentifier(MySimpleUIParser::IdentifierContext *context) = 0;


};

