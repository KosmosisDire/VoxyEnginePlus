
// Generated from ./MyceliumUI.g4 by ANTLR 4.13.2


#include "MyceliumUIListener.h"
#include "MyceliumUIVisitor.h"

#include "MyceliumUIParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct MyceliumUIParserStaticData final {
  MyceliumUIParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MyceliumUIParserStaticData(const MyceliumUIParserStaticData&) = delete;
  MyceliumUIParserStaticData(MyceliumUIParserStaticData&&) = delete;
  MyceliumUIParserStaticData& operator=(const MyceliumUIParserStaticData&) = delete;
  MyceliumUIParserStaticData& operator=(MyceliumUIParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag myceliumuiParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<MyceliumUIParserStaticData> myceliumuiParserStaticData = nullptr;

void myceliumuiParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (myceliumuiParserStaticData != nullptr) {
    return;
  }
#else
  assert(myceliumuiParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<MyceliumUIParserStaticData>(
    std::vector<std::string>{
      "program", "definition", "componentDefinition", "componentMember", 
      "propDecl", "getPropDecl", "stateBlock", "stateVarDecl", "eventDecl", 
      "paramList", "param", "styleBlock", "scriptBlock", "styleOverride", 
      "styleAssignment", "styleIdentifier", "styleValue", "selectExpression", 
      "selectCase", "defaultCase", "calculateBlock", "mainFunction", "programStatement", 
      "instanceDefinition", "instanceMember", "propAssignment", "eventHandler", 
      "functionLiteral", "expression", "primary", "argumentList", "literal", 
      "booleanLiteral", "identifier", "type", "scriptBody", "codeItem", 
      "lengthLiteral", "timeLiteral"
    },
    std::vector<std::string>{
      "", "'component'", "'prop'", "'get'", "'state'", "'event'", "'style'", 
      "'override'", "'select'", "'default'", "'script'", "'this'", "'function'", 
      "'void'", "'Main'", "'true'", "'false'", "'calculate'", "'on'", "", 
      "", "", "'px'", "'%'", "'s'", "'('", "')'", "'{'", "'}'", "';'", "':'", 
      "'\\u003F'", "'='", "','", "'.'", "'=>'", "'::'", "'-'", "'+'", "'*'", 
      "'/'", "'&&'", "'||'", "'!'", "'=='", "'!='", "'<'", "'<='", "'>'", 
      "'>='"
    },
    std::vector<std::string>{
      "", "COMPONENT", "PROP", "GET", "STATE", "EVENT", "STYLE", "OVERRIDE", 
      "SELECT", "DEFAULT", "SCRIPT", "THIS", "FUNCTION", "VOID", "MAIN", 
      "TRUE", "FALSE", "CALCULATE", "ON", "ID", "NUMBER", "STRING", "PX", 
      "PERCENT", "SECONDS", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "SEMI", 
      "COLON", "QUESTION", "ASSIGN", "COMMA", "DOT", "ARROW", "DBL_COLON", 
      "MINUS", "ADD", "MUL", "DIV", "AND", "OR", "NOT", "EQ", "NEQ", "LT", 
      "LTE", "GT", "GTE", "WS", "LINE_COMMENT", "BLOCK_COMMENT"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,52,427,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,1,0,5,0,80,8,0,10,0,12,0,83,9,0,1,0,
  	3,0,86,8,0,1,0,1,0,1,1,1,1,3,1,92,8,1,1,2,1,2,1,2,1,2,3,2,98,8,2,1,2,
  	1,2,5,2,102,8,2,10,2,12,2,105,9,2,1,2,1,2,1,3,1,3,1,3,1,3,1,3,1,3,1,3,
  	3,3,116,8,3,1,4,1,4,1,4,1,4,1,4,3,4,123,8,4,1,4,1,4,1,5,1,5,1,5,1,5,1,
  	5,1,5,1,5,1,5,1,6,1,6,1,6,5,6,138,8,6,10,6,12,6,141,9,6,1,6,1,6,1,7,1,
  	7,1,7,1,7,3,7,149,8,7,1,7,1,7,1,8,1,8,1,8,1,8,3,8,157,8,8,1,8,3,8,160,
  	8,8,1,8,1,8,1,9,1,9,1,9,5,9,167,8,9,10,9,12,9,170,9,9,1,10,1,10,1,10,
  	1,11,1,11,1,11,1,11,3,11,179,8,11,5,11,181,8,11,10,11,12,11,184,9,11,
  	1,11,1,11,1,12,1,12,1,12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,3,13,
  	199,8,13,5,13,201,8,13,10,13,12,13,204,9,13,1,13,1,13,1,14,1,14,1,14,
  	1,14,1,15,1,15,1,15,5,15,215,8,15,10,15,12,15,218,9,15,1,16,1,16,5,16,
  	222,8,16,10,16,12,16,225,9,16,1,16,3,16,228,8,16,1,17,1,17,1,17,5,17,
  	233,8,17,10,17,12,17,236,9,17,1,17,3,17,239,8,17,1,17,1,17,1,18,1,18,
  	1,18,1,18,3,18,247,8,18,1,19,1,19,1,19,1,19,3,19,253,8,19,1,20,1,20,1,
  	20,1,20,1,20,1,21,1,21,1,21,1,21,1,21,1,21,5,21,266,8,21,10,21,12,21,
  	269,9,21,1,21,1,21,1,22,1,22,1,23,1,23,3,23,277,8,23,1,23,1,23,5,23,281,
  	8,23,10,23,12,23,284,9,23,1,23,1,23,1,24,1,24,3,24,290,8,24,1,24,1,24,
  	3,24,294,8,24,1,24,1,24,3,24,298,8,24,1,25,1,25,1,25,1,25,1,26,1,26,1,
  	26,1,26,1,26,1,27,1,27,1,27,3,27,312,8,27,1,27,3,27,315,8,27,1,27,1,27,
  	1,27,1,27,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,3,28,329,8,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,3,28,341,8,28,1,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,
  	1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,1,28,
  	3,28,373,8,28,1,28,5,28,376,8,28,10,28,12,28,379,9,28,1,29,1,29,1,29,
  	3,29,384,8,29,1,30,1,30,1,30,5,30,389,8,30,10,30,12,30,392,9,30,1,31,
  	1,31,1,31,1,31,1,31,3,31,399,8,31,1,32,1,32,1,33,1,33,1,34,1,34,1,35,
  	5,35,408,8,35,10,35,12,35,411,9,35,1,36,1,36,1,36,1,36,1,36,3,36,418,
  	8,36,1,37,1,37,3,37,422,8,37,1,38,1,38,1,38,1,38,1,409,1,56,39,0,2,4,
  	6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,
  	54,56,58,60,62,64,66,68,70,72,74,76,0,7,1,0,44,45,1,0,46,49,1,0,37,38,
  	1,0,39,40,1,0,15,16,1,0,28,28,1,0,22,23,453,0,81,1,0,0,0,2,91,1,0,0,0,
  	4,93,1,0,0,0,6,115,1,0,0,0,8,117,1,0,0,0,10,126,1,0,0,0,12,134,1,0,0,
  	0,14,144,1,0,0,0,16,152,1,0,0,0,18,163,1,0,0,0,20,171,1,0,0,0,22,174,
  	1,0,0,0,24,187,1,0,0,0,26,192,1,0,0,0,28,207,1,0,0,0,30,211,1,0,0,0,32,
  	227,1,0,0,0,34,229,1,0,0,0,36,242,1,0,0,0,38,248,1,0,0,0,40,254,1,0,0,
  	0,42,259,1,0,0,0,44,272,1,0,0,0,46,274,1,0,0,0,48,297,1,0,0,0,50,299,
  	1,0,0,0,52,303,1,0,0,0,54,308,1,0,0,0,56,340,1,0,0,0,58,383,1,0,0,0,60,
  	385,1,0,0,0,62,398,1,0,0,0,64,400,1,0,0,0,66,402,1,0,0,0,68,404,1,0,0,
  	0,70,409,1,0,0,0,72,417,1,0,0,0,74,419,1,0,0,0,76,423,1,0,0,0,78,80,3,
  	2,1,0,79,78,1,0,0,0,80,83,1,0,0,0,81,79,1,0,0,0,81,82,1,0,0,0,82,85,1,
  	0,0,0,83,81,1,0,0,0,84,86,3,42,21,0,85,84,1,0,0,0,85,86,1,0,0,0,86,87,
  	1,0,0,0,87,88,5,0,0,1,88,1,1,0,0,0,89,92,3,4,2,0,90,92,3,26,13,0,91,89,
  	1,0,0,0,91,90,1,0,0,0,92,3,1,0,0,0,93,94,5,1,0,0,94,97,5,19,0,0,95,96,
  	5,30,0,0,96,98,5,19,0,0,97,95,1,0,0,0,97,98,1,0,0,0,98,99,1,0,0,0,99,
  	103,5,27,0,0,100,102,3,6,3,0,101,100,1,0,0,0,102,105,1,0,0,0,103,101,
  	1,0,0,0,103,104,1,0,0,0,104,106,1,0,0,0,105,103,1,0,0,0,106,107,5,28,
  	0,0,107,5,1,0,0,0,108,116,3,8,4,0,109,116,3,10,5,0,110,116,3,12,6,0,111,
  	116,3,16,8,0,112,116,3,22,11,0,113,116,3,24,12,0,114,116,3,46,23,0,115,
  	108,1,0,0,0,115,109,1,0,0,0,115,110,1,0,0,0,115,111,1,0,0,0,115,112,1,
  	0,0,0,115,113,1,0,0,0,115,114,1,0,0,0,116,7,1,0,0,0,117,118,5,2,0,0,118,
  	119,3,68,34,0,119,122,5,19,0,0,120,121,5,32,0,0,121,123,3,56,28,0,122,
  	120,1,0,0,0,122,123,1,0,0,0,123,124,1,0,0,0,124,125,5,29,0,0,125,9,1,
  	0,0,0,126,127,5,3,0,0,127,128,5,2,0,0,128,129,3,68,34,0,129,130,5,19,
  	0,0,130,131,5,35,0,0,131,132,3,56,28,0,132,133,5,29,0,0,133,11,1,0,0,
  	0,134,135,5,4,0,0,135,139,5,27,0,0,136,138,3,14,7,0,137,136,1,0,0,0,138,
  	141,1,0,0,0,139,137,1,0,0,0,139,140,1,0,0,0,140,142,1,0,0,0,141,139,1,
  	0,0,0,142,143,5,28,0,0,143,13,1,0,0,0,144,145,3,68,34,0,145,148,5,19,
  	0,0,146,147,5,32,0,0,147,149,3,56,28,0,148,146,1,0,0,0,148,149,1,0,0,
  	0,149,150,1,0,0,0,150,151,5,29,0,0,151,15,1,0,0,0,152,153,5,5,0,0,153,
  	159,5,19,0,0,154,156,5,25,0,0,155,157,3,18,9,0,156,155,1,0,0,0,156,157,
  	1,0,0,0,157,158,1,0,0,0,158,160,5,26,0,0,159,154,1,0,0,0,159,160,1,0,
  	0,0,160,161,1,0,0,0,161,162,5,29,0,0,162,17,1,0,0,0,163,168,3,20,10,0,
  	164,165,5,33,0,0,165,167,3,20,10,0,166,164,1,0,0,0,167,170,1,0,0,0,168,
  	166,1,0,0,0,168,169,1,0,0,0,169,19,1,0,0,0,170,168,1,0,0,0,171,172,3,
  	68,34,0,172,173,5,19,0,0,173,21,1,0,0,0,174,175,5,6,0,0,175,182,5,27,
  	0,0,176,178,3,28,14,0,177,179,5,29,0,0,178,177,1,0,0,0,178,179,1,0,0,
  	0,179,181,1,0,0,0,180,176,1,0,0,0,181,184,1,0,0,0,182,180,1,0,0,0,182,
  	183,1,0,0,0,183,185,1,0,0,0,184,182,1,0,0,0,185,186,5,28,0,0,186,23,1,
  	0,0,0,187,188,5,10,0,0,188,189,5,27,0,0,189,190,3,70,35,0,190,191,5,28,
  	0,0,191,25,1,0,0,0,192,193,5,19,0,0,193,194,5,7,0,0,194,195,5,6,0,0,195,
  	202,5,27,0,0,196,198,3,28,14,0,197,199,5,29,0,0,198,197,1,0,0,0,198,199,
  	1,0,0,0,199,201,1,0,0,0,200,196,1,0,0,0,201,204,1,0,0,0,202,200,1,0,0,
  	0,202,203,1,0,0,0,203,205,1,0,0,0,204,202,1,0,0,0,205,206,5,28,0,0,206,
  	27,1,0,0,0,207,208,3,30,15,0,208,209,5,30,0,0,209,210,3,32,16,0,210,29,
  	1,0,0,0,211,216,5,19,0,0,212,213,5,37,0,0,213,215,5,19,0,0,214,212,1,
  	0,0,0,215,218,1,0,0,0,216,214,1,0,0,0,216,217,1,0,0,0,217,31,1,0,0,0,
  	218,216,1,0,0,0,219,223,3,58,29,0,220,222,3,58,29,0,221,220,1,0,0,0,222,
  	225,1,0,0,0,223,221,1,0,0,0,223,224,1,0,0,0,224,228,1,0,0,0,225,223,1,
  	0,0,0,226,228,3,56,28,0,227,219,1,0,0,0,227,226,1,0,0,0,228,33,1,0,0,
  	0,229,230,5,8,0,0,230,234,5,27,0,0,231,233,3,36,18,0,232,231,1,0,0,0,
  	233,236,1,0,0,0,234,232,1,0,0,0,234,235,1,0,0,0,235,238,1,0,0,0,236,234,
  	1,0,0,0,237,239,3,38,19,0,238,237,1,0,0,0,238,239,1,0,0,0,239,240,1,0,
  	0,0,240,241,5,28,0,0,241,35,1,0,0,0,242,243,3,56,28,0,243,244,5,30,0,
  	0,244,246,3,56,28,0,245,247,5,33,0,0,246,245,1,0,0,0,246,247,1,0,0,0,
  	247,37,1,0,0,0,248,249,5,9,0,0,249,250,5,30,0,0,250,252,3,56,28,0,251,
  	253,5,33,0,0,252,251,1,0,0,0,252,253,1,0,0,0,253,39,1,0,0,0,254,255,5,
  	17,0,0,255,256,5,27,0,0,256,257,3,70,35,0,257,258,5,28,0,0,258,41,1,0,
  	0,0,259,260,5,13,0,0,260,261,5,14,0,0,261,262,5,25,0,0,262,263,5,26,0,
  	0,263,267,5,27,0,0,264,266,3,44,22,0,265,264,1,0,0,0,266,269,1,0,0,0,
  	267,265,1,0,0,0,267,268,1,0,0,0,268,270,1,0,0,0,269,267,1,0,0,0,270,271,
  	5,28,0,0,271,43,1,0,0,0,272,273,3,46,23,0,273,45,1,0,0,0,274,276,5,19,
  	0,0,275,277,5,19,0,0,276,275,1,0,0,0,276,277,1,0,0,0,277,278,1,0,0,0,
  	278,282,5,27,0,0,279,281,3,48,24,0,280,279,1,0,0,0,281,284,1,0,0,0,282,
  	280,1,0,0,0,282,283,1,0,0,0,283,285,1,0,0,0,284,282,1,0,0,0,285,286,5,
  	28,0,0,286,47,1,0,0,0,287,289,3,50,25,0,288,290,5,29,0,0,289,288,1,0,
  	0,0,289,290,1,0,0,0,290,298,1,0,0,0,291,293,3,52,26,0,292,294,5,29,0,
  	0,293,292,1,0,0,0,293,294,1,0,0,0,294,298,1,0,0,0,295,298,3,22,11,0,296,
  	298,3,46,23,0,297,287,1,0,0,0,297,291,1,0,0,0,297,295,1,0,0,0,297,296,
  	1,0,0,0,298,49,1,0,0,0,299,300,5,19,0,0,300,301,5,30,0,0,301,302,3,56,
  	28,0,302,51,1,0,0,0,303,304,5,18,0,0,304,305,5,19,0,0,305,306,5,30,0,
  	0,306,307,3,54,27,0,307,53,1,0,0,0,308,314,5,12,0,0,309,311,5,25,0,0,
  	310,312,3,18,9,0,311,310,1,0,0,0,311,312,1,0,0,0,312,313,1,0,0,0,313,
  	315,5,26,0,0,314,309,1,0,0,0,314,315,1,0,0,0,315,316,1,0,0,0,316,317,
  	5,27,0,0,317,318,3,70,35,0,318,319,5,28,0,0,319,55,1,0,0,0,320,321,6,
  	28,-1,0,321,322,5,43,0,0,322,341,3,56,28,10,323,324,5,19,0,0,324,325,
  	5,36,0,0,325,326,5,19,0,0,326,328,5,25,0,0,327,329,3,60,30,0,328,327,
  	1,0,0,0,328,329,1,0,0,0,329,330,1,0,0,0,330,341,5,26,0,0,331,341,3,34,
  	17,0,332,341,3,40,20,0,333,341,3,54,27,0,334,341,3,58,29,0,335,336,5,
  	25,0,0,336,337,3,56,28,0,337,338,5,26,0,0,338,341,1,0,0,0,339,341,5,11,
  	0,0,340,320,1,0,0,0,340,323,1,0,0,0,340,331,1,0,0,0,340,332,1,0,0,0,340,
  	333,1,0,0,0,340,334,1,0,0,0,340,335,1,0,0,0,340,339,1,0,0,0,341,377,1,
  	0,0,0,342,343,10,17,0,0,343,344,5,31,0,0,344,345,3,56,28,0,345,346,5,
  	30,0,0,346,347,3,56,28,18,347,376,1,0,0,0,348,349,10,16,0,0,349,350,5,
  	42,0,0,350,376,3,56,28,17,351,352,10,15,0,0,352,353,5,41,0,0,353,376,
  	3,56,28,16,354,355,10,14,0,0,355,356,7,0,0,0,356,376,3,56,28,15,357,358,
  	10,13,0,0,358,359,7,1,0,0,359,376,3,56,28,14,360,361,10,12,0,0,361,362,
  	7,2,0,0,362,376,3,56,28,13,363,364,10,11,0,0,364,365,7,3,0,0,365,376,
  	3,56,28,12,366,367,10,9,0,0,367,368,5,34,0,0,368,376,5,19,0,0,369,370,
  	10,8,0,0,370,372,5,25,0,0,371,373,3,60,30,0,372,371,1,0,0,0,372,373,1,
  	0,0,0,373,374,1,0,0,0,374,376,5,26,0,0,375,342,1,0,0,0,375,348,1,0,0,
  	0,375,351,1,0,0,0,375,354,1,0,0,0,375,357,1,0,0,0,375,360,1,0,0,0,375,
  	363,1,0,0,0,375,366,1,0,0,0,375,369,1,0,0,0,376,379,1,0,0,0,377,375,1,
  	0,0,0,377,378,1,0,0,0,378,57,1,0,0,0,379,377,1,0,0,0,380,384,3,62,31,
  	0,381,384,3,66,33,0,382,384,5,11,0,0,383,380,1,0,0,0,383,381,1,0,0,0,
  	383,382,1,0,0,0,384,59,1,0,0,0,385,390,3,56,28,0,386,387,5,33,0,0,387,
  	389,3,56,28,0,388,386,1,0,0,0,389,392,1,0,0,0,390,388,1,0,0,0,390,391,
  	1,0,0,0,391,61,1,0,0,0,392,390,1,0,0,0,393,399,5,20,0,0,394,399,5,21,
  	0,0,395,399,3,64,32,0,396,399,3,74,37,0,397,399,3,76,38,0,398,393,1,0,
  	0,0,398,394,1,0,0,0,398,395,1,0,0,0,398,396,1,0,0,0,398,397,1,0,0,0,399,
  	63,1,0,0,0,400,401,7,4,0,0,401,65,1,0,0,0,402,403,5,19,0,0,403,67,1,0,
  	0,0,404,405,5,19,0,0,405,69,1,0,0,0,406,408,3,72,36,0,407,406,1,0,0,0,
  	408,411,1,0,0,0,409,410,1,0,0,0,409,407,1,0,0,0,410,71,1,0,0,0,411,409,
  	1,0,0,0,412,413,5,27,0,0,413,414,3,70,35,0,414,415,5,28,0,0,415,418,1,
  	0,0,0,416,418,8,5,0,0,417,412,1,0,0,0,417,416,1,0,0,0,418,73,1,0,0,0,
  	419,421,5,20,0,0,420,422,7,6,0,0,421,420,1,0,0,0,421,422,1,0,0,0,422,
  	75,1,0,0,0,423,424,5,20,0,0,424,425,5,24,0,0,425,77,1,0,0,0,42,81,85,
  	91,97,103,115,122,139,148,156,159,168,178,182,198,202,216,223,227,234,
  	238,246,252,267,276,282,289,293,297,311,314,328,340,372,375,377,383,390,
  	398,409,417,421
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  myceliumuiParserStaticData = std::move(staticData);
}

}

MyceliumUIParser::MyceliumUIParser(TokenStream *input) : MyceliumUIParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

MyceliumUIParser::MyceliumUIParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  MyceliumUIParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *myceliumuiParserStaticData->atn, myceliumuiParserStaticData->decisionToDFA, myceliumuiParserStaticData->sharedContextCache, options);
}

MyceliumUIParser::~MyceliumUIParser() {
  delete _interpreter;
}

const atn::ATN& MyceliumUIParser::getATN() const {
  return *myceliumuiParserStaticData->atn;
}

std::string MyceliumUIParser::getGrammarFileName() const {
  return "MyceliumUI.g4";
}

const std::vector<std::string>& MyceliumUIParser::getRuleNames() const {
  return myceliumuiParserStaticData->ruleNames;
}

const dfa::Vocabulary& MyceliumUIParser::getVocabulary() const {
  return myceliumuiParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MyceliumUIParser::getSerializedATN() const {
  return myceliumuiParserStaticData->serializedATN;
}


//----------------- ProgramContext ------------------------------------------------------------------

MyceliumUIParser::ProgramContext::ProgramContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::ProgramContext::EOF() {
  return getToken(MyceliumUIParser::EOF, 0);
}

std::vector<MyceliumUIParser::DefinitionContext *> MyceliumUIParser::ProgramContext::definition() {
  return getRuleContexts<MyceliumUIParser::DefinitionContext>();
}

MyceliumUIParser::DefinitionContext* MyceliumUIParser::ProgramContext::definition(size_t i) {
  return getRuleContext<MyceliumUIParser::DefinitionContext>(i);
}

MyceliumUIParser::MainFunctionContext* MyceliumUIParser::ProgramContext::mainFunction() {
  return getRuleContext<MyceliumUIParser::MainFunctionContext>(0);
}


size_t MyceliumUIParser::ProgramContext::getRuleIndex() const {
  return MyceliumUIParser::RuleProgram;
}

void MyceliumUIParser::ProgramContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgram(this);
}

void MyceliumUIParser::ProgramContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgram(this);
}


std::any MyceliumUIParser::ProgramContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitProgram(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ProgramContext* MyceliumUIParser::program() {
  ProgramContext *_localctx = _tracker.createInstance<ProgramContext>(_ctx, getState());
  enterRule(_localctx, 0, MyceliumUIParser::RuleProgram);
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
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::COMPONENT

    || _la == MyceliumUIParser::ID) {
      setState(78);
      definition();
      setState(83);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(85);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::VOID) {
      setState(84);
      mainFunction();
    }
    setState(87);
    match(MyceliumUIParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DefinitionContext ------------------------------------------------------------------

MyceliumUIParser::DefinitionContext::DefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::ComponentDefinitionContext* MyceliumUIParser::DefinitionContext::componentDefinition() {
  return getRuleContext<MyceliumUIParser::ComponentDefinitionContext>(0);
}

MyceliumUIParser::StyleOverrideContext* MyceliumUIParser::DefinitionContext::styleOverride() {
  return getRuleContext<MyceliumUIParser::StyleOverrideContext>(0);
}


size_t MyceliumUIParser::DefinitionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleDefinition;
}

void MyceliumUIParser::DefinitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefinition(this);
}

void MyceliumUIParser::DefinitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefinition(this);
}


std::any MyceliumUIParser::DefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitDefinition(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::DefinitionContext* MyceliumUIParser::definition() {
  DefinitionContext *_localctx = _tracker.createInstance<DefinitionContext>(_ctx, getState());
  enterRule(_localctx, 2, MyceliumUIParser::RuleDefinition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(91);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MyceliumUIParser::COMPONENT: {
        enterOuterAlt(_localctx, 1);
        setState(89);
        componentDefinition();
        break;
      }

      case MyceliumUIParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(90);
        styleOverride();
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

//----------------- ComponentDefinitionContext ------------------------------------------------------------------

MyceliumUIParser::ComponentDefinitionContext::ComponentDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::ComponentDefinitionContext::COMPONENT() {
  return getToken(MyceliumUIParser::COMPONENT, 0);
}

tree::TerminalNode* MyceliumUIParser::ComponentDefinitionContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::ComponentDefinitionContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::ComponentDefinitionContext::ID() {
  return getTokens(MyceliumUIParser::ID);
}

tree::TerminalNode* MyceliumUIParser::ComponentDefinitionContext::ID(size_t i) {
  return getToken(MyceliumUIParser::ID, i);
}

tree::TerminalNode* MyceliumUIParser::ComponentDefinitionContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

std::vector<MyceliumUIParser::ComponentMemberContext *> MyceliumUIParser::ComponentDefinitionContext::componentMember() {
  return getRuleContexts<MyceliumUIParser::ComponentMemberContext>();
}

MyceliumUIParser::ComponentMemberContext* MyceliumUIParser::ComponentDefinitionContext::componentMember(size_t i) {
  return getRuleContext<MyceliumUIParser::ComponentMemberContext>(i);
}


size_t MyceliumUIParser::ComponentDefinitionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleComponentDefinition;
}

void MyceliumUIParser::ComponentDefinitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComponentDefinition(this);
}

void MyceliumUIParser::ComponentDefinitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComponentDefinition(this);
}


std::any MyceliumUIParser::ComponentDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitComponentDefinition(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ComponentDefinitionContext* MyceliumUIParser::componentDefinition() {
  ComponentDefinitionContext *_localctx = _tracker.createInstance<ComponentDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 4, MyceliumUIParser::RuleComponentDefinition);
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
    setState(93);
    match(MyceliumUIParser::COMPONENT);
    setState(94);
    antlrcpp::downCast<ComponentDefinitionContext *>(_localctx)->componentName = match(MyceliumUIParser::ID);
    setState(97);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::COLON) {
      setState(95);
      match(MyceliumUIParser::COLON);
      setState(96);
      antlrcpp::downCast<ComponentDefinitionContext *>(_localctx)->baseComponentName = match(MyceliumUIParser::ID);
    }
    setState(99);
    match(MyceliumUIParser::LBRACE);
    setState(103);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 525436) != 0)) {
      setState(100);
      componentMember();
      setState(105);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(106);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ComponentMemberContext ------------------------------------------------------------------

MyceliumUIParser::ComponentMemberContext::ComponentMemberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::PropDeclContext* MyceliumUIParser::ComponentMemberContext::propDecl() {
  return getRuleContext<MyceliumUIParser::PropDeclContext>(0);
}

MyceliumUIParser::GetPropDeclContext* MyceliumUIParser::ComponentMemberContext::getPropDecl() {
  return getRuleContext<MyceliumUIParser::GetPropDeclContext>(0);
}

MyceliumUIParser::StateBlockContext* MyceliumUIParser::ComponentMemberContext::stateBlock() {
  return getRuleContext<MyceliumUIParser::StateBlockContext>(0);
}

MyceliumUIParser::EventDeclContext* MyceliumUIParser::ComponentMemberContext::eventDecl() {
  return getRuleContext<MyceliumUIParser::EventDeclContext>(0);
}

MyceliumUIParser::StyleBlockContext* MyceliumUIParser::ComponentMemberContext::styleBlock() {
  return getRuleContext<MyceliumUIParser::StyleBlockContext>(0);
}

MyceliumUIParser::ScriptBlockContext* MyceliumUIParser::ComponentMemberContext::scriptBlock() {
  return getRuleContext<MyceliumUIParser::ScriptBlockContext>(0);
}

MyceliumUIParser::InstanceDefinitionContext* MyceliumUIParser::ComponentMemberContext::instanceDefinition() {
  return getRuleContext<MyceliumUIParser::InstanceDefinitionContext>(0);
}


size_t MyceliumUIParser::ComponentMemberContext::getRuleIndex() const {
  return MyceliumUIParser::RuleComponentMember;
}

void MyceliumUIParser::ComponentMemberContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComponentMember(this);
}

void MyceliumUIParser::ComponentMemberContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComponentMember(this);
}


std::any MyceliumUIParser::ComponentMemberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitComponentMember(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ComponentMemberContext* MyceliumUIParser::componentMember() {
  ComponentMemberContext *_localctx = _tracker.createInstance<ComponentMemberContext>(_ctx, getState());
  enterRule(_localctx, 6, MyceliumUIParser::RuleComponentMember);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(115);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MyceliumUIParser::PROP: {
        enterOuterAlt(_localctx, 1);
        setState(108);
        propDecl();
        break;
      }

      case MyceliumUIParser::GET: {
        enterOuterAlt(_localctx, 2);
        setState(109);
        getPropDecl();
        break;
      }

      case MyceliumUIParser::STATE: {
        enterOuterAlt(_localctx, 3);
        setState(110);
        stateBlock();
        break;
      }

      case MyceliumUIParser::EVENT: {
        enterOuterAlt(_localctx, 4);
        setState(111);
        eventDecl();
        break;
      }

      case MyceliumUIParser::STYLE: {
        enterOuterAlt(_localctx, 5);
        setState(112);
        styleBlock();
        break;
      }

      case MyceliumUIParser::SCRIPT: {
        enterOuterAlt(_localctx, 6);
        setState(113);
        scriptBlock();
        break;
      }

      case MyceliumUIParser::ID: {
        enterOuterAlt(_localctx, 7);
        setState(114);
        instanceDefinition();
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

//----------------- PropDeclContext ------------------------------------------------------------------

MyceliumUIParser::PropDeclContext::PropDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::PropDeclContext::PROP() {
  return getToken(MyceliumUIParser::PROP, 0);
}

MyceliumUIParser::TypeContext* MyceliumUIParser::PropDeclContext::type() {
  return getRuleContext<MyceliumUIParser::TypeContext>(0);
}

tree::TerminalNode* MyceliumUIParser::PropDeclContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::PropDeclContext::SEMI() {
  return getToken(MyceliumUIParser::SEMI, 0);
}

tree::TerminalNode* MyceliumUIParser::PropDeclContext::ASSIGN() {
  return getToken(MyceliumUIParser::ASSIGN, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::PropDeclContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}


size_t MyceliumUIParser::PropDeclContext::getRuleIndex() const {
  return MyceliumUIParser::RulePropDecl;
}

void MyceliumUIParser::PropDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPropDecl(this);
}

void MyceliumUIParser::PropDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPropDecl(this);
}


std::any MyceliumUIParser::PropDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitPropDecl(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::PropDeclContext* MyceliumUIParser::propDecl() {
  PropDeclContext *_localctx = _tracker.createInstance<PropDeclContext>(_ctx, getState());
  enterRule(_localctx, 8, MyceliumUIParser::RulePropDecl);
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
    setState(117);
    match(MyceliumUIParser::PROP);
    setState(118);
    type();
    setState(119);
    match(MyceliumUIParser::ID);
    setState(122);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::ASSIGN) {
      setState(120);
      match(MyceliumUIParser::ASSIGN);
      setState(121);
      expression(0);
    }
    setState(124);
    match(MyceliumUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GetPropDeclContext ------------------------------------------------------------------

MyceliumUIParser::GetPropDeclContext::GetPropDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::GetPropDeclContext::GET() {
  return getToken(MyceliumUIParser::GET, 0);
}

tree::TerminalNode* MyceliumUIParser::GetPropDeclContext::PROP() {
  return getToken(MyceliumUIParser::PROP, 0);
}

MyceliumUIParser::TypeContext* MyceliumUIParser::GetPropDeclContext::type() {
  return getRuleContext<MyceliumUIParser::TypeContext>(0);
}

tree::TerminalNode* MyceliumUIParser::GetPropDeclContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::GetPropDeclContext::ARROW() {
  return getToken(MyceliumUIParser::ARROW, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::GetPropDeclContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MyceliumUIParser::GetPropDeclContext::SEMI() {
  return getToken(MyceliumUIParser::SEMI, 0);
}


size_t MyceliumUIParser::GetPropDeclContext::getRuleIndex() const {
  return MyceliumUIParser::RuleGetPropDecl;
}

void MyceliumUIParser::GetPropDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGetPropDecl(this);
}

void MyceliumUIParser::GetPropDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGetPropDecl(this);
}


std::any MyceliumUIParser::GetPropDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitGetPropDecl(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::GetPropDeclContext* MyceliumUIParser::getPropDecl() {
  GetPropDeclContext *_localctx = _tracker.createInstance<GetPropDeclContext>(_ctx, getState());
  enterRule(_localctx, 10, MyceliumUIParser::RuleGetPropDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(126);
    match(MyceliumUIParser::GET);
    setState(127);
    match(MyceliumUIParser::PROP);
    setState(128);
    type();
    setState(129);
    match(MyceliumUIParser::ID);
    setState(130);
    match(MyceliumUIParser::ARROW);
    setState(131);
    expression(0);
    setState(132);
    match(MyceliumUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateBlockContext ------------------------------------------------------------------

MyceliumUIParser::StateBlockContext::StateBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::StateBlockContext::STATE() {
  return getToken(MyceliumUIParser::STATE, 0);
}

tree::TerminalNode* MyceliumUIParser::StateBlockContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::StateBlockContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<MyceliumUIParser::StateVarDeclContext *> MyceliumUIParser::StateBlockContext::stateVarDecl() {
  return getRuleContexts<MyceliumUIParser::StateVarDeclContext>();
}

MyceliumUIParser::StateVarDeclContext* MyceliumUIParser::StateBlockContext::stateVarDecl(size_t i) {
  return getRuleContext<MyceliumUIParser::StateVarDeclContext>(i);
}


size_t MyceliumUIParser::StateBlockContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStateBlock;
}

void MyceliumUIParser::StateBlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStateBlock(this);
}

void MyceliumUIParser::StateBlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStateBlock(this);
}


std::any MyceliumUIParser::StateBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStateBlock(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StateBlockContext* MyceliumUIParser::stateBlock() {
  StateBlockContext *_localctx = _tracker.createInstance<StateBlockContext>(_ctx, getState());
  enterRule(_localctx, 12, MyceliumUIParser::RuleStateBlock);
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
    setState(134);
    match(MyceliumUIParser::STATE);
    setState(135);
    match(MyceliumUIParser::LBRACE);
    setState(139);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::ID) {
      setState(136);
      stateVarDecl();
      setState(141);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(142);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StateVarDeclContext ------------------------------------------------------------------

MyceliumUIParser::StateVarDeclContext::StateVarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::TypeContext* MyceliumUIParser::StateVarDeclContext::type() {
  return getRuleContext<MyceliumUIParser::TypeContext>(0);
}

tree::TerminalNode* MyceliumUIParser::StateVarDeclContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::StateVarDeclContext::SEMI() {
  return getToken(MyceliumUIParser::SEMI, 0);
}

tree::TerminalNode* MyceliumUIParser::StateVarDeclContext::ASSIGN() {
  return getToken(MyceliumUIParser::ASSIGN, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::StateVarDeclContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}


size_t MyceliumUIParser::StateVarDeclContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStateVarDecl;
}

void MyceliumUIParser::StateVarDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStateVarDecl(this);
}

void MyceliumUIParser::StateVarDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStateVarDecl(this);
}


std::any MyceliumUIParser::StateVarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStateVarDecl(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StateVarDeclContext* MyceliumUIParser::stateVarDecl() {
  StateVarDeclContext *_localctx = _tracker.createInstance<StateVarDeclContext>(_ctx, getState());
  enterRule(_localctx, 14, MyceliumUIParser::RuleStateVarDecl);
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
    setState(144);
    type();
    setState(145);
    match(MyceliumUIParser::ID);
    setState(148);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::ASSIGN) {
      setState(146);
      match(MyceliumUIParser::ASSIGN);
      setState(147);
      expression(0);
    }
    setState(150);
    match(MyceliumUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EventDeclContext ------------------------------------------------------------------

MyceliumUIParser::EventDeclContext::EventDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::EventDeclContext::EVENT() {
  return getToken(MyceliumUIParser::EVENT, 0);
}

tree::TerminalNode* MyceliumUIParser::EventDeclContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::EventDeclContext::SEMI() {
  return getToken(MyceliumUIParser::SEMI, 0);
}

tree::TerminalNode* MyceliumUIParser::EventDeclContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::EventDeclContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

MyceliumUIParser::ParamListContext* MyceliumUIParser::EventDeclContext::paramList() {
  return getRuleContext<MyceliumUIParser::ParamListContext>(0);
}


size_t MyceliumUIParser::EventDeclContext::getRuleIndex() const {
  return MyceliumUIParser::RuleEventDecl;
}

void MyceliumUIParser::EventDeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEventDecl(this);
}

void MyceliumUIParser::EventDeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEventDecl(this);
}


std::any MyceliumUIParser::EventDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitEventDecl(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::EventDeclContext* MyceliumUIParser::eventDecl() {
  EventDeclContext *_localctx = _tracker.createInstance<EventDeclContext>(_ctx, getState());
  enterRule(_localctx, 16, MyceliumUIParser::RuleEventDecl);
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
    setState(152);
    match(MyceliumUIParser::EVENT);
    setState(153);
    match(MyceliumUIParser::ID);
    setState(159);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::LPAREN) {
      setState(154);
      match(MyceliumUIParser::LPAREN);
      setState(156);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::ID) {
        setState(155);
        paramList();
      }
      setState(158);
      match(MyceliumUIParser::RPAREN);
    }
    setState(161);
    match(MyceliumUIParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParamListContext ------------------------------------------------------------------

MyceliumUIParser::ParamListContext::ParamListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MyceliumUIParser::ParamContext *> MyceliumUIParser::ParamListContext::param() {
  return getRuleContexts<MyceliumUIParser::ParamContext>();
}

MyceliumUIParser::ParamContext* MyceliumUIParser::ParamListContext::param(size_t i) {
  return getRuleContext<MyceliumUIParser::ParamContext>(i);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::ParamListContext::COMMA() {
  return getTokens(MyceliumUIParser::COMMA);
}

tree::TerminalNode* MyceliumUIParser::ParamListContext::COMMA(size_t i) {
  return getToken(MyceliumUIParser::COMMA, i);
}


size_t MyceliumUIParser::ParamListContext::getRuleIndex() const {
  return MyceliumUIParser::RuleParamList;
}

void MyceliumUIParser::ParamListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParamList(this);
}

void MyceliumUIParser::ParamListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParamList(this);
}


std::any MyceliumUIParser::ParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitParamList(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ParamListContext* MyceliumUIParser::paramList() {
  ParamListContext *_localctx = _tracker.createInstance<ParamListContext>(_ctx, getState());
  enterRule(_localctx, 18, MyceliumUIParser::RuleParamList);
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
    setState(163);
    param();
    setState(168);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::COMMA) {
      setState(164);
      match(MyceliumUIParser::COMMA);
      setState(165);
      param();
      setState(170);
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

//----------------- ParamContext ------------------------------------------------------------------

MyceliumUIParser::ParamContext::ParamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::TypeContext* MyceliumUIParser::ParamContext::type() {
  return getRuleContext<MyceliumUIParser::TypeContext>(0);
}

tree::TerminalNode* MyceliumUIParser::ParamContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}


size_t MyceliumUIParser::ParamContext::getRuleIndex() const {
  return MyceliumUIParser::RuleParam;
}

void MyceliumUIParser::ParamContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParam(this);
}

void MyceliumUIParser::ParamContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParam(this);
}


std::any MyceliumUIParser::ParamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitParam(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ParamContext* MyceliumUIParser::param() {
  ParamContext *_localctx = _tracker.createInstance<ParamContext>(_ctx, getState());
  enterRule(_localctx, 20, MyceliumUIParser::RuleParam);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(171);
    type();
    setState(172);
    match(MyceliumUIParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StyleBlockContext ------------------------------------------------------------------

MyceliumUIParser::StyleBlockContext::StyleBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::StyleBlockContext::STYLE() {
  return getToken(MyceliumUIParser::STYLE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleBlockContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleBlockContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<MyceliumUIParser::StyleAssignmentContext *> MyceliumUIParser::StyleBlockContext::styleAssignment() {
  return getRuleContexts<MyceliumUIParser::StyleAssignmentContext>();
}

MyceliumUIParser::StyleAssignmentContext* MyceliumUIParser::StyleBlockContext::styleAssignment(size_t i) {
  return getRuleContext<MyceliumUIParser::StyleAssignmentContext>(i);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::StyleBlockContext::SEMI() {
  return getTokens(MyceliumUIParser::SEMI);
}

tree::TerminalNode* MyceliumUIParser::StyleBlockContext::SEMI(size_t i) {
  return getToken(MyceliumUIParser::SEMI, i);
}


size_t MyceliumUIParser::StyleBlockContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStyleBlock;
}

void MyceliumUIParser::StyleBlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleBlock(this);
}

void MyceliumUIParser::StyleBlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleBlock(this);
}


std::any MyceliumUIParser::StyleBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleBlock(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StyleBlockContext* MyceliumUIParser::styleBlock() {
  StyleBlockContext *_localctx = _tracker.createInstance<StyleBlockContext>(_ctx, getState());
  enterRule(_localctx, 22, MyceliumUIParser::RuleStyleBlock);
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
    setState(174);
    match(MyceliumUIParser::STYLE);
    setState(175);
    match(MyceliumUIParser::LBRACE);
    setState(182);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::ID) {
      setState(176);
      styleAssignment();
      setState(178);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::SEMI) {
        setState(177);
        match(MyceliumUIParser::SEMI);
      }
      setState(184);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(185);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScriptBlockContext ------------------------------------------------------------------

MyceliumUIParser::ScriptBlockContext::ScriptBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::ScriptBlockContext::SCRIPT() {
  return getToken(MyceliumUIParser::SCRIPT, 0);
}

tree::TerminalNode* MyceliumUIParser::ScriptBlockContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

MyceliumUIParser::ScriptBodyContext* MyceliumUIParser::ScriptBlockContext::scriptBody() {
  return getRuleContext<MyceliumUIParser::ScriptBodyContext>(0);
}

tree::TerminalNode* MyceliumUIParser::ScriptBlockContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}


size_t MyceliumUIParser::ScriptBlockContext::getRuleIndex() const {
  return MyceliumUIParser::RuleScriptBlock;
}

void MyceliumUIParser::ScriptBlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterScriptBlock(this);
}

void MyceliumUIParser::ScriptBlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitScriptBlock(this);
}


std::any MyceliumUIParser::ScriptBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitScriptBlock(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ScriptBlockContext* MyceliumUIParser::scriptBlock() {
  ScriptBlockContext *_localctx = _tracker.createInstance<ScriptBlockContext>(_ctx, getState());
  enterRule(_localctx, 24, MyceliumUIParser::RuleScriptBlock);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(187);
    match(MyceliumUIParser::SCRIPT);
    setState(188);
    match(MyceliumUIParser::LBRACE);
    setState(189);
    scriptBody();
    setState(190);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StyleOverrideContext ------------------------------------------------------------------

MyceliumUIParser::StyleOverrideContext::StyleOverrideContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::OVERRIDE() {
  return getToken(MyceliumUIParser::OVERRIDE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::STYLE() {
  return getToken(MyceliumUIParser::STYLE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

std::vector<MyceliumUIParser::StyleAssignmentContext *> MyceliumUIParser::StyleOverrideContext::styleAssignment() {
  return getRuleContexts<MyceliumUIParser::StyleAssignmentContext>();
}

MyceliumUIParser::StyleAssignmentContext* MyceliumUIParser::StyleOverrideContext::styleAssignment(size_t i) {
  return getRuleContext<MyceliumUIParser::StyleAssignmentContext>(i);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::StyleOverrideContext::SEMI() {
  return getTokens(MyceliumUIParser::SEMI);
}

tree::TerminalNode* MyceliumUIParser::StyleOverrideContext::SEMI(size_t i) {
  return getToken(MyceliumUIParser::SEMI, i);
}


size_t MyceliumUIParser::StyleOverrideContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStyleOverride;
}

void MyceliumUIParser::StyleOverrideContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleOverride(this);
}

void MyceliumUIParser::StyleOverrideContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleOverride(this);
}


std::any MyceliumUIParser::StyleOverrideContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleOverride(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StyleOverrideContext* MyceliumUIParser::styleOverride() {
  StyleOverrideContext *_localctx = _tracker.createInstance<StyleOverrideContext>(_ctx, getState());
  enterRule(_localctx, 26, MyceliumUIParser::RuleStyleOverride);
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
    setState(192);
    antlrcpp::downCast<StyleOverrideContext *>(_localctx)->targetType = match(MyceliumUIParser::ID);
    setState(193);
    match(MyceliumUIParser::OVERRIDE);
    setState(194);
    match(MyceliumUIParser::STYLE);
    setState(195);
    match(MyceliumUIParser::LBRACE);
    setState(202);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::ID) {
      setState(196);
      styleAssignment();
      setState(198);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::SEMI) {
        setState(197);
        match(MyceliumUIParser::SEMI);
      }
      setState(204);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(205);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StyleAssignmentContext ------------------------------------------------------------------

MyceliumUIParser::StyleAssignmentContext::StyleAssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::StyleAssignmentContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

MyceliumUIParser::StyleValueContext* MyceliumUIParser::StyleAssignmentContext::styleValue() {
  return getRuleContext<MyceliumUIParser::StyleValueContext>(0);
}

MyceliumUIParser::StyleIdentifierContext* MyceliumUIParser::StyleAssignmentContext::styleIdentifier() {
  return getRuleContext<MyceliumUIParser::StyleIdentifierContext>(0);
}


size_t MyceliumUIParser::StyleAssignmentContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStyleAssignment;
}

void MyceliumUIParser::StyleAssignmentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleAssignment(this);
}

void MyceliumUIParser::StyleAssignmentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleAssignment(this);
}


std::any MyceliumUIParser::StyleAssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleAssignment(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StyleAssignmentContext* MyceliumUIParser::styleAssignment() {
  StyleAssignmentContext *_localctx = _tracker.createInstance<StyleAssignmentContext>(_ctx, getState());
  enterRule(_localctx, 28, MyceliumUIParser::RuleStyleAssignment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(207);
    antlrcpp::downCast<StyleAssignmentContext *>(_localctx)->propertyName = styleIdentifier();
    setState(208);
    match(MyceliumUIParser::COLON);
    setState(209);
    styleValue();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StyleIdentifierContext ------------------------------------------------------------------

MyceliumUIParser::StyleIdentifierContext::StyleIdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> MyceliumUIParser::StyleIdentifierContext::ID() {
  return getTokens(MyceliumUIParser::ID);
}

tree::TerminalNode* MyceliumUIParser::StyleIdentifierContext::ID(size_t i) {
  return getToken(MyceliumUIParser::ID, i);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::StyleIdentifierContext::MINUS() {
  return getTokens(MyceliumUIParser::MINUS);
}

tree::TerminalNode* MyceliumUIParser::StyleIdentifierContext::MINUS(size_t i) {
  return getToken(MyceliumUIParser::MINUS, i);
}


size_t MyceliumUIParser::StyleIdentifierContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStyleIdentifier;
}

void MyceliumUIParser::StyleIdentifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleIdentifier(this);
}

void MyceliumUIParser::StyleIdentifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleIdentifier(this);
}


std::any MyceliumUIParser::StyleIdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleIdentifier(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::StyleIdentifierContext* MyceliumUIParser::styleIdentifier() {
  StyleIdentifierContext *_localctx = _tracker.createInstance<StyleIdentifierContext>(_ctx, getState());
  enterRule(_localctx, 30, MyceliumUIParser::RuleStyleIdentifier);
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
    setState(211);
    match(MyceliumUIParser::ID);
    setState(216);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::MINUS) {
      setState(212);
      match(MyceliumUIParser::MINUS);
      setState(213);
      match(MyceliumUIParser::ID);
      setState(218);
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

//----------------- StyleValueContext ------------------------------------------------------------------

MyceliumUIParser::StyleValueContext::StyleValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t MyceliumUIParser::StyleValueContext::getRuleIndex() const {
  return MyceliumUIParser::RuleStyleValue;
}

void MyceliumUIParser::StyleValueContext::copyFrom(StyleValueContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- StyleValuePrimarySequenceContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::PrimaryContext *> MyceliumUIParser::StyleValuePrimarySequenceContext::primary() {
  return getRuleContexts<MyceliumUIParser::PrimaryContext>();
}

MyceliumUIParser::PrimaryContext* MyceliumUIParser::StyleValuePrimarySequenceContext::primary(size_t i) {
  return getRuleContext<MyceliumUIParser::PrimaryContext>(i);
}

MyceliumUIParser::StyleValuePrimarySequenceContext::StyleValuePrimarySequenceContext(StyleValueContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::StyleValuePrimarySequenceContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleValuePrimarySequence(this);
}
void MyceliumUIParser::StyleValuePrimarySequenceContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleValuePrimarySequence(this);
}

std::any MyceliumUIParser::StyleValuePrimarySequenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleValuePrimarySequence(this);
  else
    return visitor->visitChildren(this);
}
//----------------- StyleValueFullExpressionContext ------------------------------------------------------------------

MyceliumUIParser::ExpressionContext* MyceliumUIParser::StyleValueFullExpressionContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

MyceliumUIParser::StyleValueFullExpressionContext::StyleValueFullExpressionContext(StyleValueContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::StyleValueFullExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStyleValueFullExpression(this);
}
void MyceliumUIParser::StyleValueFullExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStyleValueFullExpression(this);
}

std::any MyceliumUIParser::StyleValueFullExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStyleValueFullExpression(this);
  else
    return visitor->visitChildren(this);
}
MyceliumUIParser::StyleValueContext* MyceliumUIParser::styleValue() {
  StyleValueContext *_localctx = _tracker.createInstance<StyleValueContext>(_ctx, getState());
  enterRule(_localctx, 32, MyceliumUIParser::RuleStyleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(227);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 18, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<MyceliumUIParser::StyleValuePrimarySequenceContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(219);
      primary();
      setState(223);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
      while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
        if (alt == 1) {
          setState(220);
          primary(); 
        }
        setState(225);
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx);
      }
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<MyceliumUIParser::StyleValueFullExpressionContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(226);
      expression(0);
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

//----------------- SelectExpressionContext ------------------------------------------------------------------

MyceliumUIParser::SelectExpressionContext::SelectExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::SelectExpressionContext::SELECT() {
  return getToken(MyceliumUIParser::SELECT, 0);
}

tree::TerminalNode* MyceliumUIParser::SelectExpressionContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::SelectExpressionContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<MyceliumUIParser::SelectCaseContext *> MyceliumUIParser::SelectExpressionContext::selectCase() {
  return getRuleContexts<MyceliumUIParser::SelectCaseContext>();
}

MyceliumUIParser::SelectCaseContext* MyceliumUIParser::SelectExpressionContext::selectCase(size_t i) {
  return getRuleContext<MyceliumUIParser::SelectCaseContext>(i);
}

MyceliumUIParser::DefaultCaseContext* MyceliumUIParser::SelectExpressionContext::defaultCase() {
  return getRuleContext<MyceliumUIParser::DefaultCaseContext>(0);
}


size_t MyceliumUIParser::SelectExpressionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleSelectExpression;
}

void MyceliumUIParser::SelectExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSelectExpression(this);
}

void MyceliumUIParser::SelectExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSelectExpression(this);
}


std::any MyceliumUIParser::SelectExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitSelectExpression(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::SelectExpressionContext* MyceliumUIParser::selectExpression() {
  SelectExpressionContext *_localctx = _tracker.createInstance<SelectExpressionContext>(_ctx, getState());
  enterRule(_localctx, 34, MyceliumUIParser::RuleSelectExpression);
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
    setState(229);
    match(MyceliumUIParser::SELECT);
    setState(230);
    match(MyceliumUIParser::LBRACE);
    setState(234);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 8796130482432) != 0)) {
      setState(231);
      selectCase();
      setState(236);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(238);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::DEFAULT) {
      setState(237);
      defaultCase();
    }
    setState(240);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SelectCaseContext ------------------------------------------------------------------

MyceliumUIParser::SelectCaseContext::SelectCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::SelectCaseContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::SelectCaseContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::SelectCaseContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

tree::TerminalNode* MyceliumUIParser::SelectCaseContext::COMMA() {
  return getToken(MyceliumUIParser::COMMA, 0);
}


size_t MyceliumUIParser::SelectCaseContext::getRuleIndex() const {
  return MyceliumUIParser::RuleSelectCase;
}

void MyceliumUIParser::SelectCaseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSelectCase(this);
}

void MyceliumUIParser::SelectCaseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSelectCase(this);
}


std::any MyceliumUIParser::SelectCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitSelectCase(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::SelectCaseContext* MyceliumUIParser::selectCase() {
  SelectCaseContext *_localctx = _tracker.createInstance<SelectCaseContext>(_ctx, getState());
  enterRule(_localctx, 36, MyceliumUIParser::RuleSelectCase);
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
    setState(242);
    expression(0);
    setState(243);
    match(MyceliumUIParser::COLON);
    setState(244);
    expression(0);
    setState(246);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::COMMA) {
      setState(245);
      match(MyceliumUIParser::COMMA);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DefaultCaseContext ------------------------------------------------------------------

MyceliumUIParser::DefaultCaseContext::DefaultCaseContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::DefaultCaseContext::DEFAULT() {
  return getToken(MyceliumUIParser::DEFAULT, 0);
}

tree::TerminalNode* MyceliumUIParser::DefaultCaseContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::DefaultCaseContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MyceliumUIParser::DefaultCaseContext::COMMA() {
  return getToken(MyceliumUIParser::COMMA, 0);
}


size_t MyceliumUIParser::DefaultCaseContext::getRuleIndex() const {
  return MyceliumUIParser::RuleDefaultCase;
}

void MyceliumUIParser::DefaultCaseContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefaultCase(this);
}

void MyceliumUIParser::DefaultCaseContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefaultCase(this);
}


std::any MyceliumUIParser::DefaultCaseContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitDefaultCase(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::DefaultCaseContext* MyceliumUIParser::defaultCase() {
  DefaultCaseContext *_localctx = _tracker.createInstance<DefaultCaseContext>(_ctx, getState());
  enterRule(_localctx, 38, MyceliumUIParser::RuleDefaultCase);
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
    setState(248);
    match(MyceliumUIParser::DEFAULT);
    setState(249);
    match(MyceliumUIParser::COLON);
    setState(250);
    expression(0);
    setState(252);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::COMMA) {
      setState(251);
      match(MyceliumUIParser::COMMA);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CalculateBlockContext ------------------------------------------------------------------

MyceliumUIParser::CalculateBlockContext::CalculateBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::CalculateBlockContext::CALCULATE() {
  return getToken(MyceliumUIParser::CALCULATE, 0);
}

tree::TerminalNode* MyceliumUIParser::CalculateBlockContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

MyceliumUIParser::ScriptBodyContext* MyceliumUIParser::CalculateBlockContext::scriptBody() {
  return getRuleContext<MyceliumUIParser::ScriptBodyContext>(0);
}

tree::TerminalNode* MyceliumUIParser::CalculateBlockContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}


size_t MyceliumUIParser::CalculateBlockContext::getRuleIndex() const {
  return MyceliumUIParser::RuleCalculateBlock;
}

void MyceliumUIParser::CalculateBlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCalculateBlock(this);
}

void MyceliumUIParser::CalculateBlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCalculateBlock(this);
}


std::any MyceliumUIParser::CalculateBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitCalculateBlock(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::CalculateBlockContext* MyceliumUIParser::calculateBlock() {
  CalculateBlockContext *_localctx = _tracker.createInstance<CalculateBlockContext>(_ctx, getState());
  enterRule(_localctx, 40, MyceliumUIParser::RuleCalculateBlock);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(254);
    match(MyceliumUIParser::CALCULATE);
    setState(255);
    match(MyceliumUIParser::LBRACE);
    setState(256);
    scriptBody();
    setState(257);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MainFunctionContext ------------------------------------------------------------------

MyceliumUIParser::MainFunctionContext::MainFunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::VOID() {
  return getToken(MyceliumUIParser::VOID, 0);
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::MAIN() {
  return getToken(MyceliumUIParser::MAIN, 0);
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::MainFunctionContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<MyceliumUIParser::ProgramStatementContext *> MyceliumUIParser::MainFunctionContext::programStatement() {
  return getRuleContexts<MyceliumUIParser::ProgramStatementContext>();
}

MyceliumUIParser::ProgramStatementContext* MyceliumUIParser::MainFunctionContext::programStatement(size_t i) {
  return getRuleContext<MyceliumUIParser::ProgramStatementContext>(i);
}


size_t MyceliumUIParser::MainFunctionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleMainFunction;
}

void MyceliumUIParser::MainFunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMainFunction(this);
}

void MyceliumUIParser::MainFunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMainFunction(this);
}


std::any MyceliumUIParser::MainFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitMainFunction(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::MainFunctionContext* MyceliumUIParser::mainFunction() {
  MainFunctionContext *_localctx = _tracker.createInstance<MainFunctionContext>(_ctx, getState());
  enterRule(_localctx, 42, MyceliumUIParser::RuleMainFunction);
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
    setState(259);
    match(MyceliumUIParser::VOID);
    setState(260);
    match(MyceliumUIParser::MAIN);
    setState(261);
    match(MyceliumUIParser::LPAREN);
    setState(262);
    match(MyceliumUIParser::RPAREN);
    setState(263);
    match(MyceliumUIParser::LBRACE);
    setState(267);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::ID) {
      setState(264);
      programStatement();
      setState(269);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(270);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramStatementContext ------------------------------------------------------------------

MyceliumUIParser::ProgramStatementContext::ProgramStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::InstanceDefinitionContext* MyceliumUIParser::ProgramStatementContext::instanceDefinition() {
  return getRuleContext<MyceliumUIParser::InstanceDefinitionContext>(0);
}


size_t MyceliumUIParser::ProgramStatementContext::getRuleIndex() const {
  return MyceliumUIParser::RuleProgramStatement;
}

void MyceliumUIParser::ProgramStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgramStatement(this);
}

void MyceliumUIParser::ProgramStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgramStatement(this);
}


std::any MyceliumUIParser::ProgramStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitProgramStatement(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ProgramStatementContext* MyceliumUIParser::programStatement() {
  ProgramStatementContext *_localctx = _tracker.createInstance<ProgramStatementContext>(_ctx, getState());
  enterRule(_localctx, 44, MyceliumUIParser::RuleProgramStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(272);
    instanceDefinition();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InstanceDefinitionContext ------------------------------------------------------------------

MyceliumUIParser::InstanceDefinitionContext::InstanceDefinitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::InstanceDefinitionContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::InstanceDefinitionContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::InstanceDefinitionContext::ID() {
  return getTokens(MyceliumUIParser::ID);
}

tree::TerminalNode* MyceliumUIParser::InstanceDefinitionContext::ID(size_t i) {
  return getToken(MyceliumUIParser::ID, i);
}

std::vector<MyceliumUIParser::InstanceMemberContext *> MyceliumUIParser::InstanceDefinitionContext::instanceMember() {
  return getRuleContexts<MyceliumUIParser::InstanceMemberContext>();
}

MyceliumUIParser::InstanceMemberContext* MyceliumUIParser::InstanceDefinitionContext::instanceMember(size_t i) {
  return getRuleContext<MyceliumUIParser::InstanceMemberContext>(i);
}


size_t MyceliumUIParser::InstanceDefinitionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleInstanceDefinition;
}

void MyceliumUIParser::InstanceDefinitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInstanceDefinition(this);
}

void MyceliumUIParser::InstanceDefinitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInstanceDefinition(this);
}


std::any MyceliumUIParser::InstanceDefinitionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitInstanceDefinition(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::InstanceDefinitionContext* MyceliumUIParser::instanceDefinition() {
  InstanceDefinitionContext *_localctx = _tracker.createInstance<InstanceDefinitionContext>(_ctx, getState());
  enterRule(_localctx, 46, MyceliumUIParser::RuleInstanceDefinition);
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
    setState(274);
    antlrcpp::downCast<InstanceDefinitionContext *>(_localctx)->typeName = match(MyceliumUIParser::ID);
    setState(276);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::ID) {
      setState(275);
      antlrcpp::downCast<InstanceDefinitionContext *>(_localctx)->instanceId = match(MyceliumUIParser::ID);
    }
    setState(278);
    match(MyceliumUIParser::LBRACE);
    setState(282);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 786496) != 0)) {
      setState(279);
      instanceMember();
      setState(284);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(285);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InstanceMemberContext ------------------------------------------------------------------

MyceliumUIParser::InstanceMemberContext::InstanceMemberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::PropAssignmentContext* MyceliumUIParser::InstanceMemberContext::propAssignment() {
  return getRuleContext<MyceliumUIParser::PropAssignmentContext>(0);
}

tree::TerminalNode* MyceliumUIParser::InstanceMemberContext::SEMI() {
  return getToken(MyceliumUIParser::SEMI, 0);
}

MyceliumUIParser::EventHandlerContext* MyceliumUIParser::InstanceMemberContext::eventHandler() {
  return getRuleContext<MyceliumUIParser::EventHandlerContext>(0);
}

MyceliumUIParser::StyleBlockContext* MyceliumUIParser::InstanceMemberContext::styleBlock() {
  return getRuleContext<MyceliumUIParser::StyleBlockContext>(0);
}

MyceliumUIParser::InstanceDefinitionContext* MyceliumUIParser::InstanceMemberContext::instanceDefinition() {
  return getRuleContext<MyceliumUIParser::InstanceDefinitionContext>(0);
}


size_t MyceliumUIParser::InstanceMemberContext::getRuleIndex() const {
  return MyceliumUIParser::RuleInstanceMember;
}

void MyceliumUIParser::InstanceMemberContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInstanceMember(this);
}

void MyceliumUIParser::InstanceMemberContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInstanceMember(this);
}


std::any MyceliumUIParser::InstanceMemberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitInstanceMember(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::InstanceMemberContext* MyceliumUIParser::instanceMember() {
  InstanceMemberContext *_localctx = _tracker.createInstance<InstanceMemberContext>(_ctx, getState());
  enterRule(_localctx, 48, MyceliumUIParser::RuleInstanceMember);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(297);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(287);
      propAssignment();
      setState(289);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::SEMI) {
        setState(288);
        match(MyceliumUIParser::SEMI);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(291);
      eventHandler();
      setState(293);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::SEMI) {
        setState(292);
        match(MyceliumUIParser::SEMI);
      }
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(295);
      antlrcpp::downCast<InstanceMemberContext *>(_localctx)->inlineStyle = styleBlock();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(296);
      instanceDefinition();
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

//----------------- PropAssignmentContext ------------------------------------------------------------------

MyceliumUIParser::PropAssignmentContext::PropAssignmentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::PropAssignmentContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::PropAssignmentContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::PropAssignmentContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}


size_t MyceliumUIParser::PropAssignmentContext::getRuleIndex() const {
  return MyceliumUIParser::RulePropAssignment;
}

void MyceliumUIParser::PropAssignmentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPropAssignment(this);
}

void MyceliumUIParser::PropAssignmentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPropAssignment(this);
}


std::any MyceliumUIParser::PropAssignmentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitPropAssignment(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::PropAssignmentContext* MyceliumUIParser::propAssignment() {
  PropAssignmentContext *_localctx = _tracker.createInstance<PropAssignmentContext>(_ctx, getState());
  enterRule(_localctx, 50, MyceliumUIParser::RulePropAssignment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(299);
    match(MyceliumUIParser::ID);
    setState(300);
    match(MyceliumUIParser::COLON);
    setState(301);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EventHandlerContext ------------------------------------------------------------------

MyceliumUIParser::EventHandlerContext::EventHandlerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::EventHandlerContext::ON() {
  return getToken(MyceliumUIParser::ON, 0);
}

tree::TerminalNode* MyceliumUIParser::EventHandlerContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

tree::TerminalNode* MyceliumUIParser::EventHandlerContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

MyceliumUIParser::FunctionLiteralContext* MyceliumUIParser::EventHandlerContext::functionLiteral() {
  return getRuleContext<MyceliumUIParser::FunctionLiteralContext>(0);
}


size_t MyceliumUIParser::EventHandlerContext::getRuleIndex() const {
  return MyceliumUIParser::RuleEventHandler;
}

void MyceliumUIParser::EventHandlerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEventHandler(this);
}

void MyceliumUIParser::EventHandlerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEventHandler(this);
}


std::any MyceliumUIParser::EventHandlerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitEventHandler(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::EventHandlerContext* MyceliumUIParser::eventHandler() {
  EventHandlerContext *_localctx = _tracker.createInstance<EventHandlerContext>(_ctx, getState());
  enterRule(_localctx, 52, MyceliumUIParser::RuleEventHandler);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(303);
    match(MyceliumUIParser::ON);
    setState(304);
    match(MyceliumUIParser::ID);
    setState(305);
    match(MyceliumUIParser::COLON);
    setState(306);
    functionLiteral();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionLiteralContext ------------------------------------------------------------------

MyceliumUIParser::FunctionLiteralContext::FunctionLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::FunctionLiteralContext::FUNCTION() {
  return getToken(MyceliumUIParser::FUNCTION, 0);
}

tree::TerminalNode* MyceliumUIParser::FunctionLiteralContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

MyceliumUIParser::ScriptBodyContext* MyceliumUIParser::FunctionLiteralContext::scriptBody() {
  return getRuleContext<MyceliumUIParser::ScriptBodyContext>(0);
}

tree::TerminalNode* MyceliumUIParser::FunctionLiteralContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}

tree::TerminalNode* MyceliumUIParser::FunctionLiteralContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::FunctionLiteralContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

MyceliumUIParser::ParamListContext* MyceliumUIParser::FunctionLiteralContext::paramList() {
  return getRuleContext<MyceliumUIParser::ParamListContext>(0);
}


size_t MyceliumUIParser::FunctionLiteralContext::getRuleIndex() const {
  return MyceliumUIParser::RuleFunctionLiteral;
}

void MyceliumUIParser::FunctionLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionLiteral(this);
}

void MyceliumUIParser::FunctionLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionLiteral(this);
}


std::any MyceliumUIParser::FunctionLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitFunctionLiteral(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::FunctionLiteralContext* MyceliumUIParser::functionLiteral() {
  FunctionLiteralContext *_localctx = _tracker.createInstance<FunctionLiteralContext>(_ctx, getState());
  enterRule(_localctx, 54, MyceliumUIParser::RuleFunctionLiteral);
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
    setState(308);
    match(MyceliumUIParser::FUNCTION);
    setState(314);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MyceliumUIParser::LPAREN) {
      setState(309);
      match(MyceliumUIParser::LPAREN);
      setState(311);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == MyceliumUIParser::ID) {
        setState(310);
        paramList();
      }
      setState(313);
      match(MyceliumUIParser::RPAREN);
    }
    setState(316);
    match(MyceliumUIParser::LBRACE);
    setState(317);
    scriptBody();
    setState(318);
    match(MyceliumUIParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

MyceliumUIParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t MyceliumUIParser::ExpressionContext::getRuleIndex() const {
  return MyceliumUIParser::RuleExpression;
}

void MyceliumUIParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- FunctionLiteralExprContext ------------------------------------------------------------------

MyceliumUIParser::FunctionLiteralContext* MyceliumUIParser::FunctionLiteralExprContext::functionLiteral() {
  return getRuleContext<MyceliumUIParser::FunctionLiteralContext>(0);
}

MyceliumUIParser::FunctionLiteralExprContext::FunctionLiteralExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::FunctionLiteralExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionLiteralExpr(this);
}
void MyceliumUIParser::FunctionLiteralExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionLiteralExpr(this);
}

std::any MyceliumUIParser::FunctionLiteralExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitFunctionLiteralExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ComparisonExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::ComparisonExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::ComparisonExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::ComparisonExprContext::LT() {
  return getToken(MyceliumUIParser::LT, 0);
}

tree::TerminalNode* MyceliumUIParser::ComparisonExprContext::LTE() {
  return getToken(MyceliumUIParser::LTE, 0);
}

tree::TerminalNode* MyceliumUIParser::ComparisonExprContext::GT() {
  return getToken(MyceliumUIParser::GT, 0);
}

tree::TerminalNode* MyceliumUIParser::ComparisonExprContext::GTE() {
  return getToken(MyceliumUIParser::GTE, 0);
}

MyceliumUIParser::ComparisonExprContext::ComparisonExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::ComparisonExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparisonExpr(this);
}
void MyceliumUIParser::ComparisonExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparisonExpr(this);
}

std::any MyceliumUIParser::ComparisonExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitComparisonExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SelectExprContext ------------------------------------------------------------------

MyceliumUIParser::SelectExpressionContext* MyceliumUIParser::SelectExprContext::selectExpression() {
  return getRuleContext<MyceliumUIParser::SelectExpressionContext>(0);
}

MyceliumUIParser::SelectExprContext::SelectExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::SelectExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSelectExpr(this);
}
void MyceliumUIParser::SelectExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSelectExpr(this);
}

std::any MyceliumUIParser::SelectExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitSelectExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalNotExprContext ------------------------------------------------------------------

tree::TerminalNode* MyceliumUIParser::LogicalNotExprContext::NOT() {
  return getToken(MyceliumUIParser::NOT, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::LogicalNotExprContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

MyceliumUIParser::LogicalNotExprContext::LogicalNotExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::LogicalNotExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalNotExpr(this);
}
void MyceliumUIParser::LogicalNotExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalNotExpr(this);
}

std::any MyceliumUIParser::LogicalNotExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitLogicalNotExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalAndExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::LogicalAndExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::LogicalAndExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::LogicalAndExprContext::AND() {
  return getToken(MyceliumUIParser::AND, 0);
}

MyceliumUIParser::LogicalAndExprContext::LogicalAndExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::LogicalAndExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalAndExpr(this);
}
void MyceliumUIParser::LogicalAndExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalAndExpr(this);
}

std::any MyceliumUIParser::LogicalAndExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitLogicalAndExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LogicalOrExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::LogicalOrExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::LogicalOrExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::LogicalOrExprContext::OR() {
  return getToken(MyceliumUIParser::OR, 0);
}

MyceliumUIParser::LogicalOrExprContext::LogicalOrExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::LogicalOrExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLogicalOrExpr(this);
}
void MyceliumUIParser::LogicalOrExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLogicalOrExpr(this);
}

std::any MyceliumUIParser::LogicalOrExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitLogicalOrExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EqualityExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::EqualityExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::EqualityExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::EqualityExprContext::EQ() {
  return getToken(MyceliumUIParser::EQ, 0);
}

tree::TerminalNode* MyceliumUIParser::EqualityExprContext::NEQ() {
  return getToken(MyceliumUIParser::NEQ, 0);
}

MyceliumUIParser::EqualityExprContext::EqualityExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::EqualityExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEqualityExpr(this);
}
void MyceliumUIParser::EqualityExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEqualityExpr(this);
}

std::any MyceliumUIParser::EqualityExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitEqualityExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulDivExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::MulDivExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::MulDivExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::MulDivExprContext::MUL() {
  return getToken(MyceliumUIParser::MUL, 0);
}

tree::TerminalNode* MyceliumUIParser::MulDivExprContext::DIV() {
  return getToken(MyceliumUIParser::DIV, 0);
}

MyceliumUIParser::MulDivExprContext::MulDivExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::MulDivExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMulDivExpr(this);
}
void MyceliumUIParser::MulDivExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMulDivExpr(this);
}

std::any MyceliumUIParser::MulDivExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitMulDivExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FunctionCallExprContext ------------------------------------------------------------------

MyceliumUIParser::ExpressionContext* MyceliumUIParser::FunctionCallExprContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MyceliumUIParser::FunctionCallExprContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::FunctionCallExprContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

MyceliumUIParser::ArgumentListContext* MyceliumUIParser::FunctionCallExprContext::argumentList() {
  return getRuleContext<MyceliumUIParser::ArgumentListContext>(0);
}

MyceliumUIParser::FunctionCallExprContext::FunctionCallExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::FunctionCallExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionCallExpr(this);
}
void MyceliumUIParser::FunctionCallExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionCallExpr(this);
}

std::any MyceliumUIParser::FunctionCallExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitFunctionCallExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- StaticCallExprContext ------------------------------------------------------------------

std::vector<tree::TerminalNode *> MyceliumUIParser::StaticCallExprContext::ID() {
  return getTokens(MyceliumUIParser::ID);
}

tree::TerminalNode* MyceliumUIParser::StaticCallExprContext::ID(size_t i) {
  return getToken(MyceliumUIParser::ID, i);
}

tree::TerminalNode* MyceliumUIParser::StaticCallExprContext::DBL_COLON() {
  return getToken(MyceliumUIParser::DBL_COLON, 0);
}

tree::TerminalNode* MyceliumUIParser::StaticCallExprContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

tree::TerminalNode* MyceliumUIParser::StaticCallExprContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

MyceliumUIParser::ArgumentListContext* MyceliumUIParser::StaticCallExprContext::argumentList() {
  return getRuleContext<MyceliumUIParser::ArgumentListContext>(0);
}

MyceliumUIParser::StaticCallExprContext::StaticCallExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::StaticCallExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStaticCallExpr(this);
}
void MyceliumUIParser::StaticCallExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStaticCallExpr(this);
}

std::any MyceliumUIParser::StaticCallExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitStaticCallExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- CalculateExprContext ------------------------------------------------------------------

MyceliumUIParser::CalculateBlockContext* MyceliumUIParser::CalculateExprContext::calculateBlock() {
  return getRuleContext<MyceliumUIParser::CalculateBlockContext>(0);
}

MyceliumUIParser::CalculateExprContext::CalculateExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::CalculateExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCalculateExpr(this);
}
void MyceliumUIParser::CalculateExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCalculateExpr(this);
}

std::any MyceliumUIParser::CalculateExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitCalculateExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- PrimaryExprContext ------------------------------------------------------------------

MyceliumUIParser::PrimaryContext* MyceliumUIParser::PrimaryExprContext::primary() {
  return getRuleContext<MyceliumUIParser::PrimaryContext>(0);
}

MyceliumUIParser::PrimaryExprContext::PrimaryExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::PrimaryExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrimaryExpr(this);
}
void MyceliumUIParser::PrimaryExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrimaryExpr(this);
}

std::any MyceliumUIParser::PrimaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitPrimaryExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenExprContext ------------------------------------------------------------------

tree::TerminalNode* MyceliumUIParser::ParenExprContext::LPAREN() {
  return getToken(MyceliumUIParser::LPAREN, 0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::ParenExprContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MyceliumUIParser::ParenExprContext::RPAREN() {
  return getToken(MyceliumUIParser::RPAREN, 0);
}

MyceliumUIParser::ParenExprContext::ParenExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::ParenExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenExpr(this);
}
void MyceliumUIParser::ParenExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenExpr(this);
}

std::any MyceliumUIParser::ParenExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitParenExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MemberAccessExprContext ------------------------------------------------------------------

MyceliumUIParser::ExpressionContext* MyceliumUIParser::MemberAccessExprContext::expression() {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(0);
}

tree::TerminalNode* MyceliumUIParser::MemberAccessExprContext::DOT() {
  return getToken(MyceliumUIParser::DOT, 0);
}

tree::TerminalNode* MyceliumUIParser::MemberAccessExprContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}

MyceliumUIParser::MemberAccessExprContext::MemberAccessExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::MemberAccessExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMemberAccessExpr(this);
}
void MyceliumUIParser::MemberAccessExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMemberAccessExpr(this);
}

std::any MyceliumUIParser::MemberAccessExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitMemberAccessExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddSubExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::AddSubExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::AddSubExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::AddSubExprContext::ADD() {
  return getToken(MyceliumUIParser::ADD, 0);
}

tree::TerminalNode* MyceliumUIParser::AddSubExprContext::MINUS() {
  return getToken(MyceliumUIParser::MINUS, 0);
}

MyceliumUIParser::AddSubExprContext::AddSubExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::AddSubExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddSubExpr(this);
}
void MyceliumUIParser::AddSubExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddSubExpr(this);
}

std::any MyceliumUIParser::AddSubExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitAddSubExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ThisExprContext ------------------------------------------------------------------

tree::TerminalNode* MyceliumUIParser::ThisExprContext::THIS() {
  return getToken(MyceliumUIParser::THIS, 0);
}

MyceliumUIParser::ThisExprContext::ThisExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::ThisExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterThisExpr(this);
}
void MyceliumUIParser::ThisExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitThisExpr(this);
}

std::any MyceliumUIParser::ThisExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitThisExpr(this);
  else
    return visitor->visitChildren(this);
}
//----------------- TernaryExprContext ------------------------------------------------------------------

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::TernaryExprContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::TernaryExprContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

tree::TerminalNode* MyceliumUIParser::TernaryExprContext::QUESTION() {
  return getToken(MyceliumUIParser::QUESTION, 0);
}

tree::TerminalNode* MyceliumUIParser::TernaryExprContext::COLON() {
  return getToken(MyceliumUIParser::COLON, 0);
}

MyceliumUIParser::TernaryExprContext::TernaryExprContext(ExpressionContext *ctx) { copyFrom(ctx); }

void MyceliumUIParser::TernaryExprContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTernaryExpr(this);
}
void MyceliumUIParser::TernaryExprContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTernaryExpr(this);
}

std::any MyceliumUIParser::TernaryExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitTernaryExpr(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::expression() {
   return expression(0);
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  MyceliumUIParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  MyceliumUIParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 56;
  enterRecursionRule(_localctx, 56, MyceliumUIParser::RuleExpression, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(340);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<LogicalNotExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(321);
      match(MyceliumUIParser::NOT);
      setState(322);
      expression(10);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<StaticCallExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(323);
      match(MyceliumUIParser::ID);
      setState(324);
      match(MyceliumUIParser::DBL_COLON);
      setState(325);
      match(MyceliumUIParser::ID);
      setState(326);
      match(MyceliumUIParser::LPAREN);
      setState(328);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 8796130482432) != 0)) {
        setState(327);
        argumentList();
      }
      setState(330);
      match(MyceliumUIParser::RPAREN);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<SelectExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(331);
      selectExpression();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<CalculateExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(332);
      calculateBlock();
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<FunctionLiteralExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(333);
      functionLiteral();
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<PrimaryExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(334);
      primary();
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<ParenExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(335);
      match(MyceliumUIParser::LPAREN);
      setState(336);
      expression(0);
      setState(337);
      match(MyceliumUIParser::RPAREN);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<ThisExprContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(339);
      match(MyceliumUIParser::THIS);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(377);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(375);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<TernaryExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(342);

          if (!(precpred(_ctx, 17))) throw FailedPredicateException(this, "precpred(_ctx, 17)");
          setState(343);
          match(MyceliumUIParser::QUESTION);
          setState(344);
          expression(0);
          setState(345);
          match(MyceliumUIParser::COLON);
          setState(346);
          expression(18);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<LogicalOrExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(348);

          if (!(precpred(_ctx, 16))) throw FailedPredicateException(this, "precpred(_ctx, 16)");
          setState(349);
          match(MyceliumUIParser::OR);
          setState(350);
          expression(17);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<LogicalAndExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(351);

          if (!(precpred(_ctx, 15))) throw FailedPredicateException(this, "precpred(_ctx, 15)");
          setState(352);
          match(MyceliumUIParser::AND);
          setState(353);
          expression(16);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<EqualityExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(354);

          if (!(precpred(_ctx, 14))) throw FailedPredicateException(this, "precpred(_ctx, 14)");
          setState(355);
          _la = _input->LA(1);
          if (!(_la == MyceliumUIParser::EQ

          || _la == MyceliumUIParser::NEQ)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(356);
          expression(15);
          break;
        }

        case 5: {
          auto newContext = _tracker.createInstance<ComparisonExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(357);

          if (!(precpred(_ctx, 13))) throw FailedPredicateException(this, "precpred(_ctx, 13)");
          setState(358);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 1055531162664960) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(359);
          expression(14);
          break;
        }

        case 6: {
          auto newContext = _tracker.createInstance<AddSubExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(360);

          if (!(precpred(_ctx, 12))) throw FailedPredicateException(this, "precpred(_ctx, 12)");
          setState(361);
          _la = _input->LA(1);
          if (!(_la == MyceliumUIParser::MINUS

          || _la == MyceliumUIParser::ADD)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(362);
          expression(13);
          break;
        }

        case 7: {
          auto newContext = _tracker.createInstance<MulDivExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(363);

          if (!(precpred(_ctx, 11))) throw FailedPredicateException(this, "precpred(_ctx, 11)");
          setState(364);
          _la = _input->LA(1);
          if (!(_la == MyceliumUIParser::MUL

          || _la == MyceliumUIParser::DIV)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(365);
          expression(12);
          break;
        }

        case 8: {
          auto newContext = _tracker.createInstance<MemberAccessExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(366);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(367);
          match(MyceliumUIParser::DOT);
          setState(368);
          match(MyceliumUIParser::ID);
          break;
        }

        case 9: {
          auto newContext = _tracker.createInstance<FunctionCallExprContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(369);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(370);
          match(MyceliumUIParser::LPAREN);
          setState(372);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if ((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 8796130482432) != 0)) {
            setState(371);
            argumentList();
          }
          setState(374);
          match(MyceliumUIParser::RPAREN);
          break;
        }

        default:
          break;
        } 
      }
      setState(379);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PrimaryContext ------------------------------------------------------------------

MyceliumUIParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MyceliumUIParser::LiteralContext* MyceliumUIParser::PrimaryContext::literal() {
  return getRuleContext<MyceliumUIParser::LiteralContext>(0);
}

MyceliumUIParser::IdentifierContext* MyceliumUIParser::PrimaryContext::identifier() {
  return getRuleContext<MyceliumUIParser::IdentifierContext>(0);
}

tree::TerminalNode* MyceliumUIParser::PrimaryContext::THIS() {
  return getToken(MyceliumUIParser::THIS, 0);
}


size_t MyceliumUIParser::PrimaryContext::getRuleIndex() const {
  return MyceliumUIParser::RulePrimary;
}

void MyceliumUIParser::PrimaryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrimary(this);
}

void MyceliumUIParser::PrimaryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrimary(this);
}


std::any MyceliumUIParser::PrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitPrimary(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::PrimaryContext* MyceliumUIParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 58, MyceliumUIParser::RulePrimary);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(383);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MyceliumUIParser::TRUE:
      case MyceliumUIParser::FALSE:
      case MyceliumUIParser::NUMBER:
      case MyceliumUIParser::STRING: {
        enterOuterAlt(_localctx, 1);
        setState(380);
        literal();
        break;
      }

      case MyceliumUIParser::ID: {
        enterOuterAlt(_localctx, 2);
        setState(381);
        identifier();
        break;
      }

      case MyceliumUIParser::THIS: {
        enterOuterAlt(_localctx, 3);
        setState(382);
        match(MyceliumUIParser::THIS);
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

//----------------- ArgumentListContext ------------------------------------------------------------------

MyceliumUIParser::ArgumentListContext::ArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MyceliumUIParser::ExpressionContext *> MyceliumUIParser::ArgumentListContext::expression() {
  return getRuleContexts<MyceliumUIParser::ExpressionContext>();
}

MyceliumUIParser::ExpressionContext* MyceliumUIParser::ArgumentListContext::expression(size_t i) {
  return getRuleContext<MyceliumUIParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> MyceliumUIParser::ArgumentListContext::COMMA() {
  return getTokens(MyceliumUIParser::COMMA);
}

tree::TerminalNode* MyceliumUIParser::ArgumentListContext::COMMA(size_t i) {
  return getToken(MyceliumUIParser::COMMA, i);
}


size_t MyceliumUIParser::ArgumentListContext::getRuleIndex() const {
  return MyceliumUIParser::RuleArgumentList;
}

void MyceliumUIParser::ArgumentListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArgumentList(this);
}

void MyceliumUIParser::ArgumentListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArgumentList(this);
}


std::any MyceliumUIParser::ArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitArgumentList(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ArgumentListContext* MyceliumUIParser::argumentList() {
  ArgumentListContext *_localctx = _tracker.createInstance<ArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 60, MyceliumUIParser::RuleArgumentList);
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
    setState(385);
    expression(0);
    setState(390);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MyceliumUIParser::COMMA) {
      setState(386);
      match(MyceliumUIParser::COMMA);
      setState(387);
      expression(0);
      setState(392);
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

//----------------- LiteralContext ------------------------------------------------------------------

MyceliumUIParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::LiteralContext::NUMBER() {
  return getToken(MyceliumUIParser::NUMBER, 0);
}

tree::TerminalNode* MyceliumUIParser::LiteralContext::STRING() {
  return getToken(MyceliumUIParser::STRING, 0);
}

MyceliumUIParser::BooleanLiteralContext* MyceliumUIParser::LiteralContext::booleanLiteral() {
  return getRuleContext<MyceliumUIParser::BooleanLiteralContext>(0);
}

MyceliumUIParser::LengthLiteralContext* MyceliumUIParser::LiteralContext::lengthLiteral() {
  return getRuleContext<MyceliumUIParser::LengthLiteralContext>(0);
}

MyceliumUIParser::TimeLiteralContext* MyceliumUIParser::LiteralContext::timeLiteral() {
  return getRuleContext<MyceliumUIParser::TimeLiteralContext>(0);
}


size_t MyceliumUIParser::LiteralContext::getRuleIndex() const {
  return MyceliumUIParser::RuleLiteral;
}

void MyceliumUIParser::LiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLiteral(this);
}

void MyceliumUIParser::LiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLiteral(this);
}


std::any MyceliumUIParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::LiteralContext* MyceliumUIParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 62, MyceliumUIParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(398);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(393);
      match(MyceliumUIParser::NUMBER);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(394);
      match(MyceliumUIParser::STRING);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(395);
      booleanLiteral();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(396);
      lengthLiteral();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(397);
      timeLiteral();
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

//----------------- BooleanLiteralContext ------------------------------------------------------------------

MyceliumUIParser::BooleanLiteralContext::BooleanLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::BooleanLiteralContext::TRUE() {
  return getToken(MyceliumUIParser::TRUE, 0);
}

tree::TerminalNode* MyceliumUIParser::BooleanLiteralContext::FALSE() {
  return getToken(MyceliumUIParser::FALSE, 0);
}


size_t MyceliumUIParser::BooleanLiteralContext::getRuleIndex() const {
  return MyceliumUIParser::RuleBooleanLiteral;
}

void MyceliumUIParser::BooleanLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBooleanLiteral(this);
}

void MyceliumUIParser::BooleanLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBooleanLiteral(this);
}


std::any MyceliumUIParser::BooleanLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitBooleanLiteral(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::BooleanLiteralContext* MyceliumUIParser::booleanLiteral() {
  BooleanLiteralContext *_localctx = _tracker.createInstance<BooleanLiteralContext>(_ctx, getState());
  enterRule(_localctx, 64, MyceliumUIParser::RuleBooleanLiteral);
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
    setState(400);
    _la = _input->LA(1);
    if (!(_la == MyceliumUIParser::TRUE

    || _la == MyceliumUIParser::FALSE)) {
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

MyceliumUIParser::IdentifierContext::IdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::IdentifierContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}


size_t MyceliumUIParser::IdentifierContext::getRuleIndex() const {
  return MyceliumUIParser::RuleIdentifier;
}

void MyceliumUIParser::IdentifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIdentifier(this);
}

void MyceliumUIParser::IdentifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIdentifier(this);
}


std::any MyceliumUIParser::IdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitIdentifier(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::IdentifierContext* MyceliumUIParser::identifier() {
  IdentifierContext *_localctx = _tracker.createInstance<IdentifierContext>(_ctx, getState());
  enterRule(_localctx, 66, MyceliumUIParser::RuleIdentifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(402);
    match(MyceliumUIParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

MyceliumUIParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::TypeContext::ID() {
  return getToken(MyceliumUIParser::ID, 0);
}


size_t MyceliumUIParser::TypeContext::getRuleIndex() const {
  return MyceliumUIParser::RuleType;
}

void MyceliumUIParser::TypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType(this);
}

void MyceliumUIParser::TypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType(this);
}


std::any MyceliumUIParser::TypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitType(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::TypeContext* MyceliumUIParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 68, MyceliumUIParser::RuleType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(404);
    match(MyceliumUIParser::ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScriptBodyContext ------------------------------------------------------------------

MyceliumUIParser::ScriptBodyContext::ScriptBodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MyceliumUIParser::CodeItemContext *> MyceliumUIParser::ScriptBodyContext::codeItem() {
  return getRuleContexts<MyceliumUIParser::CodeItemContext>();
}

MyceliumUIParser::CodeItemContext* MyceliumUIParser::ScriptBodyContext::codeItem(size_t i) {
  return getRuleContext<MyceliumUIParser::CodeItemContext>(i);
}


size_t MyceliumUIParser::ScriptBodyContext::getRuleIndex() const {
  return MyceliumUIParser::RuleScriptBody;
}

void MyceliumUIParser::ScriptBodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterScriptBody(this);
}

void MyceliumUIParser::ScriptBodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitScriptBody(this);
}


std::any MyceliumUIParser::ScriptBodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitScriptBody(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::ScriptBodyContext* MyceliumUIParser::scriptBody() {
  ScriptBodyContext *_localctx = _tracker.createInstance<ScriptBodyContext>(_ctx, getState());
  enterRule(_localctx, 70, MyceliumUIParser::RuleScriptBody);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(409);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    while (alt != 1 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1 + 1) {
        setState(406);
        codeItem(); 
      }
      setState(411);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 39, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CodeItemContext ------------------------------------------------------------------

MyceliumUIParser::CodeItemContext::CodeItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::CodeItemContext::LBRACE() {
  return getToken(MyceliumUIParser::LBRACE, 0);
}

MyceliumUIParser::ScriptBodyContext* MyceliumUIParser::CodeItemContext::scriptBody() {
  return getRuleContext<MyceliumUIParser::ScriptBodyContext>(0);
}

tree::TerminalNode* MyceliumUIParser::CodeItemContext::RBRACE() {
  return getToken(MyceliumUIParser::RBRACE, 0);
}


size_t MyceliumUIParser::CodeItemContext::getRuleIndex() const {
  return MyceliumUIParser::RuleCodeItem;
}

void MyceliumUIParser::CodeItemContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCodeItem(this);
}

void MyceliumUIParser::CodeItemContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCodeItem(this);
}


std::any MyceliumUIParser::CodeItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitCodeItem(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::CodeItemContext* MyceliumUIParser::codeItem() {
  CodeItemContext *_localctx = _tracker.createInstance<CodeItemContext>(_ctx, getState());
  enterRule(_localctx, 72, MyceliumUIParser::RuleCodeItem);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(417);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 40, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(412);
      match(MyceliumUIParser::LBRACE);
      setState(413);
      scriptBody();
      setState(414);
      match(MyceliumUIParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(416);
      _la = _input->LA(1);
      if (_la == 0 || _la == Token::EOF || (_la == MyceliumUIParser::RBRACE)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
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

//----------------- LengthLiteralContext ------------------------------------------------------------------

MyceliumUIParser::LengthLiteralContext::LengthLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::LengthLiteralContext::NUMBER() {
  return getToken(MyceliumUIParser::NUMBER, 0);
}

tree::TerminalNode* MyceliumUIParser::LengthLiteralContext::PX() {
  return getToken(MyceliumUIParser::PX, 0);
}

tree::TerminalNode* MyceliumUIParser::LengthLiteralContext::PERCENT() {
  return getToken(MyceliumUIParser::PERCENT, 0);
}


size_t MyceliumUIParser::LengthLiteralContext::getRuleIndex() const {
  return MyceliumUIParser::RuleLengthLiteral;
}

void MyceliumUIParser::LengthLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLengthLiteral(this);
}

void MyceliumUIParser::LengthLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLengthLiteral(this);
}


std::any MyceliumUIParser::LengthLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitLengthLiteral(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::LengthLiteralContext* MyceliumUIParser::lengthLiteral() {
  LengthLiteralContext *_localctx = _tracker.createInstance<LengthLiteralContext>(_ctx, getState());
  enterRule(_localctx, 74, MyceliumUIParser::RuleLengthLiteral);
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
    setState(419);
    match(MyceliumUIParser::NUMBER);
    setState(421);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx)) {
    case 1: {
      setState(420);
      _la = _input->LA(1);
      if (!(_la == MyceliumUIParser::PX

      || _la == MyceliumUIParser::PERCENT)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
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

//----------------- TimeLiteralContext ------------------------------------------------------------------

MyceliumUIParser::TimeLiteralContext::TimeLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MyceliumUIParser::TimeLiteralContext::NUMBER() {
  return getToken(MyceliumUIParser::NUMBER, 0);
}

tree::TerminalNode* MyceliumUIParser::TimeLiteralContext::SECONDS() {
  return getToken(MyceliumUIParser::SECONDS, 0);
}


size_t MyceliumUIParser::TimeLiteralContext::getRuleIndex() const {
  return MyceliumUIParser::RuleTimeLiteral;
}

void MyceliumUIParser::TimeLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTimeLiteral(this);
}

void MyceliumUIParser::TimeLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<MyceliumUIListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTimeLiteral(this);
}


std::any MyceliumUIParser::TimeLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MyceliumUIVisitor*>(visitor))
    return parserVisitor->visitTimeLiteral(this);
  else
    return visitor->visitChildren(this);
}

MyceliumUIParser::TimeLiteralContext* MyceliumUIParser::timeLiteral() {
  TimeLiteralContext *_localctx = _tracker.createInstance<TimeLiteralContext>(_ctx, getState());
  enterRule(_localctx, 76, MyceliumUIParser::RuleTimeLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(423);
    match(MyceliumUIParser::NUMBER);
    setState(424);
    match(MyceliumUIParser::SECONDS);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool MyceliumUIParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 28: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool MyceliumUIParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 17);
    case 1: return precpred(_ctx, 16);
    case 2: return precpred(_ctx, 15);
    case 3: return precpred(_ctx, 14);
    case 4: return precpred(_ctx, 13);
    case 5: return precpred(_ctx, 12);
    case 6: return precpred(_ctx, 11);
    case 7: return precpred(_ctx, 9);
    case 8: return precpred(_ctx, 8);

  default:
    break;
  }
  return true;
}

void MyceliumUIParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  myceliumuiParserInitialize();
#else
  ::antlr4::internal::call_once(myceliumuiParserOnceFlag, myceliumuiParserInitialize);
#endif
}
