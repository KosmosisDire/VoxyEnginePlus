#include <chrono> // Include chrono header first
#include "parser_interface.hpp"

// Include ANTLR runtime headers
#include "antlr4-runtime.h"

// Include generated lexer and parser headers
#include "preprocessor/MySimpleUILexer.h"
#include "preprocessor/MySimpleUIParser.h"

#include <iostream> // For potential error logging

namespace Mycelium::Scripting
{

    std::shared_ptr<antlr4::tree::ParseTree> ParserInterface::parseScript(const std::string& scriptContent)
    {
        antlr4::ANTLRInputStream inputStream(scriptContent);
        MySimpleUILexer lexer(&inputStream);
        antlr4::CommonTokenStream tokens(&lexer);
        MySimpleUIParser parser(&tokens);

        // Optional: Add error listeners for more detailed diagnostics
        // parser.removeErrorListeners(); // Remove default console listener
        // parser.addErrorListener(...); // Add custom listener

        try
        {
            // Start parsing from the 'program' rule (assuming this is your grammar's entry point)
            // Replace 'program' with your actual starting rule if different.
            antlr4::tree::ParseTree* tree = parser.program();

            if (parser.getNumberOfSyntaxErrors() > 0)
            {
                // Handle parsing errors, e.g., log them or return nullptr
                std::cerr << "Parser errors encountered!" << std::endl;
                // Depending on requirements, you might still want to return the partial tree
                // or guarantee nullptr on any error. For now, let's return nullptr.
                return nullptr;
            }

            // TODO: Implement proper tree handling (e.g., visitor pattern or copying the tree)
            //       to safely return the parsed structure instead of nullptr.
            // For now, just print the tree structure for debugging.
            if (tree)
            {
                // Print the tree structure to standard output
                std::cout << "--- ANTLR Parse Tree ---" << std::endl;
                std::cout << tree->toStringTree(&parser) << std::endl;
                std::cout << "------------------------" << std::endl;
            }

            // Return nullptr for now, as the tree's lifetime is managed by the parser
            // which is going out of scope. The caller should not use this pointer.
            return nullptr;


        }
        catch (const std::exception& e)
        {
            std::cerr << "ANTLR Parsing Exception: " << e.what() << std::endl;
            return nullptr;
        }
        catch (...)
        {
            std::cerr << "Unknown ANTLR Parsing Exception" << std::endl;
            return nullptr;
        }
    }

} // namespace Mycelium::Scripting
