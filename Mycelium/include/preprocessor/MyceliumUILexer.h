
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  MyceliumUILexer : public antlr4::Lexer {
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

  explicit MyceliumUILexer(antlr4::CharStream *input);

  ~MyceliumUILexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

