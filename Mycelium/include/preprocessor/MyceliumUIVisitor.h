
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "MyceliumUIParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by MyceliumUIParser.
 */
class  MyceliumUIVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by MyceliumUIParser.
   */
    virtual std::any visitProgram(MyceliumUIParser::ProgramContext *context) = 0;

    virtual std::any visitDefinition(MyceliumUIParser::DefinitionContext *context) = 0;

    virtual std::any visitComponentDefinition(MyceliumUIParser::ComponentDefinitionContext *context) = 0;

    virtual std::any visitComponentMember(MyceliumUIParser::ComponentMemberContext *context) = 0;

    virtual std::any visitPropDecl(MyceliumUIParser::PropDeclContext *context) = 0;

    virtual std::any visitGetPropDecl(MyceliumUIParser::GetPropDeclContext *context) = 0;

    virtual std::any visitStateBlock(MyceliumUIParser::StateBlockContext *context) = 0;

    virtual std::any visitStateVarDecl(MyceliumUIParser::StateVarDeclContext *context) = 0;

    virtual std::any visitEventDecl(MyceliumUIParser::EventDeclContext *context) = 0;

    virtual std::any visitParamList(MyceliumUIParser::ParamListContext *context) = 0;

    virtual std::any visitParam(MyceliumUIParser::ParamContext *context) = 0;

    virtual std::any visitStyleBlock(MyceliumUIParser::StyleBlockContext *context) = 0;

    virtual std::any visitScriptBlock(MyceliumUIParser::ScriptBlockContext *context) = 0;

    virtual std::any visitStyleOverride(MyceliumUIParser::StyleOverrideContext *context) = 0;

    virtual std::any visitStyleAssignment(MyceliumUIParser::StyleAssignmentContext *context) = 0;

    virtual std::any visitStyleIdentifier(MyceliumUIParser::StyleIdentifierContext *context) = 0;

    virtual std::any visitStyleValuePrimarySequence(MyceliumUIParser::StyleValuePrimarySequenceContext *context) = 0;

    virtual std::any visitStyleValueFullExpression(MyceliumUIParser::StyleValueFullExpressionContext *context) = 0;

    virtual std::any visitSelectExpression(MyceliumUIParser::SelectExpressionContext *context) = 0;

    virtual std::any visitSelectCase(MyceliumUIParser::SelectCaseContext *context) = 0;

    virtual std::any visitDefaultCase(MyceliumUIParser::DefaultCaseContext *context) = 0;

    virtual std::any visitCalculateBlock(MyceliumUIParser::CalculateBlockContext *context) = 0;

    virtual std::any visitMainFunction(MyceliumUIParser::MainFunctionContext *context) = 0;

    virtual std::any visitProgramStatement(MyceliumUIParser::ProgramStatementContext *context) = 0;

    virtual std::any visitInstanceDefinition(MyceliumUIParser::InstanceDefinitionContext *context) = 0;

    virtual std::any visitInstanceMember(MyceliumUIParser::InstanceMemberContext *context) = 0;

    virtual std::any visitPropAssignment(MyceliumUIParser::PropAssignmentContext *context) = 0;

    virtual std::any visitEventHandler(MyceliumUIParser::EventHandlerContext *context) = 0;

    virtual std::any visitFunctionLiteral(MyceliumUIParser::FunctionLiteralContext *context) = 0;

    virtual std::any visitFunctionLiteralExpr(MyceliumUIParser::FunctionLiteralExprContext *context) = 0;

    virtual std::any visitComparisonExpr(MyceliumUIParser::ComparisonExprContext *context) = 0;

    virtual std::any visitSelectExpr(MyceliumUIParser::SelectExprContext *context) = 0;

    virtual std::any visitLogicalNotExpr(MyceliumUIParser::LogicalNotExprContext *context) = 0;

    virtual std::any visitLogicalAndExpr(MyceliumUIParser::LogicalAndExprContext *context) = 0;

    virtual std::any visitLogicalOrExpr(MyceliumUIParser::LogicalOrExprContext *context) = 0;

    virtual std::any visitEqualityExpr(MyceliumUIParser::EqualityExprContext *context) = 0;

    virtual std::any visitMulDivExpr(MyceliumUIParser::MulDivExprContext *context) = 0;

    virtual std::any visitFunctionCallExpr(MyceliumUIParser::FunctionCallExprContext *context) = 0;

    virtual std::any visitStaticCallExpr(MyceliumUIParser::StaticCallExprContext *context) = 0;

    virtual std::any visitCalculateExpr(MyceliumUIParser::CalculateExprContext *context) = 0;

    virtual std::any visitPrimaryExpr(MyceliumUIParser::PrimaryExprContext *context) = 0;

    virtual std::any visitParenExpr(MyceliumUIParser::ParenExprContext *context) = 0;

    virtual std::any visitMemberAccessExpr(MyceliumUIParser::MemberAccessExprContext *context) = 0;

    virtual std::any visitAddSubExpr(MyceliumUIParser::AddSubExprContext *context) = 0;

    virtual std::any visitThisExpr(MyceliumUIParser::ThisExprContext *context) = 0;

    virtual std::any visitTernaryExpr(MyceliumUIParser::TernaryExprContext *context) = 0;

    virtual std::any visitPrimary(MyceliumUIParser::PrimaryContext *context) = 0;

    virtual std::any visitArgumentList(MyceliumUIParser::ArgumentListContext *context) = 0;

    virtual std::any visitLiteral(MyceliumUIParser::LiteralContext *context) = 0;

    virtual std::any visitBooleanLiteral(MyceliumUIParser::BooleanLiteralContext *context) = 0;

    virtual std::any visitIdentifier(MyceliumUIParser::IdentifierContext *context) = 0;

    virtual std::any visitType(MyceliumUIParser::TypeContext *context) = 0;

    virtual std::any visitScriptBody(MyceliumUIParser::ScriptBodyContext *context) = 0;

    virtual std::any visitCodeItem(MyceliumUIParser::CodeItemContext *context) = 0;

    virtual std::any visitLengthLiteral(MyceliumUIParser::LengthLiteralContext *context) = 0;

    virtual std::any visitTimeLiteral(MyceliumUIParser::TimeLiteralContext *context) = 0;


};

