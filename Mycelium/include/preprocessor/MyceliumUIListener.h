
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MyceliumUIParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by MyceliumUIParser.
 */
class  MyceliumUIListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterProgram(MyceliumUIParser::ProgramContext *ctx) = 0;
  virtual void exitProgram(MyceliumUIParser::ProgramContext *ctx) = 0;

  virtual void enterDefinition(MyceliumUIParser::DefinitionContext *ctx) = 0;
  virtual void exitDefinition(MyceliumUIParser::DefinitionContext *ctx) = 0;

  virtual void enterComponentDefinition(MyceliumUIParser::ComponentDefinitionContext *ctx) = 0;
  virtual void exitComponentDefinition(MyceliumUIParser::ComponentDefinitionContext *ctx) = 0;

  virtual void enterComponentMember(MyceliumUIParser::ComponentMemberContext *ctx) = 0;
  virtual void exitComponentMember(MyceliumUIParser::ComponentMemberContext *ctx) = 0;

  virtual void enterPropDecl(MyceliumUIParser::PropDeclContext *ctx) = 0;
  virtual void exitPropDecl(MyceliumUIParser::PropDeclContext *ctx) = 0;

  virtual void enterGetPropDecl(MyceliumUIParser::GetPropDeclContext *ctx) = 0;
  virtual void exitGetPropDecl(MyceliumUIParser::GetPropDeclContext *ctx) = 0;

  virtual void enterStateBlock(MyceliumUIParser::StateBlockContext *ctx) = 0;
  virtual void exitStateBlock(MyceliumUIParser::StateBlockContext *ctx) = 0;

  virtual void enterStateVarDecl(MyceliumUIParser::StateVarDeclContext *ctx) = 0;
  virtual void exitStateVarDecl(MyceliumUIParser::StateVarDeclContext *ctx) = 0;

  virtual void enterEventDecl(MyceliumUIParser::EventDeclContext *ctx) = 0;
  virtual void exitEventDecl(MyceliumUIParser::EventDeclContext *ctx) = 0;

  virtual void enterParamList(MyceliumUIParser::ParamListContext *ctx) = 0;
  virtual void exitParamList(MyceliumUIParser::ParamListContext *ctx) = 0;

  virtual void enterParam(MyceliumUIParser::ParamContext *ctx) = 0;
  virtual void exitParam(MyceliumUIParser::ParamContext *ctx) = 0;

  virtual void enterStyleBlock(MyceliumUIParser::StyleBlockContext *ctx) = 0;
  virtual void exitStyleBlock(MyceliumUIParser::StyleBlockContext *ctx) = 0;

  virtual void enterScriptBlock(MyceliumUIParser::ScriptBlockContext *ctx) = 0;
  virtual void exitScriptBlock(MyceliumUIParser::ScriptBlockContext *ctx) = 0;

  virtual void enterStyleOverride(MyceliumUIParser::StyleOverrideContext *ctx) = 0;
  virtual void exitStyleOverride(MyceliumUIParser::StyleOverrideContext *ctx) = 0;

  virtual void enterStyleAssignment(MyceliumUIParser::StyleAssignmentContext *ctx) = 0;
  virtual void exitStyleAssignment(MyceliumUIParser::StyleAssignmentContext *ctx) = 0;

  virtual void enterStyleIdentifier(MyceliumUIParser::StyleIdentifierContext *ctx) = 0;
  virtual void exitStyleIdentifier(MyceliumUIParser::StyleIdentifierContext *ctx) = 0;

  virtual void enterStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext *ctx) = 0;
  virtual void exitStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext *ctx) = 0;

  virtual void enterStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext *ctx) = 0;
  virtual void exitStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext *ctx) = 0;

  virtual void enterSelectExpression(MyceliumUIParser::SelectExpressionContext *ctx) = 0;
  virtual void exitSelectExpression(MyceliumUIParser::SelectExpressionContext *ctx) = 0;

  virtual void enterSelectCase(MyceliumUIParser::SelectCaseContext *ctx) = 0;
  virtual void exitSelectCase(MyceliumUIParser::SelectCaseContext *ctx) = 0;

  virtual void enterDefaultCase(MyceliumUIParser::DefaultCaseContext *ctx) = 0;
  virtual void exitDefaultCase(MyceliumUIParser::DefaultCaseContext *ctx) = 0;

  virtual void enterCalculateBlock(MyceliumUIParser::CalculateBlockContext *ctx) = 0;
  virtual void exitCalculateBlock(MyceliumUIParser::CalculateBlockContext *ctx) = 0;

  virtual void enterMainFunction(MyceliumUIParser::MainFunctionContext *ctx) = 0;
  virtual void exitMainFunction(MyceliumUIParser::MainFunctionContext *ctx) = 0;

  virtual void enterProgramStatement(MyceliumUIParser::ProgramStatementContext *ctx) = 0;
  virtual void exitProgramStatement(MyceliumUIParser::ProgramStatementContext *ctx) = 0;

  virtual void enterInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext *ctx) = 0;
  virtual void exitInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext *ctx) = 0;

  virtual void enterInstanceMember(MyceliumUIParser::InstanceMemberContext *ctx) = 0;
  virtual void exitInstanceMember(MyceliumUIParser::InstanceMemberContext *ctx) = 0;

  virtual void enterPropAssignment(MyceliumUIParser::PropAssignmentContext *ctx) = 0;
  virtual void exitPropAssignment(MyceliumUIParser::PropAssignmentContext *ctx) = 0;

  virtual void enterEventHandler(MyceliumUIParser::EventHandlerContext *ctx) = 0;
  virtual void exitEventHandler(MyceliumUIParser::EventHandlerContext *ctx) = 0;

  virtual void enterFunctionLiteral(MyceliumUIParser::FunctionLiteralContext *ctx) = 0;
  virtual void exitFunctionLiteral(MyceliumUIParser::FunctionLiteralContext *ctx) = 0;

  virtual void enterFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext *ctx) = 0;
  virtual void exitFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext *ctx) = 0;

  virtual void enterComparisonExpr(MyceliumUIParser::ComparisonExprContext *ctx) = 0;
  virtual void exitComparisonExpr(MyceliumUIParser::ComparisonExprContext *ctx) = 0;

  virtual void enterSelectExpr(MyceliumUIParser::SelectExprContext *ctx) = 0;
  virtual void exitSelectExpr(MyceliumUIParser::SelectExprContext *ctx) = 0;

  virtual void enterLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext *ctx) = 0;
  virtual void exitLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext *ctx) = 0;

  virtual void enterLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext *ctx) = 0;
  virtual void exitLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext *ctx) = 0;

  virtual void enterLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext *ctx) = 0;
  virtual void exitLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext *ctx) = 0;

  virtual void enterEqualityExpr(MyceliumUIParser::EqualityExprContext *ctx) = 0;
  virtual void exitEqualityExpr(MyceliumUIParser::EqualityExprContext *ctx) = 0;

  virtual void enterMulDivExpr(MyceliumUIParser::MulDivExprContext *ctx) = 0;
  virtual void exitMulDivExpr(MyceliumUIParser::MulDivExprContext *ctx) = 0;

  virtual void enterFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext *ctx) = 0;
  virtual void exitFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext *ctx) = 0;

  virtual void enterStaticCallExpr(MyceliumUIParser::StaticCallExprContext *ctx) = 0;
  virtual void exitStaticCallExpr(MyceliumUIParser::StaticCallExprContext *ctx) = 0;

  virtual void enterCalculateExpr(MyceliumUIParser::CalculateExprContext *ctx) = 0;
  virtual void exitCalculateExpr(MyceliumUIParser::CalculateExprContext *ctx) = 0;

  virtual void enterPrimaryExpr(MyceliumUIParser::PrimaryExprContext *ctx) = 0;
  virtual void exitPrimaryExpr(MyceliumUIParser::PrimaryExprContext *ctx) = 0;

  virtual void enterParenExpr(MyceliumUIParser::ParenExprContext *ctx) = 0;
  virtual void exitParenExpr(MyceliumUIParser::ParenExprContext *ctx) = 0;

  virtual void enterMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext *ctx) = 0;
  virtual void exitMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext *ctx) = 0;

  virtual void enterAddSubExpr(MyceliumUIParser::AddSubExprContext *ctx) = 0;
  virtual void exitAddSubExpr(MyceliumUIParser::AddSubExprContext *ctx) = 0;

  virtual void enterThisExpr(MyceliumUIParser::ThisExprContext *ctx) = 0;
  virtual void exitThisExpr(MyceliumUIParser::ThisExprContext *ctx) = 0;

  virtual void enterTernaryExpr(MyceliumUIParser::TernaryExprContext *ctx) = 0;
  virtual void exitTernaryExpr(MyceliumUIParser::TernaryExprContext *ctx) = 0;

  virtual void enterPrimary(MyceliumUIParser::PrimaryContext *ctx) = 0;
  virtual void exitPrimary(MyceliumUIParser::PrimaryContext *ctx) = 0;

  virtual void enterArgumentList(MyceliumUIParser::ArgumentListContext *ctx) = 0;
  virtual void exitArgumentList(MyceliumUIParser::ArgumentListContext *ctx) = 0;

  virtual void enterLiteral(MyceliumUIParser::LiteralContext *ctx) = 0;
  virtual void exitLiteral(MyceliumUIParser::LiteralContext *ctx) = 0;

  virtual void enterBooleanLiteral(MyceliumUIParser::BooleanLiteralContext *ctx) = 0;
  virtual void exitBooleanLiteral(MyceliumUIParser::BooleanLiteralContext *ctx) = 0;

  virtual void enterIdentifier(MyceliumUIParser::IdentifierContext *ctx) = 0;
  virtual void exitIdentifier(MyceliumUIParser::IdentifierContext *ctx) = 0;

  virtual void enterType(MyceliumUIParser::TypeContext *ctx) = 0;
  virtual void exitType(MyceliumUIParser::TypeContext *ctx) = 0;

  virtual void enterScriptBody(MyceliumUIParser::ScriptBodyContext *ctx) = 0;
  virtual void exitScriptBody(MyceliumUIParser::ScriptBodyContext *ctx) = 0;

  virtual void enterCodeItem(MyceliumUIParser::CodeItemContext *ctx) = 0;
  virtual void exitCodeItem(MyceliumUIParser::CodeItemContext *ctx) = 0;

  virtual void enterLengthLiteral(MyceliumUIParser::LengthLiteralContext *ctx) = 0;
  virtual void exitLengthLiteral(MyceliumUIParser::LengthLiteralContext *ctx) = 0;

  virtual void enterTimeLiteral(MyceliumUIParser::TimeLiteralContext *ctx) = 0;
  virtual void exitTimeLiteral(MyceliumUIParser::TimeLiteralContext *ctx) = 0;


};

