
// Generated from MySimpleUI.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  MySimpleUILexer : public antlr4::Lexer
{
    public:
        enum
        {
            STATE = 1, BIND = 2, NUMBER = 3, STRING = 4, ID = 5, AT_ID = 6, LPAREN = 7,
            RPAREN = 8, LBRACE = 9, RBRACE = 10, COLON = 11, SEMI = 12, ASSIGN = 13,
            COMMA = 14, WS = 15, LINE_COMMENT = 16
        };

        explicit MySimpleUILexer(antlr4::CharStream *input);

        ~MySimpleUILexer() override;


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

