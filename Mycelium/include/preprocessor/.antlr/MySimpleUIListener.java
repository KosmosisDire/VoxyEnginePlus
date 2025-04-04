// Generated from c:/Main/Projects/Coding/C++/VoxyEnginePlus/Mycelium/include/preprocessor/MySimpleUI.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link MySimpleUIParser}.
 */
public interface MySimpleUIListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#program}.
	 * @param ctx the parse tree
	 */
	void enterProgram(MySimpleUIParser.ProgramContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#program}.
	 * @param ctx the parse tree
	 */
	void exitProgram(MySimpleUIParser.ProgramContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#stateDecl}.
	 * @param ctx the parse tree
	 */
	void enterStateDecl(MySimpleUIParser.StateDeclContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#stateDecl}.
	 * @param ctx the parse tree
	 */
	void exitStateDecl(MySimpleUIParser.StateDeclContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#rootComponent}.
	 * @param ctx the parse tree
	 */
	void enterRootComponent(MySimpleUIParser.RootComponentContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#rootComponent}.
	 * @param ctx the parse tree
	 */
	void exitRootComponent(MySimpleUIParser.RootComponentContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#componentCall}.
	 * @param ctx the parse tree
	 */
	void enterComponentCall(MySimpleUIParser.ComponentCallContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#componentCall}.
	 * @param ctx the parse tree
	 */
	void exitComponentCall(MySimpleUIParser.ComponentCallContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#parameterList}.
	 * @param ctx the parse tree
	 */
	void enterParameterList(MySimpleUIParser.ParameterListContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#parameterList}.
	 * @param ctx the parse tree
	 */
	void exitParameterList(MySimpleUIParser.ParameterListContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#parameter}.
	 * @param ctx the parse tree
	 */
	void enterParameter(MySimpleUIParser.ParameterContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#parameter}.
	 * @param ctx the parse tree
	 */
	void exitParameter(MySimpleUIParser.ParameterContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#element}.
	 * @param ctx the parse tree
	 */
	void enterElement(MySimpleUIParser.ElementContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#element}.
	 * @param ctx the parse tree
	 */
	void exitElement(MySimpleUIParser.ElementContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#propertyAssignment}.
	 * @param ctx the parse tree
	 */
	void enterPropertyAssignment(MySimpleUIParser.PropertyAssignmentContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#propertyAssignment}.
	 * @param ctx the parse tree
	 */
	void exitPropertyAssignment(MySimpleUIParser.PropertyAssignmentContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#expression}.
	 * @param ctx the parse tree
	 */
	void enterExpression(MySimpleUIParser.ExpressionContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#expression}.
	 * @param ctx the parse tree
	 */
	void exitExpression(MySimpleUIParser.ExpressionContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#literal}.
	 * @param ctx the parse tree
	 */
	void enterLiteral(MySimpleUIParser.LiteralContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#literal}.
	 * @param ctx the parse tree
	 */
	void exitLiteral(MySimpleUIParser.LiteralContext ctx);
	/**
	 * Enter a parse tree produced by {@link MySimpleUIParser#identifier}.
	 * @param ctx the parse tree
	 */
	void enterIdentifier(MySimpleUIParser.IdentifierContext ctx);
	/**
	 * Exit a parse tree produced by {@link MySimpleUIParser#identifier}.
	 * @param ctx the parse tree
	 */
	void exitIdentifier(MySimpleUIParser.IdentifierContext ctx);
}