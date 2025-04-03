
// Generated from MySimpleUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MySimpleUIListener.h"


/**
 * This class provides an empty implementation of MySimpleUIListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  MySimpleUIBaseListener : public MySimpleUIListener {
public:

  virtual void enterProgram(MySimpleUIParser::ProgramContext * /*ctx*/) override { }
  virtual void exitProgram(MySimpleUIParser::ProgramContext * /*ctx*/) override { }

  virtual void enterStateDecl(MySimpleUIParser::StateDeclContext * /*ctx*/) override { }
  virtual void exitStateDecl(MySimpleUIParser::StateDeclContext * /*ctx*/) override { }

  virtual void enterRootComponent(MySimpleUIParser::RootComponentContext * /*ctx*/) override { }
  virtual void exitRootComponent(MySimpleUIParser::RootComponentContext * /*ctx*/) override { }

  virtual void enterComponentCall(MySimpleUIParser::ComponentCallContext * /*ctx*/) override { }
  virtual void exitComponentCall(MySimpleUIParser::ComponentCallContext * /*ctx*/) override { }

  virtual void enterParameterList(MySimpleUIParser::ParameterListContext * /*ctx*/) override { }
  virtual void exitParameterList(MySimpleUIParser::ParameterListContext * /*ctx*/) override { }

  virtual void enterParameter(MySimpleUIParser::ParameterContext * /*ctx*/) override { }
  virtual void exitParameter(MySimpleUIParser::ParameterContext * /*ctx*/) override { }

  virtual void enterElement(MySimpleUIParser::ElementContext * /*ctx*/) override { }
  virtual void exitElement(MySimpleUIParser::ElementContext * /*ctx*/) override { }

  virtual void enterPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext * /*ctx*/) override { }
  virtual void exitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext * /*ctx*/) override { }

  virtual void enterExpression(MySimpleUIParser::ExpressionContext * /*ctx*/) override { }
  virtual void exitExpression(MySimpleUIParser::ExpressionContext * /*ctx*/) override { }

  virtual void enterLiteral(MySimpleUIParser::LiteralContext * /*ctx*/) override { }
  virtual void exitLiteral(MySimpleUIParser::LiteralContext * /*ctx*/) override { }

  virtual void enterIdentifier(MySimpleUIParser::IdentifierContext * /*ctx*/) override { }
  virtual void exitIdentifier(MySimpleUIParser::IdentifierContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

