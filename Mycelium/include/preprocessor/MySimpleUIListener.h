
// Generated from ./MySimpleUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MySimpleUIParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by MySimpleUIParser.
 */
class  MySimpleUIListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterProgram(MySimpleUIParser::ProgramContext *ctx) = 0;
  virtual void exitProgram(MySimpleUIParser::ProgramContext *ctx) = 0;

  virtual void enterStateDecl(MySimpleUIParser::StateDeclContext *ctx) = 0;
  virtual void exitStateDecl(MySimpleUIParser::StateDeclContext *ctx) = 0;

  virtual void enterRootComponent(MySimpleUIParser::RootComponentContext *ctx) = 0;
  virtual void exitRootComponent(MySimpleUIParser::RootComponentContext *ctx) = 0;

  virtual void enterComponentCall(MySimpleUIParser::ComponentCallContext *ctx) = 0;
  virtual void exitComponentCall(MySimpleUIParser::ComponentCallContext *ctx) = 0;

  virtual void enterParameterList(MySimpleUIParser::ParameterListContext *ctx) = 0;
  virtual void exitParameterList(MySimpleUIParser::ParameterListContext *ctx) = 0;

  virtual void enterParameter(MySimpleUIParser::ParameterContext *ctx) = 0;
  virtual void exitParameter(MySimpleUIParser::ParameterContext *ctx) = 0;

  virtual void enterElement(MySimpleUIParser::ElementContext *ctx) = 0;
  virtual void exitElement(MySimpleUIParser::ElementContext *ctx) = 0;

  virtual void enterPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext *ctx) = 0;
  virtual void exitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext *ctx) = 0;

  virtual void enterExpression(MySimpleUIParser::ExpressionContext *ctx) = 0;
  virtual void exitExpression(MySimpleUIParser::ExpressionContext *ctx) = 0;

  virtual void enterLiteral(MySimpleUIParser::LiteralContext *ctx) = 0;
  virtual void exitLiteral(MySimpleUIParser::LiteralContext *ctx) = 0;

  virtual void enterIdentifier(MySimpleUIParser::IdentifierContext *ctx) = 0;
  virtual void exitIdentifier(MySimpleUIParser::IdentifierContext *ctx) = 0;


};

