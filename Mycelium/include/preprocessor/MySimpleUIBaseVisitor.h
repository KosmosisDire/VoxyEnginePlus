
// Generated from MySimpleUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MySimpleUIVisitor.h"


/**
 * This class provides an empty implementation of MySimpleUIVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  MySimpleUIBaseVisitor : public MySimpleUIVisitor {
public:

  virtual std::any visitProgram(MySimpleUIParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStateDecl(MySimpleUIParser::StateDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRootComponent(MySimpleUIParser::RootComponentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComponentCall(MySimpleUIParser::ComponentCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameterList(MySimpleUIParser::ParameterListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParameter(MySimpleUIParser::ParameterContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitElement(MySimpleUIParser::ElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(MySimpleUIParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(MySimpleUIParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifier(MySimpleUIParser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }


};

