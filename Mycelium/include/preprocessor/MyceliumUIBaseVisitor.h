
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MyceliumUIVisitor.h"


/**
 * This class provides an empty implementation of MyceliumUIVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  MyceliumUIBaseVisitor : public MyceliumUIVisitor {
public:

  virtual std::any visitProgram(MyceliumUIParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefinition(MyceliumUIParser::DefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComponentDefinition(MyceliumUIParser::ComponentDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComponentMember(MyceliumUIParser::ComponentMemberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPropDecl(MyceliumUIParser::PropDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGetPropDecl(MyceliumUIParser::GetPropDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStateBlock(MyceliumUIParser::StateBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStateVarDecl(MyceliumUIParser::StateVarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEventDecl(MyceliumUIParser::EventDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParamList(MyceliumUIParser::ParamListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParam(MyceliumUIParser::ParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleBlock(MyceliumUIParser::StyleBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScriptBlock(MyceliumUIParser::ScriptBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleOverride(MyceliumUIParser::StyleOverrideContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleAssignment(MyceliumUIParser::StyleAssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleIdentifier(MyceliumUIParser::StyleIdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectExpression(MyceliumUIParser::SelectExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectCase(MyceliumUIParser::SelectCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefaultCase(MyceliumUIParser::DefaultCaseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCalculateBlock(MyceliumUIParser::CalculateBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMainFunction(MyceliumUIParser::MainFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProgramStatement(MyceliumUIParser::ProgramStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInstanceMember(MyceliumUIParser::InstanceMemberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPropAssignment(MyceliumUIParser::PropAssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEventHandler(MyceliumUIParser::EventHandlerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionLiteral(MyceliumUIParser::FunctionLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComparisonExpr(MyceliumUIParser::ComparisonExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectExpr(MyceliumUIParser::SelectExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqualityExpr(MyceliumUIParser::EqualityExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulDivExpr(MyceliumUIParser::MulDivExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStaticCallExpr(MyceliumUIParser::StaticCallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCalculateExpr(MyceliumUIParser::CalculateExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryExpr(MyceliumUIParser::PrimaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenExpr(MyceliumUIParser::ParenExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddSubExpr(MyceliumUIParser::AddSubExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitThisExpr(MyceliumUIParser::ThisExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTernaryExpr(MyceliumUIParser::TernaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimary(MyceliumUIParser::PrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgumentList(MyceliumUIParser::ArgumentListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral(MyceliumUIParser::LiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBooleanLiteral(MyceliumUIParser::BooleanLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifier(MyceliumUIParser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(MyceliumUIParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitScriptBody(MyceliumUIParser::ScriptBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCodeItem(MyceliumUIParser::CodeItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLengthLiteral(MyceliumUIParser::LengthLiteralContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTimeLiteral(MyceliumUIParser::TimeLiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

