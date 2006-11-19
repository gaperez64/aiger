import java.io.InputStream;
import java.io.PrintStream;
import java.io.IOException;


public class JaigerToCNF {
	
	static final boolean debug = true;
	
	InputStream in;
	PrintStream out;
	
	int M, I, L, O, A;
	int single_output_literal;
	
	int charno;
	int lineno;
	int prev;
	
	JaigerToCNF (InputStream in, PrintStream out) {
		this.in = in;
		this.out = out;
		charno = 0;
		lineno = 0;
		prev = '\n';
	}
	
	int next () throws IOException {
		int res = in.read();
		if (debug)
			System.err.println ("JaigerToCNF.next:" +
			                    lineno + ":" + 
					    charno + ": " + res);
		if (res != -1) {
			charno++;
			if (prev == '\n')
				lineno++;
		}

		prev = res;
		return res;
	}
	
	int aigerlit2dimacslit (int l) {
		int res;
		
		assert (0 <= l);
		assert (l <= M);
		
		int sign = l & 1;
		int idx = l / 2;
		
		if (idx == 0)
			res = M + 1;
		else
			res = idx;
		
		if (sign != 0)
			res *= -1;
		
		return res;
	}
	
	void parseError (String msg) throws IOException {
	   throw new IOException ("line " + lineno + 
	                          ": character " + charno + ": " + msg);
	}

	int parseInt (char expected) throws IOException {
		int res, ch;
		ch = next ();

		if (ch < '0' || ch > '9')
			parseError ("expected digit");
		res = ch - '0';

		while ((ch = next ()) >= '0' && ch <= '9')
			res = 10 * res + (ch - '0');

		if (ch != expected)
			parseError ("unexpected character");

		return res;
	}
	
	void parseHeader () throws IOException {
		if (next () != 'a' || 
		    next () != 'i' ||
		    next () != 'g' ||
		    next () != ' ')
			parseError ("expected 'aig' header line");
		
		M = parseInt (' ');
		I = parseInt (' ');
		
		L = parseInt (' ');
		if (L > 0)
			parseError ("can not handle AIGs with latches");
		
		O = parseInt (' ');
		if (O != 1)
			parseError ("expected exactly one output");
		
		A = parseInt ('\n');
		
		if (M != I + A)
			parseError ("invalid header");
	}
	
	void parseOutput () throws IOException {
		single_output_literal = parseInt ('\n');
	}
	
	int safeGet () throws IOException {
		int ch = next ();
		if (ch==-1) {
			parseError ("unexpected EOF");
		}
		return ch;
	}
	
	int decode () throws IOException {
		int x = 0, i = 0;
		int ch;
		
		while (((ch = safeGet ()) & 0x80)>0) {
			x |= (ch & 0x7f) << (7 * i++);
		}
		
		return x | (ch << (7 * i));
	}
	
	void printLiteral (int aiger_literal) {
		out.print (aigerlit2dimacslit (aiger_literal) + " ");
	}
	
	void printBinaryClause (int l1, int l2) {
		printLiteral (l1);
		printLiteral (l2);
		out.println("0");
	}
	
	void printTernaryClause (int l1, int l2, int l3) {
		printLiteral (l1);
		printLiteral (l2);
		printLiteral (l3);
		out.println("0");
	}
	
	void parseAnds () throws IOException {
		int lhs = 2 * (I + 1);
		out.println ("p cnf " + (M + 1) + " " + (3 * A + 2));
		for (int i = 0; i < A; i++) {
			int tmp = decode ();
			assert (tmp < lhs);
			int rhs0 = lhs - tmp;
			tmp = decode ();
			assert (tmp <= rhs0);
			int rhs1 = rhs0 - tmp;
			if (debug)
				System.err.println ("JaigerToCNF.parseAnd: " +
				                    lhs + " " + 
						    rhs0 + " " +
						    rhs1);
			printBinaryClause (-lhs, rhs0);
			printBinaryClause (-lhs, rhs1);
			printTernaryClause (lhs, -rhs0, -rhs1);
			lhs += 2;
		}
	}
	
	void printUnaryClause (int lit) {
		printLiteral (lit);
		out.println("0");
	}
	
	void printRest () {
		printUnaryClause (single_output_literal);
		printUnaryClause (2 * (M + 1) + 1);
	}
	
	public static void main(String[] args) throws IOException {
		JaigerToCNF reader = new JaigerToCNF (System.in, System.out);
		reader.parseHeader();
		reader.parseOutput ();
		reader.parseAnds ();
		reader.printRest ();
	}
}
