
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  MyceliumUIParser : public antlr4::Parser {
public:
  enum {
    COMPONENT = 1, PROP = 2, GET = 3, STATE = 4, EVENT = 5, STYLE = 6, OVERRIDE = 7, 
    SELECT = 8, DEFAULT = 9, SCRIPT = 10, THIS = 11, FUNCTION = 12, VOID = 13, 
    MAIN = 14, TRUE = 15, FALSE = 16, CALCULATE = 17, ON = 18, ID = 19, 
    NUMBER = 20, STRING = 21, PX = 22, PERCENT = 23, SECONDS = 24, LPAREN = 25, 
    RPAREN = 26, LBRACE = 27, RBRACE = 28, SEMI = 29, COLON = 30, QUESTION = 31, 
    ASSIGN = 32, COMMA = 33, DOT = 34, ARROW = 35, DBL_COLON = 36, MINUS = 37, 
    ADD = 38, MUL = 39, DIV = 40, AND = 41, OR = 42, NOT = 43, EQ = 44, 
    NEQ = 45, LT = 46, LTE = 47, GT = 48, GTE = 49, WS = 50, LINE_COMMENT = 51, 
    BLOCK_COMMENT = 52
  };

  enum {
    RuleProgram = 0, RuleDefinition = 1, RuleComponentDefinition = 2, RuleComponentMember = 3, 
    RulePropDecl = 4, RuleGetPropDecl = 5, RuleStateBlock = 6, RuleStateVarDecl = 7, 
    RuleEventDecl = 8, RuleParamList = 9, RuleParam = 10, RuleStyleBlock = 11, 
    RuleScriptBlock = 12, RuleStyleOverride = 13, RuleStyleAssignment = 14, 
    RuleStyleIdentifier = 15, RuleStyleValue = 16, RuleSelectExpression = 17, 
    RuleSelectCase = 18, RuleDefaultCase = 19, RuleCalculateBlock = 20, 
    RuleMainFunction = 21, RuleProgramStatement = 22, RuleInstanceDefinition = 23, 
    RuleInstanceMember = 24, RulePropAssignment = 25, RuleEventHandler = 26, 
    RuleFunctionLiteral = 27, RuleExpression = 28, RulePrimary = 29, RuleArgumentList = 30, 
    RuleLiteral = 31, RuleBooleanLiteral = 32, RuleIdentifier = 33, RuleType = 34, 
    RuleScriptBody = 35, RuleCodeItem = 36, RuleLengthLiteral = 37, RuleTimeLiteral = 38
  };

  explicit MyceliumUIParser(antlr4::TokenStream *input);

  MyceliumUIParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~MyceliumUIParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ProgramContext;
  class DefinitionContext;
  class ComponentDefinitionContext;
  class ComponentMemberContext;
  class PropDeclContext;
  class GetPropDeclContext;
  class StateBlockContext;
  class StateVarDeclContext;
  class EventDeclContext;
  class ParamListContext;
  class ParamContext;
  class StyleBlockContext;
  class ScriptBlockContext;
  class StyleOverrideContext;
  class StyleAssignmentContext;
  class StyleIdentifierContext;
  class StyleValueContext;
  class SelectExpressionContext;
  class SelectCaseContext;
  class DefaultCaseContext;
  class CalculateBlockContext;
  class MainFunctionContext;
  class ProgramStatementContext;
  class InstanceDefinitionContext;
  class InstanceMemberContext;
  class PropAssignmentContext;
  class EventHandlerContext;
  class FunctionLiteralContext;
  class ExpressionContext;
  class PrimaryContext;
  class ArgumentListContext;
  class LiteralContext;
  class BooleanLiteralContext;
  class IdentifierContext;
  class TypeContext;
  class ScriptBodyContext;
  class CodeItemContext;
  class LengthLiteralContext;
  class TimeLiteralContext; 

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<DefinitionContext *> definition();
    DefinitionContext* definition(size_t i);
    MainFunctionContext *mainFunction();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramContext* program();

  class  DefinitionContext : public antlr4::ParserRuleContext {
  public:
    DefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ComponentDefinitionContext *componentDefinition();
    StyleOverrideContext *styleOverride();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DefinitionContext* definition();

  class  ComponentDefinitionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *componentName = nullptr;
    antlr4::Token *baseComponentName = nullptr;
    ComponentDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMPONENT();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    antlr4::tree::TerminalNode *COLON();
    std::vector<ComponentMemberContext *> componentMember();
    ComponentMemberContext* componentMember(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComponentDefinitionContext* componentDefinition();

  class  ComponentMemberContext : public antlr4::ParserRuleContext {
  public:
    ComponentMemberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropDeclContext *propDecl();
    GetPropDeclContext *getPropDecl();
    StateBlockContext *stateBlock();
    EventDeclContext *eventDecl();
    StyleBlockContext *styleBlock();
    ScriptBlockContext *scriptBlock();
    InstanceDefinitionContext *instanceDefinition();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ComponentMemberContext* componentMember();

  class  PropDeclContext : public antlr4::ParserRuleContext {
  public:
    PropDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PROP();
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropDeclContext* propDecl();

  class  GetPropDeclContext : public antlr4::ParserRuleContext {
  public:
    GetPropDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *GET();
    antlr4::tree::TerminalNode *PROP();
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *ARROW();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *SEMI();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  GetPropDeclContext* getPropDecl();

  class  StateBlockContext : public antlr4::ParserRuleContext {
  public:
    StateBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STATE();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<StateVarDeclContext *> stateVarDecl();
    StateVarDeclContext* stateVarDecl(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StateBlockContext* stateBlock();

  class  StateVarDeclContext : public antlr4::ParserRuleContext {
  public:
    StateVarDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *ASSIGN();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StateVarDeclContext* stateVarDecl();

  class  EventDeclContext : public antlr4::ParserRuleContext {
  public:
    EventDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EVENT();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *SEMI();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ParamListContext *paramList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EventDeclContext* eventDecl();

  class  ParamListContext : public antlr4::ParserRuleContext {
  public:
    ParamListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ParamContext *> param();
    ParamContext* param(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamListContext* paramList();

  class  ParamContext : public antlr4::ParserRuleContext {
  public:
    ParamContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    TypeContext *type();
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ParamContext* param();

  class  StyleBlockContext : public antlr4::ParserRuleContext {
  public:
    StyleBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *STYLE();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<StyleAssignmentContext *> styleAssignment();
    StyleAssignmentContext* styleAssignment(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SEMI();
    antlr4::tree::TerminalNode* SEMI(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StyleBlockContext* styleBlock();

  class  ScriptBlockContext : public antlr4::ParserRuleContext {
  public:
    ScriptBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SCRIPT();
    antlr4::tree::TerminalNode *LBRACE();
    ScriptBodyContext *scriptBody();
    antlr4::tree::TerminalNode *RBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ScriptBlockContext* scriptBlock();

  class  StyleOverrideContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *targetType = nullptr;
    StyleOverrideContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *OVERRIDE();
    antlr4::tree::TerminalNode *STYLE();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    antlr4::tree::TerminalNode *ID();
    std::vector<StyleAssignmentContext *> styleAssignment();
    StyleAssignmentContext* styleAssignment(size_t i);
    std::vector<antlr4::tree::TerminalNode *> SEMI();
    antlr4::tree::TerminalNode* SEMI(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StyleOverrideContext* styleOverride();

  class  StyleAssignmentContext : public antlr4::ParserRuleContext {
  public:
    MyceliumUIParser::StyleIdentifierContext *propertyName = nullptr;
    StyleAssignmentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COLON();
    StyleValueContext *styleValue();
    StyleIdentifierContext *styleIdentifier();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StyleAssignmentContext* styleAssignment();

  class  StyleIdentifierContext : public antlr4::ParserRuleContext {
  public:
    StyleIdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    std::vector<antlr4::tree::TerminalNode *> MINUS();
    antlr4::tree::TerminalNode* MINUS(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StyleIdentifierContext* styleIdentifier();

  class  StyleValueContext : public antlr4::ParserRuleContext {
  public:
    StyleValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    StyleValueContext() = default;
    void copyFrom(StyleValueContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  StyleValuePrimarySequenceContext : public StyleValueContext {
  public:
    StyleValuePrimarySequenceContext(StyleValueContext *ctx);

    std::vector<PrimaryContext *> primary();
    PrimaryContext* primary(size_t i);
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  StyleValueFullExpressionContext : public StyleValueContext {
  public:
    StyleValueFullExpressionContext(StyleValueContext *ctx);

    ExpressionContext *expression();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  StyleValueContext* styleValue();

  class  SelectExpressionContext : public antlr4::ParserRuleContext {
  public:
    SelectExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SELECT();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<SelectCaseContext *> selectCase();
    SelectCaseContext* selectCase(size_t i);
    DefaultCaseContext *defaultCase();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectExpressionContext* selectExpression();

  class  SelectCaseContext : public antlr4::ParserRuleContext {
  public:
    SelectCaseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *COLON();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectCaseContext* selectCase();

  class  DefaultCaseContext : public antlr4::ParserRuleContext {
  public:
    DefaultCaseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DEFAULT();
    antlr4::tree::TerminalNode *COLON();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *COMMA();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  DefaultCaseContext* defaultCase();

  class  CalculateBlockContext : public antlr4::ParserRuleContext {
  public:
    CalculateBlockContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CALCULATE();
    antlr4::tree::TerminalNode *LBRACE();
    ScriptBodyContext *scriptBody();
    antlr4::tree::TerminalNode *RBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CalculateBlockContext* calculateBlock();

  class  MainFunctionContext : public antlr4::ParserRuleContext {
  public:
    MainFunctionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *VOID();
    antlr4::tree::TerminalNode *MAIN();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<ProgramStatementContext *> programStatement();
    ProgramStatementContext* programStatement(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  MainFunctionContext* mainFunction();

  class  ProgramStatementContext : public antlr4::ParserRuleContext {
  public:
    ProgramStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    InstanceDefinitionContext *instanceDefinition();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramStatementContext* programStatement();

  class  InstanceDefinitionContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *typeName = nullptr;
    antlr4::Token *instanceId = nullptr;
    InstanceDefinitionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    antlr4::tree::TerminalNode *RBRACE();
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    std::vector<InstanceMemberContext *> instanceMember();
    InstanceMemberContext* instanceMember(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InstanceDefinitionContext* instanceDefinition();

  class  InstanceMemberContext : public antlr4::ParserRuleContext {
  public:
    MyceliumUIParser::StyleBlockContext *inlineStyle = nullptr;
    InstanceMemberContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    PropAssignmentContext *propAssignment();
    antlr4::tree::TerminalNode *SEMI();
    EventHandlerContext *eventHandler();
    StyleBlockContext *styleBlock();
    InstanceDefinitionContext *instanceDefinition();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  InstanceMemberContext* instanceMember();

  class  PropAssignmentContext : public antlr4::ParserRuleContext {
  public:
    PropAssignmentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *COLON();
    ExpressionContext *expression();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PropAssignmentContext* propAssignment();

  class  EventHandlerContext : public antlr4::ParserRuleContext {
  public:
    EventHandlerContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ON();
    antlr4::tree::TerminalNode *ID();
    antlr4::tree::TerminalNode *COLON();
    FunctionLiteralContext *functionLiteral();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  EventHandlerContext* eventHandler();

  class  FunctionLiteralContext : public antlr4::ParserRuleContext {
  public:
    FunctionLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *FUNCTION();
    antlr4::tree::TerminalNode *LBRACE();
    ScriptBodyContext *scriptBody();
    antlr4::tree::TerminalNode *RBRACE();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ParamListContext *paramList();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionLiteralContext* functionLiteral();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() = default;
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  FunctionLiteralExprContext : public ExpressionContext {
  public:
    FunctionLiteralExprContext(ExpressionContext *ctx);

    FunctionLiteralContext *functionLiteral();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ComparisonExprContext : public ExpressionContext {
  public:
    ComparisonExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *LT();
    antlr4::tree::TerminalNode *LTE();
    antlr4::tree::TerminalNode *GT();
    antlr4::tree::TerminalNode *GTE();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SelectExprContext : public ExpressionContext {
  public:
    SelectExprContext(ExpressionContext *ctx);

    SelectExpressionContext *selectExpression();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicalNotExprContext : public ExpressionContext {
  public:
    LogicalNotExprContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *NOT();
    ExpressionContext *expression();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicalAndExprContext : public ExpressionContext {
  public:
    LogicalAndExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *AND();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicalOrExprContext : public ExpressionContext {
  public:
    LogicalOrExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *OR();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  EqualityExprContext : public ExpressionContext {
  public:
    EqualityExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *EQ();
    antlr4::tree::TerminalNode *NEQ();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MulDivExprContext : public ExpressionContext {
  public:
    MulDivExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *MUL();
    antlr4::tree::TerminalNode *DIV();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FunctionCallExprContext : public ExpressionContext {
  public:
    FunctionCallExprContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ArgumentListContext *argumentList();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  StaticCallExprContext : public ExpressionContext {
  public:
    StaticCallExprContext(ExpressionContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    antlr4::tree::TerminalNode *DBL_COLON();
    antlr4::tree::TerminalNode *LPAREN();
    antlr4::tree::TerminalNode *RPAREN();
    ArgumentListContext *argumentList();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  CalculateExprContext : public ExpressionContext {
  public:
    CalculateExprContext(ExpressionContext *ctx);

    CalculateBlockContext *calculateBlock();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PrimaryExprContext : public ExpressionContext {
  public:
    PrimaryExprContext(ExpressionContext *ctx);

    PrimaryContext *primary();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ParenExprContext : public ExpressionContext {
  public:
    ParenExprContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *LPAREN();
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *RPAREN();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MemberAccessExprContext : public ExpressionContext {
  public:
    MemberAccessExprContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    antlr4::tree::TerminalNode *DOT();
    antlr4::tree::TerminalNode *ID();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AddSubExprContext : public ExpressionContext {
  public:
    AddSubExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *ADD();
    antlr4::tree::TerminalNode *MINUS();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ThisExprContext : public ExpressionContext {
  public:
    ThisExprContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *THIS();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TernaryExprContext : public ExpressionContext {
  public:
    TernaryExprContext(ExpressionContext *ctx);

    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    antlr4::tree::TerminalNode *QUESTION();
    antlr4::tree::TerminalNode *COLON();
    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  PrimaryContext : public antlr4::ParserRuleContext {
  public:
    PrimaryContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    LiteralContext *literal();
    IdentifierContext *identifier();
    antlr4::tree::TerminalNode *THIS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  PrimaryContext* primary();

  class  ArgumentListContext : public antlr4::ParserRuleContext {
  public:
    ArgumentListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMA();
    antlr4::tree::TerminalNode* COMMA(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ArgumentListContext* argumentList();

  class  LiteralContext : public antlr4::ParserRuleContext {
  public:
    LiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *STRING();
    BooleanLiteralContext *booleanLiteral();
    LengthLiteralContext *lengthLiteral();
    TimeLiteralContext *timeLiteral();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LiteralContext* literal();

  class  BooleanLiteralContext : public antlr4::ParserRuleContext {
  public:
    BooleanLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TRUE();
    antlr4::tree::TerminalNode *FALSE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BooleanLiteralContext* booleanLiteral();

  class  IdentifierContext : public antlr4::ParserRuleContext {
  public:
    IdentifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifierContext* identifier();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ID();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();

  class  ScriptBodyContext : public antlr4::ParserRuleContext {
  public:
    ScriptBodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<CodeItemContext *> codeItem();
    CodeItemContext* codeItem(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ScriptBodyContext* scriptBody();

  class  CodeItemContext : public antlr4::ParserRuleContext {
  public:
    CodeItemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *LBRACE();
    ScriptBodyContext *scriptBody();
    antlr4::tree::TerminalNode *RBRACE();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  CodeItemContext* codeItem();

  class  LengthLiteralContext : public antlr4::ParserRuleContext {
  public:
    LengthLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *PX();
    antlr4::tree::TerminalNode *PERCENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  LengthLiteralContext* lengthLiteral();

  class  TimeLiteralContext : public antlr4::ParserRuleContext {
  public:
    TimeLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();
    antlr4::tree::TerminalNode *SECONDS();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TimeLiteralContext* timeLiteral();


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

