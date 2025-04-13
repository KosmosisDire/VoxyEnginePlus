#include <chrono> // Include chrono header first
#include "parser_interface.hpp"

// Include ANTLR runtime headers
#include "antlr4-runtime.h"

// Include generated lexer and parser headers
#include "preprocessor/MySimpleUILexer.h"
#include "preprocessor/MySimpleUIParser.h"

#include <iostream> // For potential error logging
#include <sstream>  // For error messages

namespace Mycelium::Scripting
{
    // Custom error listener (optional but recommended)
    class ParserErrorListener : public antlr4::BaseErrorListener
    {
        public:
            std::stringstream errorMessages;
            bool hasErrors = false;

            void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
                             const std::string &msg, std::exception_ptr e) override
            {
                hasErrors = true;
                // Prepend "// Error: " to make it clear in the output string
                errorMessages << "// Error: line " << line << ":" << charPositionInLine << " " << msg << std::endl;
            }
    };


    // --- Implementation for C-API Generation ---

    std::string ParserInterface::generateCApi(const std::string& scriptContent)
    {
        antlr4::ANTLRInputStream inputStream(scriptContent);
        MySimpleUILexer lexer(&inputStream);
        antlr4::CommonTokenStream tokens(&lexer);
        MySimpleUIParser parser(&tokens);

        // Add custom error listener
        ParserErrorListener errorListener;
        parser.removeErrorListeners(); // Remove default console listener
        parser.addErrorListener(&errorListener);

        antlr4::tree::ParseTree* tree = nullptr;

        try
        {
            // Start parsing from the 'program' rule
            // This tree pointer is valid only within this function's scope
            tree = parser.program();

            // Check for errors reported by the listener or the parser itself
            if (errorListener.hasErrors || parser.getNumberOfSyntaxErrors() > 0 || !tree)
            {
                std::cerr << "Parser errors encountered!" << std::endl;
                std::cerr << errorListener.errorMessages.str(); // Log errors to console
                // Return the collected error messages
                return "// Error: Failed to parse the script.\n" + errorListener.errorMessages.str();
            }

            // Parsing succeeded, print the raw tree for debugging (optional)
            // std::cout << "--- ANTLR Parse Tree ---" << std::endl;
            // std::cout << tree->toStringTree(&parser) << std::endl;
            // std::cout << "------------------------" << std::endl;


            // Now convert the valid tree using AstConverter
            AstConverter converter;
            return converter.generate(tree); // Pass the tree

        }
        catch (const std::exception& e)
        {
            // Catch exceptions during parsing or conversion
            std::cerr << "ANTLR Parsing/Conversion Exception: " << e.what() << std::endl;
            return "// Error: Exception during parsing or C-API generation: " + std::string(e.what()) + "\n";
        }
        catch (...)
        {
            // Catch unknown exceptions
            std::cerr << "Unknown ANTLR Parsing/Conversion Exception" << std::endl;
            return "// Error: Unknown exception during parsing or C-API generation.\n";
        }
    }

} // namespace Mycelium::Scripting
