#include "ast_converter.hpp"
#include "preprocessor/MySimpleUIParser.h" // Include parser context definitions
#include <iostream> // For potential debugging
#include <stdexcept> // For error handling

namespace Mycelium::Scripting
{

    // --- Helper Implementations ---

    void AstConverter::indent()
    {
        indentLevel++;
    }

    void AstConverter::unindent()
    {
        if (indentLevel > 0)
        {
            indentLevel--;
        }
    }

    std::string AstConverter::getIndent()
    {
        return std::string(indentLevel * 4, ' '); // 4 spaces per indent level
    }

    std::string AstConverter::getText(antlr4::tree::ParseTree* node)
    {
        if (!node) return "";

        // Simple text extraction, might need refinement for complex nodes
        return node->getText();
    }

    std::string AstConverter::getText(antlr4::Token* token)
    {
        if (!token) return "";

        return token->getText();
    }


    // --- Public Generate Method ---

    std::string AstConverter::generate(antlr4::tree::ParseTree* tree)
    {
        outputBuffer.str(""); // Clear the buffer
        outputBuffer.clear();
        indentLevel = 0;
        visit(tree); // Start visiting from the root
        return outputBuffer.str();
    }

    // --- Visitor Overrides ---

    std::any AstConverter::visitProgram(MySimpleUIParser::ProgramContext* ctx)
    {
        outputBuffer << getIndent() << "// Program Start\n";

        // Visit state declarations first
        for (auto stateDecl : ctx->stateDecl())
        {
            visit(stateDecl);
        }

        // Then visit the root component
        if (ctx->rootComponent())
        {
            visit(ctx->rootComponent());
        }

        outputBuffer << getIndent() << "// Program End\n";
        return nullptr; // Return value not used here
    }

    std::any AstConverter::visitRootComponent(MySimpleUIParser::RootComponentContext* ctx)
    {
        outputBuffer << getIndent() << "// Root Component Start\n";
        visit(ctx->componentCall()); // Visit the single root component call
        outputBuffer << getIndent() << "// Root Component End\n";
        return nullptr;
    }


    std::any AstConverter::visitStateDecl(MySimpleUIParser::StateDeclContext* ctx)
    {
        // Use the labels defined in the grammar: type=ID, name=ID
        std::string stateType = getText(ctx->type); // Access labeled token 'type'
        std::string stateName = getText(ctx->name); // Access labeled token 'name'
        // Visit the expression to get the initial value representation
        std::string initialValue = std::any_cast<std::string>(visit(ctx->value)); // Access labeled rule 'value'

        outputBuffer << getIndent() << "declare_state(\"" << stateType << "\", \""
                     << stateName << "\", " << initialValue << ");\n";
        return nullptr;
    }

    std::any AstConverter::visitComponentCall(MySimpleUIParser::ComponentCallContext* ctx)
    {
        // Use the label defined in the grammar: componentName=ID
        std::string componentName = getText(ctx->componentName); // Access labeled token 'componentName'
        outputBuffer << getIndent() << "begin_component(\"" << componentName << "\");\n";
        indent();

        // Visit parameters if they exist
        if (ctx->parameterList())
        {
            outputBuffer << getIndent() << "// Parameters:\n";
            visit(ctx->parameterList());
        }


        // Visit child elements (properties and nested components)
        outputBuffer << getIndent() << "// Elements:\n";

        for (auto element : ctx->element())
        {
            visit(element); // Visit each property or nested component
        }

        unindent();
        outputBuffer << getIndent() << "end_component();\n";
        return nullptr;
    }

    std::any AstConverter::visitElement(MySimpleUIParser::ElementContext* ctx)
    {
        // An element can be either a property assignment or another component call
        if (ctx->propertyAssignment())
        {
            visit(ctx->propertyAssignment());
        }
        else if (ctx->componentCall())
        {
            visit(ctx->componentCall());
        }

        // Add handling for other element types if the grammar expands
        return nullptr;
    }


    std::any AstConverter::visitPropertyAssignment(MySimpleUIParser::PropertyAssignmentContext* ctx)
    {
        // Use the label defined in the grammar: propertyName=ID
        std::string propertyName = getText(ctx->propertyName); // Access labeled token 'propertyName'
        // Visit the expression to get the value representation
        std::string value = std::any_cast<std::string>(visit(ctx->value)); // Access labeled rule 'value'

        outputBuffer << getIndent() << "set_property(\"" << propertyName << "\", " << value << ");\n";
        return nullptr;
    }

    std::any AstConverter::visitParameterList(MySimpleUIParser::ParameterListContext* ctx)
    {
        // Visit each parameter in the list
        for (auto paramCtx : ctx->parameter())
        {
            visit(paramCtx);
        }

        return nullptr;
    }

    std::any AstConverter::visitParameter(MySimpleUIParser::ParameterContext* ctx)
    {
        // Parameters can be 'bind identifier' or just an expression
        std::string paramValue;

        if (ctx->BIND()) // Check if BIND keyword exists
        {
            // Need to safely get the identifier text from the expression
            std::string boundVar = "<error_binding_non_identifier>"; // Default error value

            if (ctx->expression() && ctx->expression()->identifier())
            {
                auto idCtx = ctx->expression()->identifier();

                if (idCtx->ID())   // Check if it's a regular ID
                {
                    boundVar = getText(idCtx->ID());
                }

                // Note: Binding to @something (AT_ID) might not make sense, handle if needed
            }

            paramValue = "bind_to(\"" + boundVar + "\")"; // Special representation for binding
        }
        else
        {
            paramValue = std::any_cast<std::string>(visit(ctx->expression()));
        }

        outputBuffer << getIndent() << "add_parameter(" << paramValue << ");\n";
        return nullptr;
    }


    std::any AstConverter::visitExpression(MySimpleUIParser::ExpressionContext* ctx)
    {
        // Determine the type of expression and visit the appropriate child
        // Return the string representation of the expression's value
        if (ctx->literal())
        {
            return visit(ctx->literal());
        }
        else if (ctx->identifier())
        {
            return visit(ctx->identifier());
        }

        // Add more expression types if needed (e.g., binary operations)
        return std::string("\"<unknown_expression>\""); // Placeholder
    }

    std::any AstConverter::visitLiteral(MySimpleUIParser::LiteralContext* ctx)
    {
        // Check which terminal node exists (NUMBER or STRING) based on the grammar
        if (ctx->STRING()) // Check for STRING token
        {
            // Keep the quotes for string literals in the C API representation
            return std::string(getText(ctx->STRING()));
        }
        else if (ctx->NUMBER()) // Check for NUMBER token (covers int and float)
        {
            return std::string(getText(ctx->NUMBER()));
        }

        // Add boolean etc. if defined in grammar
        return std::string("\"<unknown_literal>\"");
    }

    std::any AstConverter::visitIdentifier(MySimpleUIParser::IdentifierContext* ctx)
    {
        // Check which terminal node exists (ID or AT_ID) based on the grammar
        if (ctx->AT_ID()) // Check for @delegateName first
        {
            std::string handlerName = getText(ctx->AT_ID());

            // Remove the leading '@' for the C API call? Or keep it? Let's remove it for now.
            if (!handlerName.empty() && handlerName[0] == '@')
            {
                handlerName = handlerName.substr(1);
            }

            return std::string("event_handler(\"" + handlerName + "\")"); // Represent event handlers
        }
        else if (ctx->ID()) // Check for regular identifier
        {
            std::string id = getText(ctx->ID());

            // Check if it's a known keyword/enum value used as an identifier
            if (id == "true" || id == "false" || id == "null" || id == "grey" || id == "white" || id == "left" || id == "decimal" || id == "black" || id == "vertical")   // Example enum-like values
            {
                return std::string("enum_val(\"" + id + "\")"); // Represent known constants/enums
            }
            else
            {
                return std::string("variable(\"" + id + "\")"); // Represent variable access
            }
        }

        return std::string("\"<unknown_identifier>\"");
    }

    // Implement other visit* methods as needed based on your grammar...

} // namespace Mycelium::Scripting
