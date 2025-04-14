
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MyceliumUIListener.h"


/**
 * This class provides an empty implementation of MyceliumUIListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  MyceliumUIBaseListener : public MyceliumUIListener {
public:

  virtual void enterProgram(MyceliumUIParser::ProgramContext * /*ctx*/) override { }
  virtual void exitProgram(MyceliumUIParser::ProgramContext * /*ctx*/) override { }

  virtual void enterDefinition(MyceliumUIParser::DefinitionContext * /*ctx*/) override { }
  virtual void exitDefinition(MyceliumUIParser::DefinitionContext * /*ctx*/) override { }

  virtual void enterComponentDefinition(MyceliumUIParser::ComponentDefinitionContext * /*ctx*/) override { }
  virtual void exitComponentDefinition(MyceliumUIParser::ComponentDefinitionContext * /*ctx*/) override { }

  virtual void enterComponentMember(MyceliumUIParser::ComponentMemberContext * /*ctx*/) override { }
  virtual void exitComponentMember(MyceliumUIParser::ComponentMemberContext * /*ctx*/) override { }

  virtual void enterPropDecl(MyceliumUIParser::PropDeclContext * /*ctx*/) override { }
  virtual void exitPropDecl(MyceliumUIParser::PropDeclContext * /*ctx*/) override { }

  virtual void enterGetPropDecl(MyceliumUIParser::GetPropDeclContext * /*ctx*/) override { }
  virtual void exitGetPropDecl(MyceliumUIParser::GetPropDeclContext * /*ctx*/) override { }

  virtual void enterStateBlock(MyceliumUIParser::StateBlockContext * /*ctx*/) override { }
  virtual void exitStateBlock(MyceliumUIParser::StateBlockContext * /*ctx*/) override { }

  virtual void enterStateVarDecl(MyceliumUIParser::StateVarDeclContext * /*ctx*/) override { }
  virtual void exitStateVarDecl(MyceliumUIParser::StateVarDeclContext * /*ctx*/) override { }

  virtual void enterEventDecl(MyceliumUIParser::EventDeclContext * /*ctx*/) override { }
  virtual void exitEventDecl(MyceliumUIParser::EventDeclContext * /*ctx*/) override { }

  virtual void enterParamList(MyceliumUIParser::ParamListContext * /*ctx*/) override { }
  virtual void exitParamList(MyceliumUIParser::ParamListContext * /*ctx*/) override { }

  virtual void enterParam(MyceliumUIParser::ParamContext * /*ctx*/) override { }
  virtual void exitParam(MyceliumUIParser::ParamContext * /*ctx*/) override { }

  virtual void enterStyleBlock(MyceliumUIParser::StyleBlockContext * /*ctx*/) override { }
  virtual void exitStyleBlock(MyceliumUIParser::StyleBlockContext * /*ctx*/) override { }

  virtual void enterScriptBlock(MyceliumUIParser::ScriptBlockContext * /*ctx*/) override { }
  virtual void exitScriptBlock(MyceliumUIParser::ScriptBlockContext * /*ctx*/) override { }

  virtual void enterStyleOverride(MyceliumUIParser::StyleOverrideContext * /*ctx*/) override { }
  virtual void exitStyleOverride(MyceliumUIParser::StyleOverrideContext * /*ctx*/) override { }

  virtual void enterStyleAssignment(MyceliumUIParser::StyleAssignmentContext * /*ctx*/) override { }
  virtual void exitStyleAssignment(MyceliumUIParser::StyleAssignmentContext * /*ctx*/) override { }

  virtual void enterStyleIdentifier(MyceliumUIParser::StyleIdentifierContext * /*ctx*/) override { }
  virtual void exitStyleIdentifier(MyceliumUIParser::StyleIdentifierContext * /*ctx*/) override { }

  virtual void enterStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext * /*ctx*/) override { }
  virtual void exitStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext * /*ctx*/) override { }

  virtual void enterStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext * /*ctx*/) override { }
  virtual void exitStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext * /*ctx*/) override { }

  virtual void enterSelectExpression(MyceliumUIParser::SelectExpressionContext * /*ctx*/) override { }
  virtual void exitSelectExpression(MyceliumUIParser::SelectExpressionContext * /*ctx*/) override { }

  virtual void enterSelectCase(MyceliumUIParser::SelectCaseContext * /*ctx*/) override { }
  virtual void exitSelectCase(MyceliumUIParser::SelectCaseContext * /*ctx*/) override { }

  virtual void enterDefaultCase(MyceliumUIParser::DefaultCaseContext * /*ctx*/) override { }
  virtual void exitDefaultCase(MyceliumUIParser::DefaultCaseContext * /*ctx*/) override { }

  virtual void enterCalculateBlock(MyceliumUIParser::CalculateBlockContext * /*ctx*/) override { }
  virtual void exitCalculateBlock(MyceliumUIParser::CalculateBlockContext * /*ctx*/) override { }

  virtual void enterMainFunction(MyceliumUIParser::MainFunctionContext * /*ctx*/) override { }
  virtual void exitMainFunction(MyceliumUIParser::MainFunctionContext * /*ctx*/) override { }

  virtual void enterProgramStatement(MyceliumUIParser::ProgramStatementContext * /*ctx*/) override { }
  virtual void exitProgramStatement(MyceliumUIParser::ProgramStatementContext * /*ctx*/) override { }

  virtual void enterInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext * /*ctx*/) override { }
  virtual void exitInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext * /*ctx*/) override { }

  virtual void enterInstanceMember(MyceliumUIParser::InstanceMemberContext * /*ctx*/) override { }
  virtual void exitInstanceMember(MyceliumUIParser::InstanceMemberContext * /*ctx*/) override { }

  virtual void enterPropAssignment(MyceliumUIParser::PropAssignmentContext * /*ctx*/) override { }
  virtual void exitPropAssignment(MyceliumUIParser::PropAssignmentContext * /*ctx*/) override { }

  virtual void enterEventHandler(MyceliumUIParser::EventHandlerContext * /*ctx*/) override { }
  virtual void exitEventHandler(MyceliumUIParser::EventHandlerContext * /*ctx*/) override { }

  virtual void enterFunctionLiteral(MyceliumUIParser::FunctionLiteralContext * /*ctx*/) override { }
  virtual void exitFunctionLiteral(MyceliumUIParser::FunctionLiteralContext * /*ctx*/) override { }

  virtual void enterFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext * /*ctx*/) override { }
  virtual void exitFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext * /*ctx*/) override { }

  virtual void enterComparisonExpr(MyceliumUIParser::ComparisonExprContext * /*ctx*/) override { }
  virtual void exitComparisonExpr(MyceliumUIParser::ComparisonExprContext * /*ctx*/) override { }

  virtual void enterSelectExpr(MyceliumUIParser::SelectExprContext * /*ctx*/) override { }
  virtual void exitSelectExpr(MyceliumUIParser::SelectExprContext * /*ctx*/) override { }

  virtual void enterLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext * /*ctx*/) override { }
  virtual void exitLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext * /*ctx*/) override { }

  virtual void enterLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext * /*ctx*/) override { }
  virtual void exitLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext * /*ctx*/) override { }

  virtual void enterLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext * /*ctx*/) override { }
  virtual void exitLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext * /*ctx*/) override { }

  virtual void enterEqualityExpr(MyceliumUIParser::EqualityExprContext * /*ctx*/) override { }
  virtual void exitEqualityExpr(MyceliumUIParser::EqualityExprContext * /*ctx*/) override { }

  virtual void enterMulDivExpr(MyceliumUIParser::MulDivExprContext * /*ctx*/) override { }
  virtual void exitMulDivExpr(MyceliumUIParser::MulDivExprContext * /*ctx*/) override { }

  virtual void enterFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext * /*ctx*/) override { }
  virtual void exitFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext * /*ctx*/) override { }

  virtual void enterStaticCallExpr(MyceliumUIParser::StaticCallExprContext * /*ctx*/) override { }
  virtual void exitStaticCallExpr(MyceliumUIParser::StaticCallExprContext * /*ctx*/) override { }

  virtual void enterCalculateExpr(MyceliumUIParser::CalculateExprContext * /*ctx*/) override { }
  virtual void exitCalculateExpr(MyceliumUIParser::CalculateExprContext * /*ctx*/) override { }

  virtual void enterPrimaryExpr(MyceliumUIParser::PrimaryExprContext * /*ctx*/) override { }
  virtual void exitPrimaryExpr(MyceliumUIParser::PrimaryExprContext * /*ctx*/) override { }

  virtual void enterParenExpr(MyceliumUIParser::ParenExprContext * /*ctx*/) override { }
  virtual void exitParenExpr(MyceliumUIParser::ParenExprContext * /*ctx*/) override { }

  virtual void enterMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext * /*ctx*/) override { }
  virtual void exitMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext * /*ctx*/) override { }

  virtual void enterAddSubExpr(MyceliumUIParser::AddSubExprContext * /*ctx*/) override { }
  virtual void exitAddSubExpr(MyceliumUIParser::AddSubExprContext * /*ctx*/) override { }

  virtual void enterThisExpr(MyceliumUIParser::ThisExprContext * /*ctx*/) override { }
  virtual void exitThisExpr(MyceliumUIParser::ThisExprContext * /*ctx*/) override { }

  virtual void enterTernaryExpr(MyceliumUIParser::TernaryExprContext * /*ctx*/) override { }
  virtual void exitTernaryExpr(MyceliumUIParser::TernaryExprContext * /*ctx*/) override { }

  virtual void enterPrimary(MyceliumUIParser::PrimaryContext * /*ctx*/) override { }
  virtual void exitPrimary(MyceliumUIParser::PrimaryContext * /*ctx*/) override { }

  virtual void enterArgumentList(MyceliumUIParser::ArgumentListContext * /*ctx*/) override { }
  virtual void exitArgumentList(MyceliumUIParser::ArgumentListContext * /*ctx*/) override { }

  virtual void enterLiteral(MyceliumUIParser::LiteralContext * /*ctx*/) override { }
  virtual void exitLiteral(MyceliumUIParser::LiteralContext * /*ctx*/) override { }

  virtual void enterBooleanLiteral(MyceliumUIParser::BooleanLiteralContext * /*ctx*/) override { }
  virtual void exitBooleanLiteral(MyceliumUIParser::BooleanLiteralContext * /*ctx*/) override { }

  virtual void enterIdentifier(MyceliumUIParser::IdentifierContext * /*ctx*/) override { }
  virtual void exitIdentifier(MyceliumUIParser::IdentifierContext * /*ctx*/) override { }

  virtual void enterType(MyceliumUIParser::TypeContext * /*ctx*/) override { }
  virtual void exitType(MyceliumUIParser::TypeContext * /*ctx*/) override { }

  virtual void enterScriptBody(MyceliumUIParser::ScriptBodyContext * /*ctx*/) override { }
  virtual void exitScriptBody(MyceliumUIParser::ScriptBodyContext * /*ctx*/) override { }

  virtual void enterCodeItem(MyceliumUIParser::CodeItemContext * /*ctx*/) override { }
  virtual void exitCodeItem(MyceliumUIParser::CodeItemContext * /*ctx*/) override { }

  virtual void enterLengthLiteral(MyceliumUIParser::LengthLiteralContext * /*ctx*/) override { }
  virtual void exitLengthLiteral(MyceliumUIParser::LengthLiteralContext * /*ctx*/) override { }

  virtual void enterTimeLiteral(MyceliumUIParser::TimeLiteralContext * /*ctx*/) override { }
  virtual void exitTimeLiteral(MyceliumUIParser::TimeLiteralContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

