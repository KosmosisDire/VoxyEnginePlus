#ifndef MYCELIUM_PARSER_INTERFACE_HPP
#define MYCELIUM_PARSER_INTERFACE_HPP

#include <string>
#include <memory>

// Forward declare ANTLR types to avoid including heavy headers here
namespace antlr4
{
    class ANTLRInputStream;
    class CommonTokenStream;
    namespace tree
    {
        class ParseTree;
    }
}

// Include generated lexer and parser headers directly
#include "preprocessor/MySimpleUILexer.h"
#include "preprocessor/MySimpleUIParser.h"

namespace Mycelium::Scripting
{

    /**
     * @brief Provides an interface to the MySimpleUI ANTLR parser.
     */
    class ParserInterface
    {
        public:
            /**
             * @brief Parses the given script content using the MySimpleUI parser.
             *
             * @param scriptContent The string content of the script to parse.
             * @return A shared pointer to the ANTLR ParseTree. Returns nullptr on error.
             *         (Consider returning a more abstract representation or error info later)
             */
            static std::shared_ptr<antlr4::tree::ParseTree> parseScript(const std::string& scriptContent);

            // Potentially add more methods here later, e.g., to walk the tree, extract specific info, etc.

        private:
            // Private constructor to prevent instantiation if only static methods are needed
            ParserInterface() = default;
    };

} // namespace Mycelium::Scripting

#endif // MYCELIUM_PARSER_INTERFACE_HPP
