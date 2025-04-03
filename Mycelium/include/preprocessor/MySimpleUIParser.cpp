
// Generated from MySimpleUI.g4 by ANTLR 4.13.2


#include "MySimpleUIListener.h"
#include "MySimpleUIVisitor.h"

#include "MySimpleUIParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct MySimpleUIParserStaticData final {
  MySimpleUIParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MySimpleUIParserStaticData(const MySimpleUIParserStaticData&) = delete;
  MySimpleUIParserStaticData(MySimpleUIParserStaticData&&) = delete;
  MySimpleUIParserStaticData& operator=(const MySimpleUIParserStaticData&) = delete;
  MySimpleUIParserStaticData& operator=(MySimpleUIParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag mysimpleuiParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<MySimpleUIParserStaticData> mysimpleuiParserStaticData = nullptr;

void mysimpleuiParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (mysimpleuiParserStaticData != nullptr) {
    return;
  }
#else
  assert(mysimpleuiParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<MySimpleUIParserStaticData>(
    std::vector<std::string>{
      "program", "stateDecl", "rootComponent", "componentCall", "parameterList", 
      "parameter", "element", "propertyAssignment", "expression", "literal", 
      "identifier"
    },
    std::vector<std::string>{
      "", "'state'", "'bind'", "", "", "", "", "'('", "')'", "'{'", "'}'", 
      "':'", "';'", "'='", "','"
    },
    std::vector<std::string>{
      "", "STATE", "BIND", "NUMBER", "STRING", "ID", "AT_ID", "LPAREN", 
      "RPAREN", "LBRACE", "RBRACE", "COLON", "SEMI", "ASSIGN", "COMMA", 
      "WS", "LINE_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,16,86,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,7,
  	7,7,2,8,7,8,2,9,7,9,2,10,7,10,1,0,5,0,24,8,0,10,0,12,0,27,9,0,1,0,1,0,
  	1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,3,1,3,1,3,3,3,44,8,3,1,3,1,
  	3,1,3,5,3,49,8,3,10,3,12,3,52,9,3,1,3,1,3,1,4,1,4,1,4,5,4,59,8,4,10,4,
  	12,4,62,9,4,1,5,3,5,65,8,5,1,5,1,5,1,6,1,6,3,6,71,8,6,1,7,1,7,1,7,1,7,
  	1,7,1,8,1,8,3,8,80,8,8,1,9,1,9,1,10,1,10,1,10,0,0,11,0,2,4,6,8,10,12,
  	14,16,18,20,0,2,1,0,3,4,1,0,5,6,81,0,25,1,0,0,0,2,31,1,0,0,0,4,38,1,0,
  	0,0,6,40,1,0,0,0,8,55,1,0,0,0,10,64,1,0,0,0,12,70,1,0,0,0,14,72,1,0,0,
  	0,16,79,1,0,0,0,18,81,1,0,0,0,20,83,1,0,0,0,22,24,3,2,1,0,23,22,1,0,0,
  	0,24,27,1,0,0,0,25,23,1,0,0,0,25,26,1,0,0,0,26,28,1,0,0,0,27,25,1,0,0,
  	0,28,29,3,4,2,0,29,30,5,0,0,1,30,1,1,0,0,0,31,32,5,1,0,0,32,33,5,5,0,
  	0,33,34,5,5,0,0,34,35,5,13,0,0,35,36,3,16,8,0,36,37,5,12,0,0,37,3,1,0,
  	0,0,38,39,3,6,3,0,39,5,1,0,0,0,40,41,5,5,0,0,41,43,5,7,0,0,42,44,3,8,
  	4,0,43,42,1,0,0,0,43,44,1,0,0,0,44,45,1,0,0,0,45,46,5,8,0,0,46,50,5,9,
  	0,0,47,49,3,12,6,0,48,47,1,0,0,0,49,52,1,0,0,0,50,48,1,0,0,0,50,51,1,
  	0,0,0,51,53,1,0,0,0,52,50,1,0,0,0,53,54,5,10,0,0,54,7,1,0,0,0,55,60,3,
  	10,5,0,56,57,5,14,0,0,57,59,3,10,5,0,58,56,1,0,0,0,59,62,1,0,0,0,60,58,
  	1,0,0,0,60,61,1,0,0,0,61,9,1,0,0,0,62,60,1,0,0,0,63,65,5,2,0,0,64,63,
  	1,0,0,0,64,65,1,0,0,0,65,66,1,0,0,0,66,67,3,16,8,0,67,11,1,0,0,0,68,71,
  	3,14,7,0,69,71,3,6,3,0,70,68,1,0,0,0,70,69,1,0,0,0,71,13,1,0,0,0,72,73,
  	5,5,0,0,73,74,5,11,0,0,74,75,3,16,8,0,75,76,5,12,0,0,76,15,1,0,0,0,77,
  	80,3,18,9,0,78,80,3,20,10,0,79,77,1,0,0,0,79,78,1,0,0,0,80,17,1,0,0,0,
  	81,82,7,0,0,0,82,19,1,0,0,0,83,84,7,1,0,0,84,21,1,0,0,0,7,25,43,50,60,
  	64,70,79
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  mysimpleuiParserStaticData = std::move(staticData);
}

}

MySimpleUIParser::MySimpleUIParser(TokenStream *input) : MySimpleUIParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

MySimpleUIParser::MySimpleUIParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  MySimpleUIParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *mysimpleuiParserStaticData->atn, mysimpleuiParserStaticData->decisionToDFA, mysimpleuiParserStaticData->sharedContextCache, options);
}

MySimpleUIParser::~MySimpleUIParser() {
  delete _interpreter;
}

const atn::ATN& MySimpleUIParser::getATN() const {
  return *mysimpleuiParserStaticData->atn;
}

std::string MySimpleUIParser::getGrammarFileName() const {
  return "MySimpleUI.g4";
}

const std::vector<std::string>& MySimpleUIParser::getRuleNames() const {
  return mysimpleuiParserStaticData->ruleNames;
}

const dfa::Vocabulary& MySimpleUIParser::getVocabulary() const {
  return mysimpleuiParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MySimpleUIParser::getSerializedATN() const {
  return mysimpleuiParserStaticData->serializedATN;
}


//----------------- ProgramContext ------------------------------------------------------------------

MySimpleUIParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MySimpleUIParser::RootComponentContext* MySimpleUIParser::ProgramContext::rootComponent() {
  return getRuleContext<MySimpleUIParser::RootComponentContext>(0);
}

tree::TerminalNode* MySimpleUIParser::ProgramContext::EOF() {
  return getToken(MySimpleUIParser::EOF, 0);
}

std::vector<MySimpleUIParser::StateDeclContext *> MySimpleUIParser::ProgramContext::stateDecl() {
  return getRuleContexts<MySimpleUIParser::StateDeclContext>();
}

MySimpleUIParser::StateDeclContext* MySimpleUIParser::ProgramContext::stateDecl(size_t i) {
  return getRuleContext<MySimpleUIParser::StateDeclContext>(i);
}


size_t MySimpleUIParser::ProgramContext::getRuleIndex() const {
  return MySimpleUIParser::RuleProgram;
}

void MySimpleUIParser::ProgramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgram(this);
}

void MySimpleUIParser::ProgramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgram(this);
}


std::any MySimpleUIParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ProgramContext* MySimpleUIParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 0, MySimpleUIParser::RuleProgram);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(25);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MySimpleUIParser::STATE) {
      setState(22);
      stateDecl();
      setState(27);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(28);
    rootComponent();
    setState(29);
    match(MySimpleUIParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateDeclContext ------------------------------------------------------------------

MySimpleUIParser::StateDeclContext::StateDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MySimpleUIParser::StateDeclContext::STATE() {
  return getToken(MySimpleUIParser::STATE, 0);
}

tree::TerminalNode* MySimpleUIParser::StateDeclContext::ASSIGN() {
  return getToken(MySimpleUIParser::ASSIGN, 0);
}

tree::TerminalNode* MySimpleUIParser::StateDeclContext::SEMI() {
  return getToken(MySimpleUIParser::SEMI, 0);
}

std::vector<tree::TerminalNode *> MySimpleUIParser::StateDeclContext::ID() {
  return getTokens(MySimpleUIParser::ID);
}

tree::TerminalNode* MySimpleUIParser::StateDeclContext::ID(size_t i) {
  return getToken(MySimpleUIParser::ID, i);
}

MySimpleUIParser::ExpressionContext* MySimpleUIParser::StateDeclContext::expression() {
  return getRuleContext<MySimpleUIParser::ExpressionContext>(0);
}


size_t MySimpleUIParser::StateDeclContext::getRuleIndex() const {
  return MySimpleUIParser::RuleStateDecl;
}

void MySimpleUIParser::StateDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStateDecl(this);
}

void MySimpleUIParser::StateDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStateDecl(this);
}


std::any MySimpleUIParser::StateDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitStateDecl(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::StateDeclContext* MySimpleUIParser::stateDecl() {
  StateDeclContext *_localctx = _tracker.createInstance<StateDeclContext>(_ctx, getState());
  enterRule(_localctx, 2, MySimpleUIParser::RuleStateDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(31);
    match(MySimpleUIParser::STATE);
    setState(32);
    antlrcpp::downCast<StateDeclContext *>(_localctx)->type = match(MySimpleUIParser::ID);
    setState(33);
    antlrcpp::downCast<StateDeclContext *>(_localctx)->name = match(MySimpleUIParser::ID);
    setState(34);
    match(MySimpleUIParser::ASSIGN);
    setState(35);
    antlrcpp::downCast<StateDeclContext *>(_localctx)->value = expression();
    setState(36);
    match(MySimpleUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RootComponentContext ------------------------------------------------------------------

MySimpleUIParser::RootComponentContext::RootComponentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MySimpleUIParser::ComponentCallContext* MySimpleUIParser::RootComponentContext::componentCall() {
  return getRuleContext<MySimpleUIParser::ComponentCallContext>(0);
}


size_t MySimpleUIParser::RootComponentContext::getRuleIndex() const {
  return MySimpleUIParser::RuleRootComponent;
}

void MySimpleUIParser::RootComponentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRootComponent(this);
}

void MySimpleUIParser::RootComponentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRootComponent(this);
}


std::any MySimpleUIParser::RootComponentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitRootComponent(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::RootComponentContext* MySimpleUIParser::rootComponent() {
  RootComponentContext *_localctx = _tracker.createInstance<RootComponentContext>(_ctx, getState());
  enterRule(_localctx, 4, MySimpleUIParser::RuleRootComponent);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(38);
    componentCall();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ComponentCallContext ------------------------------------------------------------------

MySimpleUIParser::ComponentCallContext::ComponentCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MySimpleUIParser::ComponentCallContext::LPAREN() {
  return getToken(MySimpleUIParser::LPAREN, 0);
}

tree::TerminalNode* MySimpleUIParser::ComponentCallContext::RPAREN() {
  return getToken(MySimpleUIParser::RPAREN, 0);
}

tree::TerminalNode* MySimpleUIParser::ComponentCallContext::LBRACE() {
  return getToken(MySimpleUIParser::LBRACE, 0);
}

tree::TerminalNode* MySimpleUIParser::ComponentCallContext::RBRACE() {
  return getToken(MySimpleUIParser::RBRACE, 0);
}

tree::TerminalNode* MySimpleUIParser::ComponentCallContext::ID() {
  return getToken(MySimpleUIParser::ID, 0);
}

MySimpleUIParser::ParameterListContext* MySimpleUIParser::ComponentCallContext::parameterList() {
  return getRuleContext<MySimpleUIParser::ParameterListContext>(0);
}

std::vector<MySimpleUIParser::ElementContext *> MySimpleUIParser::ComponentCallContext::element() {
  return getRuleContexts<MySimpleUIParser::ElementContext>();
}

MySimpleUIParser::ElementContext* MySimpleUIParser::ComponentCallContext::element(size_t i) {
  return getRuleContext<MySimpleUIParser::ElementContext>(i);
}


size_t MySimpleUIParser::ComponentCallContext::getRuleIndex() const {
  return MySimpleUIParser::RuleComponentCall;
}

void MySimpleUIParser::ComponentCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComponentCall(this);
}

void MySimpleUIParser::ComponentCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComponentCall(this);
}


std::any MySimpleUIParser::ComponentCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitComponentCall(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ComponentCallContext* MySimpleUIParser::componentCall() {
  ComponentCallContext *_localctx = _tracker.createInstance<ComponentCallContext>(_ctx, getState());
  enterRule(_localctx, 6, MySimpleUIParser::RuleComponentCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(40);
    antlrcpp::downCast<ComponentCallContext *>(_localctx)->componentName = match(MySimpleUIParser::ID);
    setState(41);
    match(MySimpleUIParser::LPAREN);
    setState(43);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 124) != 0)) {
      setState(42);
      parameterList();
    }
    setState(45);
    match(MySimpleUIParser::RPAREN);
    setState(46);
    match(MySimpleUIParser::LBRACE);
    setState(50);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MySimpleUIParser::ID) {
      setState(47);
      element();
      setState(52);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(53);
    match(MySimpleUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterListContext ------------------------------------------------------------------

MySimpleUIParser::ParameterListContext::ParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MySimpleUIParser::ParameterContext *> MySimpleUIParser::ParameterListContext::parameter() {
  return getRuleContexts<MySimpleUIParser::ParameterContext>();
}

MySimpleUIParser::ParameterContext* MySimpleUIParser::ParameterListContext::parameter(size_t i) {
  return getRuleContext<MySimpleUIParser::ParameterContext>(i);
}

std::vector<tree::TerminalNode *> MySimpleUIParser::ParameterListContext::COMMA() {
  return getTokens(MySimpleUIParser::COMMA);
}

tree::TerminalNode* MySimpleUIParser::ParameterListContext::COMMA(size_t i) {
  return getToken(MySimpleUIParser::COMMA, i);
}


size_t MySimpleUIParser::ParameterListContext::getRuleIndex() const {
  return MySimpleUIParser::RuleParameterList;
}

void MySimpleUIParser::ParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParameterList(this);
}

void MySimpleUIParser::ParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParameterList(this);
}


std::any MySimpleUIParser::ParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitParameterList(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ParameterListContext* MySimpleUIParser::parameterList() {
  ParameterListContext *_localctx = _tracker.createInstance<ParameterListContext>(_ctx, getState());
  enterRule(_localctx, 8, MySimpleUIParser::RuleParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(55);
    parameter();
    setState(60);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MySimpleUIParser::COMMA) {
      setState(56);
      match(MySimpleUIParser::COMMA);
      setState(57);
      parameter();
      setState(62);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParameterContext ------------------------------------------------------------------

MySimpleUIParser::ParameterContext::ParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MySimpleUIParser::ExpressionContext* MySimpleUIParser::ParameterContext::expression() {
  return getRuleContext<MySimpleUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MySimpleUIParser::ParameterContext::BIND() {
  return getToken(MySimpleUIParser::BIND, 0);
}


size_t MySimpleUIParser::ParameterContext::getRuleIndex() const {
  return MySimpleUIParser::RuleParameter;
}

void MySimpleUIParser::ParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParameter(this);
}

void MySimpleUIParser::ParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParameter(this);
}


std::any MySimpleUIParser::ParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitParameter(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ParameterContext* MySimpleUIParser::parameter() {
  ParameterContext *_localctx = _tracker.createInstance<ParameterContext>(_ctx, getState());
  enterRule(_localctx, 10, MySimpleUIParser::RuleParameter);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(64);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MySimpleUIParser::BIND) {
      setState(63);
      match(MySimpleUIParser::BIND);
    }
    setState(66);
    expression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ElementContext ------------------------------------------------------------------

MySimpleUIParser::ElementContext::ElementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MySimpleUIParser::PropertyAssignmentContext* MySimpleUIParser::ElementContext::propertyAssignment() {
  return getRuleContext<MySimpleUIParser::PropertyAssignmentContext>(0);
}

MySimpleUIParser::ComponentCallContext* MySimpleUIParser::ElementContext::componentCall() {
  return getRuleContext<MySimpleUIParser::ComponentCallContext>(0);
}


size_t MySimpleUIParser::ElementContext::getRuleIndex() const {
  return MySimpleUIParser::RuleElement;
}

void MySimpleUIParser::ElementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterElement(this);
}

void MySimpleUIParser::ElementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitElement(this);
}


std::any MySimpleUIParser::ElementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitElement(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ElementContext* MySimpleUIParser::element() {
  ElementContext *_localctx = _tracker.createInstance<ElementContext>(_ctx, getState());
  enterRule(_localctx, 12, MySimpleUIParser::RuleElement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(70);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(68);
      propertyAssignment();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(69);
      componentCall();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PropertyAssignmentContext ------------------------------------------------------------------

MySimpleUIParser::PropertyAssignmentContext::PropertyAssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MySimpleUIParser::PropertyAssignmentContext::COLON() {
  return getToken(MySimpleUIParser::COLON, 0);
}

tree::TerminalNode* MySimpleUIParser::PropertyAssignmentContext::SEMI() {
  return getToken(MySimpleUIParser::SEMI, 0);
}

tree::TerminalNode* MySimpleUIParser::PropertyAssignmentContext::ID() {
  return getToken(MySimpleUIParser::ID, 0);
}

MySimpleUIParser::ExpressionContext* MySimpleUIParser::PropertyAssignmentContext::expression() {
  return getRuleContext<MySimpleUIParser::ExpressionContext>(0);
}


size_t MySimpleUIParser::PropertyAssignmentContext::getRuleIndex() const {
  return MySimpleUIParser::RulePropertyAssignment;
}

void MySimpleUIParser::PropertyAssignmentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPropertyAssignment(this);
}

void MySimpleUIParser::PropertyAssignmentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPropertyAssignment(this);
}


std::any MySimpleUIParser::PropertyAssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitPropertyAssignment(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::PropertyAssignmentContext* MySimpleUIParser::propertyAssignment() {
  PropertyAssignmentContext *_localctx = _tracker.createInstance<PropertyAssignmentContext>(_ctx, getState());
  enterRule(_localctx, 14, MySimpleUIParser::RulePropertyAssignment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(72);
    antlrcpp::downCast<PropertyAssignmentContext *>(_localctx)->propertyName = match(MySimpleUIParser::ID);
    setState(73);
    match(MySimpleUIParser::COLON);
    setState(74);
    antlrcpp::downCast<PropertyAssignmentContext *>(_localctx)->value = expression();
    setState(75);
    match(MySimpleUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

MySimpleUIParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MySimpleUIParser::LiteralContext* MySimpleUIParser::ExpressionContext::literal() {
  return getRuleContext<MySimpleUIParser::LiteralContext>(0);
}

MySimpleUIParser::IdentifierContext* MySimpleUIParser::ExpressionContext::identifier() {
  return getRuleContext<MySimpleUIParser::IdentifierContext>(0);
}


size_t MySimpleUIParser::ExpressionContext::getRuleIndex() const {
  return MySimpleUIParser::RuleExpression;
}

void MySimpleUIParser::ExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExpression(this);
}

void MySimpleUIParser::ExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExpression(this);
}


std::any MySimpleUIParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::ExpressionContext* MySimpleUIParser::expression() {
  ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, getState());
  enterRule(_localctx, 16, MySimpleUIParser::RuleExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(79);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MySimpleUIParser::NUMBER:
      case MySimpleUIParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(77);
        literal();
        break;
      }

      case MySimpleUIParser::ID:
      case MySimpleUIParser::AT_ID: {
        enterOuterAlt(_localctx, 2);
        setState(78);
        identifier();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LiteralContext ------------------------------------------------------------------

MySimpleUIParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MySimpleUIParser::LiteralContext::NUMBER() {
  return getToken(MySimpleUIParser::NUMBER, 0);
}

tree::TerminalNode* MySimpleUIParser::LiteralContext::STRING() {
  return getToken(MySimpleUIParser::STRING, 0);
}


size_t MySimpleUIParser::LiteralContext::getRuleIndex() const {
  return MySimpleUIParser::RuleLiteral;
}

void MySimpleUIParser::LiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLiteral(this);
}

void MySimpleUIParser::LiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLiteral(this);
}


std::any MySimpleUIParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::LiteralContext* MySimpleUIParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 18, MySimpleUIParser::RuleLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(81);
    _la = _input->LA(1);
    if (!(_la == MySimpleUIParser::NUMBER

    || _la == MySimpleUIParser::STRING)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdentifierContext ------------------------------------------------------------------

MySimpleUIParser::IdentifierContext::IdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MySimpleUIParser::IdentifierContext::ID() {
  return getToken(MySimpleUIParser::ID, 0);
}

tree::TerminalNode* MySimpleUIParser::IdentifierContext::AT_ID() {
  return getToken(MySimpleUIParser::AT_ID, 0);
}


size_t MySimpleUIParser::IdentifierContext::getRuleIndex() const {
  return MySimpleUIParser::RuleIdentifier;
}

void MySimpleUIParser::IdentifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIdentifier(this);
}

void MySimpleUIParser::IdentifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MySimpleUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIdentifier(this);
}


std::any MySimpleUIParser::IdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MySimpleUIVisitor*>(visitor))
    return parserVisitor->visitIdentifier(this);
  else
    return visitor->visitChildren(this);
}

MySimpleUIParser::IdentifierContext* MySimpleUIParser::identifier() {
  IdentifierContext *_localctx = _tracker.createInstance<IdentifierContext>(_ctx, getState());
  enterRule(_localctx, 20, MySimpleUIParser::RuleIdentifier);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(83);
    _la = _input->LA(1);
    if (!(_la == MySimpleUIParser::ID

    || _la == MySimpleUIParser::AT_ID)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void MySimpleUIParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  mysimpleuiParserInitialize();
#else
  ::antlr4::internal::call_once(mysimpleuiParserOnceFlag, mysimpleuiParserInitialize);
#endif
}
