// Generated from c:/Main/Projects/Coding/C++/VoxyEnginePlus/Mycelium/include/preprocessor/MySimpleUI.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue"})
public class MySimpleUIParser extends Parser {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		STATE=1, BIND=2, NUMBER=3, STRING=4, ID=5, AT_ID=6, LPAREN=7, RPAREN=8, 
		LBRACE=9, RBRACE=10, COLON=11, SEMI=12, ASSIGN=13, COMMA=14, WS=15, LINE_COMMENT=16;
	public static final int
		RULE_program = 0, RULE_stateDecl = 1, RULE_rootComponent = 2, RULE_componentCall = 3, 
		RULE_parameterList = 4, RULE_parameter = 5, RULE_element = 6, RULE_propertyAssignment = 7, 
		RULE_expression = 8, RULE_literal = 9, RULE_identifier = 10;
	private static String[] makeRuleNames() {
		return new String[] {
			"program", "stateDecl", "rootComponent", "componentCall", "parameterList", 
			"parameter", "element", "propertyAssignment", "expression", "literal", 
			"identifier"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'state'", "'bind'", null, null, null, null, "'('", "')'", "'{'", 
			"'}'", "':'", "';'", "'='", "','"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, "STATE", "BIND", "NUMBER", "STRING", "ID", "AT_ID", "LPAREN", "RPAREN", 
			"LBRACE", "RBRACE", "COLON", "SEMI", "ASSIGN", "COMMA", "WS", "LINE_COMMENT"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}

	@Override
	public String getGrammarFileName() { return "MySimpleUI.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public MySimpleUIParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ProgramContext extends ParserRuleContext {
		public RootComponentContext rootComponent() {
			return getRuleContext(RootComponentContext.class,0);
		}
		public TerminalNode EOF() { return getToken(MySimpleUIParser.EOF, 0); }
		public List<StateDeclContext> stateDecl() {
			return getRuleContexts(StateDeclContext.class);
		}
		public StateDeclContext stateDecl(int i) {
			return getRuleContext(StateDeclContext.class,i);
		}
		public ProgramContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_program; }
	}

	public final ProgramContext program() throws RecognitionException {
		ProgramContext _localctx = new ProgramContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_program);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(25);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==STATE) {
				{
				{
				setState(22);
				stateDecl();
				}
				}
				setState(27);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(28);
			rootComponent();
			setState(29);
			match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class StateDeclContext extends ParserRuleContext {
		public Token type;
		public Token name;
		public ExpressionContext value;
		public TerminalNode STATE() { return getToken(MySimpleUIParser.STATE, 0); }
		public TerminalNode ASSIGN() { return getToken(MySimpleUIParser.ASSIGN, 0); }
		public TerminalNode SEMI() { return getToken(MySimpleUIParser.SEMI, 0); }
		public List<TerminalNode> ID() { return getTokens(MySimpleUIParser.ID); }
		public TerminalNode ID(int i) {
			return getToken(MySimpleUIParser.ID, i);
		}
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public StateDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_stateDecl; }
	}

	public final StateDeclContext stateDecl() throws RecognitionException {
		StateDeclContext _localctx = new StateDeclContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_stateDecl);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(31);
			match(STATE);
			setState(32);
			((StateDeclContext)_localctx).type = match(ID);
			setState(33);
			((StateDeclContext)_localctx).name = match(ID);
			setState(34);
			match(ASSIGN);
			setState(35);
			((StateDeclContext)_localctx).value = expression();
			setState(36);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class RootComponentContext extends ParserRuleContext {
		public ComponentCallContext componentCall() {
			return getRuleContext(ComponentCallContext.class,0);
		}
		public RootComponentContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_rootComponent; }
	}

	public final RootComponentContext rootComponent() throws RecognitionException {
		RootComponentContext _localctx = new RootComponentContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_rootComponent);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(38);
			componentCall();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ComponentCallContext extends ParserRuleContext {
		public Token componentName;
		public TerminalNode LBRACE() { return getToken(MySimpleUIParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(MySimpleUIParser.RBRACE, 0); }
		public TerminalNode ID() { return getToken(MySimpleUIParser.ID, 0); }
		public TerminalNode LPAREN() { return getToken(MySimpleUIParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(MySimpleUIParser.RPAREN, 0); }
		public List<ElementContext> element() {
			return getRuleContexts(ElementContext.class);
		}
		public ElementContext element(int i) {
			return getRuleContext(ElementContext.class,i);
		}
		public ParameterListContext parameterList() {
			return getRuleContext(ParameterListContext.class,0);
		}
		public ComponentCallContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_componentCall; }
	}

	public final ComponentCallContext componentCall() throws RecognitionException {
		ComponentCallContext _localctx = new ComponentCallContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_componentCall);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(40);
			((ComponentCallContext)_localctx).componentName = match(ID);
			setState(46);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==LPAREN) {
				{
				setState(41);
				match(LPAREN);
				setState(43);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 124L) != 0)) {
					{
					setState(42);
					parameterList();
					}
				}

				setState(45);
				match(RPAREN);
				}
			}

			setState(48);
			match(LBRACE);
			setState(52);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==ID) {
				{
				{
				setState(49);
				element();
				}
				}
				setState(54);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(55);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParameterListContext extends ParserRuleContext {
		public List<ParameterContext> parameter() {
			return getRuleContexts(ParameterContext.class);
		}
		public ParameterContext parameter(int i) {
			return getRuleContext(ParameterContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(MySimpleUIParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(MySimpleUIParser.COMMA, i);
		}
		public ParameterListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_parameterList; }
	}

	public final ParameterListContext parameterList() throws RecognitionException {
		ParameterListContext _localctx = new ParameterListContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_parameterList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(57);
			parameter();
			setState(62);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(58);
				match(COMMA);
				setState(59);
				parameter();
				}
				}
				setState(64);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParameterContext extends ParserRuleContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode BIND() { return getToken(MySimpleUIParser.BIND, 0); }
		public ParameterContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_parameter; }
	}

	public final ParameterContext parameter() throws RecognitionException {
		ParameterContext _localctx = new ParameterContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_parameter);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(66);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==BIND) {
				{
				setState(65);
				match(BIND);
				}
			}

			setState(68);
			expression();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ElementContext extends ParserRuleContext {
		public PropertyAssignmentContext propertyAssignment() {
			return getRuleContext(PropertyAssignmentContext.class,0);
		}
		public ComponentCallContext componentCall() {
			return getRuleContext(ComponentCallContext.class,0);
		}
		public ElementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_element; }
	}

	public final ElementContext element() throws RecognitionException {
		ElementContext _localctx = new ElementContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_element);
		try {
			setState(72);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,6,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(70);
				propertyAssignment();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(71);
				componentCall();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PropertyAssignmentContext extends ParserRuleContext {
		public Token propertyName;
		public ExpressionContext value;
		public TerminalNode COLON() { return getToken(MySimpleUIParser.COLON, 0); }
		public TerminalNode SEMI() { return getToken(MySimpleUIParser.SEMI, 0); }
		public TerminalNode ID() { return getToken(MySimpleUIParser.ID, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public PropertyAssignmentContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_propertyAssignment; }
	}

	public final PropertyAssignmentContext propertyAssignment() throws RecognitionException {
		PropertyAssignmentContext _localctx = new PropertyAssignmentContext(_ctx, getState());
		enterRule(_localctx, 14, RULE_propertyAssignment);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(74);
			((PropertyAssignmentContext)_localctx).propertyName = match(ID);
			setState(75);
			match(COLON);
			setState(76);
			((PropertyAssignmentContext)_localctx).value = expression();
			setState(77);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExpressionContext extends ParserRuleContext {
		public LiteralContext literal() {
			return getRuleContext(LiteralContext.class,0);
		}
		public IdentifierContext identifier() {
			return getRuleContext(IdentifierContext.class,0);
		}
		public ExpressionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_expression; }
	}

	public final ExpressionContext expression() throws RecognitionException {
		ExpressionContext _localctx = new ExpressionContext(_ctx, getState());
		enterRule(_localctx, 16, RULE_expression);
		try {
			setState(81);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case NUMBER:
			case STRING:
				enterOuterAlt(_localctx, 1);
				{
				setState(79);
				literal();
				}
				break;
			case ID:
			case AT_ID:
				enterOuterAlt(_localctx, 2);
				{
				setState(80);
				identifier();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class LiteralContext extends ParserRuleContext {
		public TerminalNode NUMBER() { return getToken(MySimpleUIParser.NUMBER, 0); }
		public TerminalNode STRING() { return getToken(MySimpleUIParser.STRING, 0); }
		public LiteralContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_literal; }
	}

	public final LiteralContext literal() throws RecognitionException {
		LiteralContext _localctx = new LiteralContext(_ctx, getState());
		enterRule(_localctx, 18, RULE_literal);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(83);
			_la = _input.LA(1);
			if ( !(_la==NUMBER || _la==STRING) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class IdentifierContext extends ParserRuleContext {
		public TerminalNode ID() { return getToken(MySimpleUIParser.ID, 0); }
		public TerminalNode AT_ID() { return getToken(MySimpleUIParser.AT_ID, 0); }
		public IdentifierContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_identifier; }
	}

	public final IdentifierContext identifier() throws RecognitionException {
		IdentifierContext _localctx = new IdentifierContext(_ctx, getState());
		enterRule(_localctx, 20, RULE_identifier);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(85);
			_la = _input.LA(1);
			if ( !(_la==ID || _la==AT_ID) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public static final String _serializedATN =
		"\u0004\u0001\u0010X\u0002\u0000\u0007\u0000\u0002\u0001\u0007\u0001\u0002"+
		"\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004\u0007\u0004\u0002"+
		"\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002\u0007\u0007\u0007\u0002"+
		"\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0001\u0000\u0005\u0000\u0018"+
		"\b\u0000\n\u0000\f\u0000\u001b\t\u0000\u0001\u0000\u0001\u0000\u0001\u0000"+
		"\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0001\u0002\u0001\u0002\u0001\u0003\u0001\u0003\u0001\u0003"+
		"\u0003\u0003,\b\u0003\u0001\u0003\u0003\u0003/\b\u0003\u0001\u0003\u0001"+
		"\u0003\u0005\u00033\b\u0003\n\u0003\f\u00036\t\u0003\u0001\u0003\u0001"+
		"\u0003\u0001\u0004\u0001\u0004\u0001\u0004\u0005\u0004=\b\u0004\n\u0004"+
		"\f\u0004@\t\u0004\u0001\u0005\u0003\u0005C\b\u0005\u0001\u0005\u0001\u0005"+
		"\u0001\u0006\u0001\u0006\u0003\u0006I\b\u0006\u0001\u0007\u0001\u0007"+
		"\u0001\u0007\u0001\u0007\u0001\u0007\u0001\b\u0001\b\u0003\bR\b\b\u0001"+
		"\t\u0001\t\u0001\n\u0001\n\u0001\n\u0000\u0000\u000b\u0000\u0002\u0004"+
		"\u0006\b\n\f\u000e\u0010\u0012\u0014\u0000\u0002\u0001\u0000\u0003\u0004"+
		"\u0001\u0000\u0005\u0006T\u0000\u0019\u0001\u0000\u0000\u0000\u0002\u001f"+
		"\u0001\u0000\u0000\u0000\u0004&\u0001\u0000\u0000\u0000\u0006(\u0001\u0000"+
		"\u0000\u0000\b9\u0001\u0000\u0000\u0000\nB\u0001\u0000\u0000\u0000\fH"+
		"\u0001\u0000\u0000\u0000\u000eJ\u0001\u0000\u0000\u0000\u0010Q\u0001\u0000"+
		"\u0000\u0000\u0012S\u0001\u0000\u0000\u0000\u0014U\u0001\u0000\u0000\u0000"+
		"\u0016\u0018\u0003\u0002\u0001\u0000\u0017\u0016\u0001\u0000\u0000\u0000"+
		"\u0018\u001b\u0001\u0000\u0000\u0000\u0019\u0017\u0001\u0000\u0000\u0000"+
		"\u0019\u001a\u0001\u0000\u0000\u0000\u001a\u001c\u0001\u0000\u0000\u0000"+
		"\u001b\u0019\u0001\u0000\u0000\u0000\u001c\u001d\u0003\u0004\u0002\u0000"+
		"\u001d\u001e\u0005\u0000\u0000\u0001\u001e\u0001\u0001\u0000\u0000\u0000"+
		"\u001f \u0005\u0001\u0000\u0000 !\u0005\u0005\u0000\u0000!\"\u0005\u0005"+
		"\u0000\u0000\"#\u0005\r\u0000\u0000#$\u0003\u0010\b\u0000$%\u0005\f\u0000"+
		"\u0000%\u0003\u0001\u0000\u0000\u0000&\'\u0003\u0006\u0003\u0000\'\u0005"+
		"\u0001\u0000\u0000\u0000(.\u0005\u0005\u0000\u0000)+\u0005\u0007\u0000"+
		"\u0000*,\u0003\b\u0004\u0000+*\u0001\u0000\u0000\u0000+,\u0001\u0000\u0000"+
		"\u0000,-\u0001\u0000\u0000\u0000-/\u0005\b\u0000\u0000.)\u0001\u0000\u0000"+
		"\u0000./\u0001\u0000\u0000\u0000/0\u0001\u0000\u0000\u000004\u0005\t\u0000"+
		"\u000013\u0003\f\u0006\u000021\u0001\u0000\u0000\u000036\u0001\u0000\u0000"+
		"\u000042\u0001\u0000\u0000\u000045\u0001\u0000\u0000\u000057\u0001\u0000"+
		"\u0000\u000064\u0001\u0000\u0000\u000078\u0005\n\u0000\u00008\u0007\u0001"+
		"\u0000\u0000\u00009>\u0003\n\u0005\u0000:;\u0005\u000e\u0000\u0000;=\u0003"+
		"\n\u0005\u0000<:\u0001\u0000\u0000\u0000=@\u0001\u0000\u0000\u0000><\u0001"+
		"\u0000\u0000\u0000>?\u0001\u0000\u0000\u0000?\t\u0001\u0000\u0000\u0000"+
		"@>\u0001\u0000\u0000\u0000AC\u0005\u0002\u0000\u0000BA\u0001\u0000\u0000"+
		"\u0000BC\u0001\u0000\u0000\u0000CD\u0001\u0000\u0000\u0000DE\u0003\u0010"+
		"\b\u0000E\u000b\u0001\u0000\u0000\u0000FI\u0003\u000e\u0007\u0000GI\u0003"+
		"\u0006\u0003\u0000HF\u0001\u0000\u0000\u0000HG\u0001\u0000\u0000\u0000"+
		"I\r\u0001\u0000\u0000\u0000JK\u0005\u0005\u0000\u0000KL\u0005\u000b\u0000"+
		"\u0000LM\u0003\u0010\b\u0000MN\u0005\f\u0000\u0000N\u000f\u0001\u0000"+
		"\u0000\u0000OR\u0003\u0012\t\u0000PR\u0003\u0014\n\u0000QO\u0001\u0000"+
		"\u0000\u0000QP\u0001\u0000\u0000\u0000R\u0011\u0001\u0000\u0000\u0000"+
		"ST\u0007\u0000\u0000\u0000T\u0013\u0001\u0000\u0000\u0000UV\u0007\u0001"+
		"\u0000\u0000V\u0015\u0001\u0000\u0000\u0000\b\u0019+.4>BHQ";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}