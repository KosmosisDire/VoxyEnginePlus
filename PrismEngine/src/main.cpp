#include "project_paths.h"
#include "voxel-app.hpp"
#include <engine/apis/Resources.hpp>
#include <scripting/scripting.hpp>
#include <scripting/parser_interface.hpp> // Include the new parser interface
#include <iostream> // For test output

auto main() -> int
{
    // --- Test ANTLR Parser Interface ---
    std::cout << "--- Testing ANTLR Parser ---" << std::endl;
    std::string testScript =
        "// test.mysimpleui - Example for MySimpleUI subset parser\n"
        "\n"
        "// --- State Declarations ---\n"
        "// Test different types and initial values\n"
        "state string appTitle = \"My Simple UI Test\";\n"
        "state float defaultWidth = 200.0;\n"
        "state int counter = 0;\n"
        "state bool showDetails = true;\n"
        "\n"
        "// --- Root UI Definition ---\n"
        "// A single Column component at the root\n"
        "Column { // Root component call, no parameters, has body\n"
        "\n"
        "    // --- Property Assignments ---\n"
        "    // Using number and identifier values\n"
        "    padding: 10;\n"
        "    background: grey; // 'grey' treated as an identifier keyword value\n"
        "\n"
        "    // --- Nested Component Call with Parameters and Body ---\n"
        "    // Uses string literal, state variable identifier, and @delegate identifier\n"
        "    Header(appTitle, @OnHeaderClick) {\n"
        "        fontSize: 24; // Property using number\n"
        "        color: white; // Property using identifier keyword\n"
        "    }\n"
        "\n"
        "    // --- Nested Component with different parameter types ---\n"
        "    // Uses bind, number literals\n"
        "    Slider(bind counter, 0, 100) { // Note: 'bind' is a keyword before the identifier\n"
        "        width: defaultWidth; // Property using state variable identifier\n"
        "        height: 20;\n"
        "    }\n"
        "\n"
        "    // --- Nested Component with only literal parameters ---\n"
        "    Text(\"Current Count:\") {\n"
        "        align: left; // Property using identifier keyword\n"
        "    }\n"
        "\n"
        "    // --- Nested Component using state variable in parameter ---\n"
        "    DisplayValue(counter) { // Passing state variable 'counter' as parameter\n"
        "        format: decimal;\n"
        "    }\n"
        "\n"
        "    // --- Nested Component with no parameters, but with body ---\n"
        "    Panel() {\n"
        "        borderWidth: 1;\n"
        "        borderColor: black;\n"
        "\n"
        "        // --- Deeper Nesting ---\n"
        "        Button(\"Toggle Details\", @ToggleDetails) {\n"
        "             // Properties inside the nested button\n"
        "             margin: 5;\n"
        "             enabled: showDetails; // Property using state variable\n"
        "        }\n"
        "    }\n"
        "\n"
        "    // --- Nested Component with parameters, but no body ---\n"
        "    // Note the empty {} required by this specific grammar design\n"
        "    Separator(vertical) {} // Parameter is an identifier keyword\n"
        "\n"
        "    // --- Nested Component with no parameters and no body ---\n"
        "    // Still requires () and {}\n"
        "    Icon() {}\n"
        "\n"
        "} // End of root Column component\n";

    std::cout << "Parsing script 1: '" << testScript << "'" << std::endl;
    auto tree1 = Mycelium::Scripting::ParserInterface::parseScript(testScript);
    std::cout << "Parse result 1 (expecting tree output above if successful): " << (tree1 ? "Success (Pointer returned)" : "Failure (nullptr returned)") << std::endl; // Note: Currently always returns nullptr

    std::cout << "--- End ANTLR Parser Test ---" << std::endl << std::endl;
    // --- End Test ---

    scriptingEngine.registerStdLib();
    scriptingEngine.registerEngineAPI();

    VoxyApp app;

    scripts->addScriptFile(Resources::GetResourcePath("test.as").string());
    scriptingEngine.savePredefinedScript(PROJECT_ROOT_DIR "/as.predefined");

    while (true)
    {
        scriptingEngine.checkHotReload();

        if (app.Update())
        {
            break;
        }
    }

    return 0;
}
