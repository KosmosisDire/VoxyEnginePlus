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
#include "ast_converter.hpp" // Include the converter

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
            /**
             * @brief Parses the script and generates a C-API style string representation.
             *
             * @param scriptContent The string content of the script to parse and convert.
             * @return A string containing the generated C-API calls. Returns an error message string on failure.
             */
            static std::string generateCApi(const std::string& scriptContent);


        private:
            // Private constructor to prevent instantiation if only static methods are needed
            ParserInterface() = default;
    };

} // namespace Mycelium::Scripting

#endif // MYCELIUM_PARSER_INTERFACE_HPP
