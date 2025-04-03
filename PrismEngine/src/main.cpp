// #include "project_paths.h"
// #include "voxel-app.hpp"
// #include <engine/apis/Resources.hpp>
// #include <scripting/scripting.hpp>

// auto main() -> int
// {
//     scriptingEngine.registerStdLib();
//     scriptingEngine.registerEngineAPI();

//     VoxyApp app;

//     scripts->addScriptFile(Resources::GetResourcePath("test.as").string());
//     scriptingEngine.savePredefinedScript(PROJECT_ROOT_DIR "/as.predefined");

//     while (true)
//     {
//         scriptingEngine.checkHotReload();

//         if (app.Update())
//         {
//             break;
//         }
//     }

//     return 0;
// }

#include <iostream>
#include <fstream>
#include <string>

// ANTLR Runtime Headers
#include "antlr4-runtime.h" // Main runtime header (usually includes others)
#include "ANTLRInputStream.h"
#include "CommonTokenStream.h"
#include "tree/ParseTree.h"
#include "tree/ParseTreeWalker.h" // If using listeners later
#include "tree/ParseTreeVisitor.h" // For visitor pattern

// Generated Headers
#include <MySimpleUILexer.h>
#include <MySimpleUIParser.h>
#include <MySimpleUIBaseVisitor.h> // Include your visitor base

// --- Placeholder for your AST Builder Visitor ---
// You will create this class later to actually build your AST
// class MyAstBuilderVisitor : public MySimpleUIBaseVisitor {
// public:
//     // Override visit methods like visitProgram, visitComponentCall, etc.
//     // antlrcpp::Any visitProgram(MySimpleUIParser::ProgramContext *ctx) override {
//     //     std::cout << "Visiting Program Node!" << std::endl;
//     //     // Visit children and build your ProgramNode AST object
//     //     return visitChildren(ctx); // Default behavior
//     // }
//     // ... other visit methods
// };
// ---

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file.mysimpleui>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream stream;
    stream.open(filename);

    if (!stream)
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    std::cout << "Parsing file: " << filename << std::endl;

    // 1. Create Input Stream
    antlr4::ANTLRInputStream input(stream);

    // 2. Create Lexer
    MySimpleUILexer lexer(&input);

    // 3. Create Token Stream
    antlr4::CommonTokenStream tokens(&lexer);
    // Optional: Print tokens for debugging
    // tokens.fill();
    // for (auto token : tokens.getTokens()) {
    //     std::cout << token->toString() << std::endl;
    // }
    // tokens.reset(); // Reset stream position if you filled it

    // 4. Create Parser
    MySimpleUIParser parser(&tokens);

    // 5. Parse starting from the 'program' rule
    // This generates the Parse Tree
    antlr4::tree::ParseTree *tree = parser.program();

    if (parser.getNumberOfSyntaxErrors() > 0)
    {
        std::cerr << "Syntax errors found during parsing." << std::endl;
        // ANTLR automatically prints errors to stderr by default
        return 1; // Indicate failure
    }
    else
    {
        std::cout << "Parsing successful." << std::endl;
    }


    // --- Output/Use the Parse Tree ---

    // Option A: Print the LISP-style tree (for basic verification)
    std::cout << "\nParse Tree (LISP format):" << std::endl;
    std::cout << tree->toStringTree(&parser) << std::endl;

    // Option B: Use the Visitor (The RECOMMENDED way for AST building)
    std::cout << "\nRunning Visitor (placeholder)..." << std::endl;
    // MyAstBuilderVisitor astBuilder; // Create your custom visitor instance
    // try {
    //     antlrcpp::Any astRoot = tree->accept(&astBuilder);
    //     // 'astRoot' would hold your top-level AST node (e.g., ProgramNode*)
    //     // Process your AST here...
    //     std::cout << "AST building step completed (actual implementation needed)." << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Error during AST construction: " << e.what() << std::endl;
    //     return 1;
    // }
    // ---

    std::cout << "\nExecution finished." << std::endl;
    return 0;
}