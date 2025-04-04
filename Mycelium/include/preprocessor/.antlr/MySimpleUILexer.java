// Generated from c:/Main/Projects/Coding/C++/VoxyEnginePlus/Mycelium/include/preprocessor/MySimpleUI.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue", "this-escape"})
public class MySimpleUILexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		STATE=1, BIND=2, NUMBER=3, STRING=4, ID=5, AT_ID=6, LPAREN=7, RPAREN=8, 
		LBRACE=9, RBRACE=10, COLON=11, SEMI=12, ASSIGN=13, COMMA=14, WS=15, LINE_COMMENT=16;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"STATE", "BIND", "NUMBER", "STRING", "ID", "AT_ID", "LPAREN", "RPAREN", 
			"LBRACE", "RBRACE", "COLON", "SEMI", "ASSIGN", "COMMA", "WS", "LINE_COMMENT"
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


	public MySimpleUILexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "MySimpleUI.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\u0004\u0000\u0010s\u0006\uffff\uffff\u0002\u0000\u0007\u0000\u0002\u0001"+
		"\u0007\u0001\u0002\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004"+
		"\u0007\u0004\u0002\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002\u0007"+
		"\u0007\u0007\u0002\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0002\u000b"+
		"\u0007\u000b\u0002\f\u0007\f\u0002\r\u0007\r\u0002\u000e\u0007\u000e\u0002"+
		"\u000f\u0007\u000f\u0001\u0000\u0001\u0000\u0001\u0000\u0001\u0000\u0001"+
		"\u0000\u0001\u0000\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001\u0001"+
		"\u0001\u0001\u0002\u0004\u0002.\b\u0002\u000b\u0002\f\u0002/\u0001\u0002"+
		"\u0001\u0002\u0004\u00024\b\u0002\u000b\u0002\f\u00025\u0003\u00028\b"+
		"\u0002\u0001\u0003\u0001\u0003\u0005\u0003<\b\u0003\n\u0003\f\u0003?\t"+
		"\u0003\u0001\u0003\u0001\u0003\u0001\u0004\u0001\u0004\u0005\u0004E\b"+
		"\u0004\n\u0004\f\u0004H\t\u0004\u0001\u0005\u0001\u0005\u0001\u0005\u0005"+
		"\u0005M\b\u0005\n\u0005\f\u0005P\t\u0005\u0001\u0006\u0001\u0006\u0001"+
		"\u0007\u0001\u0007\u0001\b\u0001\b\u0001\t\u0001\t\u0001\n\u0001\n\u0001"+
		"\u000b\u0001\u000b\u0001\f\u0001\f\u0001\r\u0001\r\u0001\u000e\u0004\u000e"+
		"c\b\u000e\u000b\u000e\f\u000ed\u0001\u000e\u0001\u000e\u0001\u000f\u0001"+
		"\u000f\u0001\u000f\u0001\u000f\u0005\u000fm\b\u000f\n\u000f\f\u000fp\t"+
		"\u000f\u0001\u000f\u0001\u000f\u0001=\u0000\u0010\u0001\u0001\u0003\u0002"+
		"\u0005\u0003\u0007\u0004\t\u0005\u000b\u0006\r\u0007\u000f\b\u0011\t\u0013"+
		"\n\u0015\u000b\u0017\f\u0019\r\u001b\u000e\u001d\u000f\u001f\u0010\u0001"+
		"\u0000\u0006\u0001\u000009\u0003\u0000\n\n\r\r\"\"\u0003\u0000AZ__az\u0004"+
		"\u000009AZ__az\u0003\u0000\t\n\r\r  \u0002\u0000\n\n\r\rz\u0000\u0001"+
		"\u0001\u0000\u0000\u0000\u0000\u0003\u0001\u0000\u0000\u0000\u0000\u0005"+
		"\u0001\u0000\u0000\u0000\u0000\u0007\u0001\u0000\u0000\u0000\u0000\t\u0001"+
		"\u0000\u0000\u0000\u0000\u000b\u0001\u0000\u0000\u0000\u0000\r\u0001\u0000"+
		"\u0000\u0000\u0000\u000f\u0001\u0000\u0000\u0000\u0000\u0011\u0001\u0000"+
		"\u0000\u0000\u0000\u0013\u0001\u0000\u0000\u0000\u0000\u0015\u0001\u0000"+
		"\u0000\u0000\u0000\u0017\u0001\u0000\u0000\u0000\u0000\u0019\u0001\u0000"+
		"\u0000\u0000\u0000\u001b\u0001\u0000\u0000\u0000\u0000\u001d\u0001\u0000"+
		"\u0000\u0000\u0000\u001f\u0001\u0000\u0000\u0000\u0001!\u0001\u0000\u0000"+
		"\u0000\u0003\'\u0001\u0000\u0000\u0000\u0005-\u0001\u0000\u0000\u0000"+
		"\u00079\u0001\u0000\u0000\u0000\tB\u0001\u0000\u0000\u0000\u000bI\u0001"+
		"\u0000\u0000\u0000\rQ\u0001\u0000\u0000\u0000\u000fS\u0001\u0000\u0000"+
		"\u0000\u0011U\u0001\u0000\u0000\u0000\u0013W\u0001\u0000\u0000\u0000\u0015"+
		"Y\u0001\u0000\u0000\u0000\u0017[\u0001\u0000\u0000\u0000\u0019]\u0001"+
		"\u0000\u0000\u0000\u001b_\u0001\u0000\u0000\u0000\u001db\u0001\u0000\u0000"+
		"\u0000\u001fh\u0001\u0000\u0000\u0000!\"\u0005s\u0000\u0000\"#\u0005t"+
		"\u0000\u0000#$\u0005a\u0000\u0000$%\u0005t\u0000\u0000%&\u0005e\u0000"+
		"\u0000&\u0002\u0001\u0000\u0000\u0000\'(\u0005b\u0000\u0000()\u0005i\u0000"+
		"\u0000)*\u0005n\u0000\u0000*+\u0005d\u0000\u0000+\u0004\u0001\u0000\u0000"+
		"\u0000,.\u0007\u0000\u0000\u0000-,\u0001\u0000\u0000\u0000./\u0001\u0000"+
		"\u0000\u0000/-\u0001\u0000\u0000\u0000/0\u0001\u0000\u0000\u000007\u0001"+
		"\u0000\u0000\u000013\u0005.\u0000\u000024\u0007\u0000\u0000\u000032\u0001"+
		"\u0000\u0000\u000045\u0001\u0000\u0000\u000053\u0001\u0000\u0000\u0000"+
		"56\u0001\u0000\u0000\u000068\u0001\u0000\u0000\u000071\u0001\u0000\u0000"+
		"\u000078\u0001\u0000\u0000\u00008\u0006\u0001\u0000\u0000\u00009=\u0005"+
		"\"\u0000\u0000:<\b\u0001\u0000\u0000;:\u0001\u0000\u0000\u0000<?\u0001"+
		"\u0000\u0000\u0000=>\u0001\u0000\u0000\u0000=;\u0001\u0000\u0000\u0000"+
		">@\u0001\u0000\u0000\u0000?=\u0001\u0000\u0000\u0000@A\u0005\"\u0000\u0000"+
		"A\b\u0001\u0000\u0000\u0000BF\u0007\u0002\u0000\u0000CE\u0007\u0003\u0000"+
		"\u0000DC\u0001\u0000\u0000\u0000EH\u0001\u0000\u0000\u0000FD\u0001\u0000"+
		"\u0000\u0000FG\u0001\u0000\u0000\u0000G\n\u0001\u0000\u0000\u0000HF\u0001"+
		"\u0000\u0000\u0000IJ\u0005@\u0000\u0000JN\u0007\u0002\u0000\u0000KM\u0007"+
		"\u0003\u0000\u0000LK\u0001\u0000\u0000\u0000MP\u0001\u0000\u0000\u0000"+
		"NL\u0001\u0000\u0000\u0000NO\u0001\u0000\u0000\u0000O\f\u0001\u0000\u0000"+
		"\u0000PN\u0001\u0000\u0000\u0000QR\u0005(\u0000\u0000R\u000e\u0001\u0000"+
		"\u0000\u0000ST\u0005)\u0000\u0000T\u0010\u0001\u0000\u0000\u0000UV\u0005"+
		"{\u0000\u0000V\u0012\u0001\u0000\u0000\u0000WX\u0005}\u0000\u0000X\u0014"+
		"\u0001\u0000\u0000\u0000YZ\u0005:\u0000\u0000Z\u0016\u0001\u0000\u0000"+
		"\u0000[\\\u0005;\u0000\u0000\\\u0018\u0001\u0000\u0000\u0000]^\u0005="+
		"\u0000\u0000^\u001a\u0001\u0000\u0000\u0000_`\u0005,\u0000\u0000`\u001c"+
		"\u0001\u0000\u0000\u0000ac\u0007\u0004\u0000\u0000ba\u0001\u0000\u0000"+
		"\u0000cd\u0001\u0000\u0000\u0000db\u0001\u0000\u0000\u0000de\u0001\u0000"+
		"\u0000\u0000ef\u0001\u0000\u0000\u0000fg\u0006\u000e\u0000\u0000g\u001e"+
		"\u0001\u0000\u0000\u0000hi\u0005/\u0000\u0000ij\u0005/\u0000\u0000jn\u0001"+
		"\u0000\u0000\u0000km\b\u0005\u0000\u0000lk\u0001\u0000\u0000\u0000mp\u0001"+
		"\u0000\u0000\u0000nl\u0001\u0000\u0000\u0000no\u0001\u0000\u0000\u0000"+
		"oq\u0001\u0000\u0000\u0000pn\u0001\u0000\u0000\u0000qr\u0006\u000f\u0000"+
		"\u0000r \u0001\u0000\u0000\u0000\t\u0000/57=FNdn\u0001\u0006\u0000\u0000";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}