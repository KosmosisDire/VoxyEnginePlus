#pragma once

#include "preprocessor/MySimpleUIBaseVisitor.h" // Include the ANTLR generated base visitor
#include <string>
#include <sstream>
#include <any> // To return generated code or other data

namespace Mycelium::Scripting
{
    /**
     * @brief Visits the ANTLR parse tree and converts it into a C-API style representation.
     *
     * This class traverses the parse tree generated from a .flow script and outputs
     * a string simulating C-API calls, which can later be used to generate
     * AngelScript or directly interact with a C/C++ backend.
     */
    class AstConverter : public MySimpleUIBaseVisitor
    {
        private:
            std::stringstream outputBuffer; // Buffer to build the C-API call string
            int indentLevel = 0;

            // Helper to manage indentation
            void indent();
            void unindent();
            std::string getIndent();

            // Helper to extract text from terminals or contexts safely
            std::string getText(antlr4::tree::ParseTree* node);
            std::string getText(antlr4::Token* token);


        public:
            /**
             * @brief Generates the C-API call string from a given parse tree.
             * @param tree The root of the parse tree to visit.
             * @return A string containing the generated C-API calls.
             */
            std::string generate(antlr4::tree::ParseTree* tree);

            // --- Visitor Overrides ---
            // We will override methods for the grammar rules we want to translate.

            // Example: Visiting a component call like `Column { ... }`
            std::any visitComponentCall(MySimpleUIParser::ComponentCallContext* ctx) override;

            // Example: Visiting a property assignment like `padding: 10;`
            std::any visitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext* ctx) override;

            // Example: Visiting a state declaration like `state string appTitle = "Test";`
            std::any visitStateDecl(MySimpleUIParser::StateDeclContext* ctx) override;

            // Example: Visiting expressions and literals
            std::any visitExpression(MySimpleUIParser::ExpressionContext* ctx) override;
            std::any visitLiteral(MySimpleUIParser::LiteralContext* ctx) override;
            std::any visitIdentifier(MySimpleUIParser::IdentifierContext* ctx) override;

            // Visiting the root program node
            std::any visitProgram(MySimpleUIParser::ProgramContext* ctx) override;

            // Visiting the root component node
            std::any visitRootComponent(MySimpleUIParser::RootComponentContext* ctx) override;

            // Visiting elements within a component
            std::any visitElement(MySimpleUIParser::ElementContext* ctx) override;

            // Visiting parameter lists
            std::any visitParameterList(MySimpleUIParser::ParameterListContext* ctx) override;
            std::any visitParameter(MySimpleUIParser::ParameterContext* ctx) override;

            // Add overrides for other relevant grammar rules as needed...
    };

} // namespace Mycelium::Scripting
