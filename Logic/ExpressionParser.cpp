#include "stdafx.h"
#include "ExpressionParser.h"

//#define  PRINT_DEBUG

namespace Logic
{
   namespace Scripts
   {
      namespace Compiler
      {
         // -------------------------------- CONSTRUCTION --------------------------------

         /// <summary>Creates a script expression parser</summary>
         /// <param name="begin">Position of first expression token</param>
         /// <param name="end">Position after last expression token</param>
         /// <exception cref="Logic::AlgorithmException">Error in parsing algorithm</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error in expression</exception>
         ExpressionParser::ExpressionParser(TokenIterator& begin, const TokenIterator end)
            : InputBegin(begin), InputEnd(end)
         {
            Parse(begin);
         }


         ExpressionParser::~ExpressionParser()
         {
         }

         // ------------------------------- STATIC METHODS -------------------------------

         // ------------------------------- PUBLIC METHODS -------------------------------
         
         // ------------------------------ PROTECTED METHODS -----------------------------
         
         /// <summary>Parses the expression, ensures it is correct and produces infix/postfix tokens.</summary>
         /// <exception cref="Logic::AlgorithmException">Error in parsing algorithm</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error in expression</exception>
         void  ExpressionParser::Parse(TokenIterator& start)
         {
            TokenIterator pos = InputBegin;

            // Produce parse tree
            ExpressionTree tree = ReadExpression(pos);

            // Ensure all tokens parsed
            if (pos != InputEnd)
               throw ExpressionParserException(HERE, pos, L"Unexpected token");

            // Extract tokens
            tree->getTokenArray(Traversal::InOrder, InfixParams);
            tree->getTokenArray(Traversal::PostOrder, PostfixParams);

#ifdef PRINT_DEBUG
            Console << L"Output: " << tree->debugPrint() << ENDL;
            //Console << L"Infix: " << tree->debugPrintTraversal(Traversal::InOrder) << ENDL;
            //Console << L"Postfix: " << tree->debugPrintTraversal(Traversal::PostOrder) << ENDL;
#endif
         }

         // ------------------------------- PRIVATE METHODS ------------------------------

         /// <summary>Attempts to matches any literal</summary>
         /// <param name="pos">Position of literal</param>
         /// <returns></returns>
         bool  ExpressionParser::MatchLiteral(const TokenIterator& pos)
         {
            // Validate position 
            if (pos < InputEnd) 
               // check type
               switch (pos->Type)
               {
               case TokenType::Variable:
               case TokenType::Number:
               case TokenType::Null:
               case TokenType::String:
               case TokenType::GameObject:
               case TokenType::ScriptObject:
                  return true;
               }

            return false;
         }

         /// <summary>Attempts to matches a specific operator</summary>
         /// <param name="pos">Position of operator</param>
         /// <returns></returns>
         bool ExpressionParser::MatchOperator(const TokenIterator& pos, const WCHAR* op)
         {
            // Validate position and compare operator
            return pos < InputEnd && (pos->Type == TokenType::BinaryOp || pos->Type == TokenType::UnaryOp) && pos->Text == op;
         }

         /// <summary>Attempts to matches any operator of a given precedence</summary>
         /// <param name="pos">Position of operator</param>
         /// <returns></returns>
         /// <exception cref="Logic::AlgorithmException">Invalid precendence detected</exception>
         bool ExpressionParser::MatchOperator(const TokenIterator& pos, UINT precedence)
         {
            // Validate position. Ensure operator
            if (pos >= InputEnd || (pos->Type != TokenType::BinaryOp && pos->Type != TokenType::UnaryOp))
               return false;

            // Precedence table taken from X2 scripting manual
            switch (precedence)
            {
            case 0: return pos->Text == L"OR" || pos->Text == L"||";
            case 1: return pos->Text == L"AND" || pos->Text == L"&&";
            case 2: return pos->Text == L"|";
            case 3: return pos->Text == L"^";
            case 4: return pos->Text == L"&";
            case 5: return pos->Text == L"==" || pos->Text == L"!=";
            case 6: return pos->Text == L"<" || pos->Text == L">" || pos->Text == L"<=" || pos->Text == L">=";
            case 7: return pos->Text == L"+" || pos->Text == L"-"; 
            case 8: return pos->Text == L"*" || pos->Text == L"/" || pos->Text == L"%" || pos->Text == L"MOD";
            case 9: return pos->Text == L"~" || pos->Text == L"!" || pos->Text == L"-"; 
            }

            throw AlgorithmException(HERE, VString(L"Invalid precedence %d", precedence));
         }

         /// <summary>Reads the current token as a literal</summary>
         /// <param name="pos">Current position</param>
         /// <returns>Token</returns>
         /// <exception cref="Logic::AlgorithmException">Token is not a literal</exception>
         /// <remarks>Advances the iterator to beyond the literal</remarks>
         const ScriptToken&  ExpressionParser::ReadLiteral(TokenIterator& pos)
         {
            // Validation position
            if (pos < InputEnd)
               switch (pos->Type)
               {
               // Check type
               case TokenType::Variable:
               case TokenType::Number:
               case TokenType::Null:
               case TokenType::String:
               case TokenType::GameObject:
               case TokenType::ScriptObject:
                  // Read literal. Advance position
                  return *(pos++);
               }
          
            // Error:
            throw AlgorithmException(HERE, VString(L"'%s' is not a literal", pos < InputEnd ? pos->Text : L"<eof>"));
         }

         /// <summary>Reads the current token as an operator</summary>
         /// <param name="pos">Current position</param>
         /// <returns>Token</returns>
         /// <exception cref="Logic::AlgorithmException">Token is not a operator</exception>
         /// <remarks>Advances the iterator to beyond the operator</remarks>
         const ScriptToken&  ExpressionParser::ReadOperator(TokenIterator& pos)
         {
            // Verify operator
            if (pos >= InputEnd || (pos->Type != TokenType::BinaryOp && pos->Type != TokenType::UnaryOp))
               throw AlgorithmException(HERE, L"Not an operator");

            // Read operator. Advance position
            return *(pos++);
         }


         /// <summary>Reads an entire expression</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::AlgorithmException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ExpressionParser::ExpressionTree  ExpressionParser::ReadExpression(TokenIterator& pos)
         {
            // Rule: Expression = Comparison
            return ReadBinaryExpression(pos, MIN_PRECEDENCE);
         }

         /// <summary>Reads a binary expression, unary expression, sub-expression, or literal</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::AlgorithmException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ExpressionParser::ExpressionTree  ExpressionParser::ReadBinaryExpression(TokenIterator& pos, UINT precedence)
         {
            ExpressionTree expr = nullptr;

            // Rule: Expr        = LogicalExpr
            // Rule: LogicalExpr = BitwiseExpr (AND/OR BitwiseExpr)*
            // Rule: BitwiseExpr = Comparison (&/|/^ Comparison)*
            // Rule: Comparison  = SumExpr (==/!=/<=/>=/</> SumExpr)*
            // Rule: SumExpr     = ProductExpr (+/- ProductExpr)*
            // Rule: ProductExpr = UnaryExpr (*/div UnaryExpr)*
            // Rule: UnaryExpr   = (!/-/~)? Value
            // Rule: Value       = literal / '(' expr ')'

            // Read: BinaryExpr := higher-precedence-expr (input-precedence-operator  higher-precedence-expr)*
            expr = (precedence < MAX_PRECEDENCE ? ReadBinaryExpression(pos, precedence+1) : ReadUnaryExpression(pos));    // throws

            // Match: operator  
            while (MatchOperator(pos, precedence))
            {  
               // Read: operator
               auto op = ReadOperator(pos);                                                        

               // Read: higher-precedence-expr / Value
               auto rhs = (precedence < MAX_PRECEDENCE ? ReadBinaryExpression(pos, precedence+1) : ReadUnaryExpression(pos));  // throws
               expr = ExpressionTree(new BinaryExpression(op, expr, rhs));     // throws
            }

            // Success:
            return expr;
         }


         /// <summary>Reads a unary expression, sub-expression, or literal</summary>
         /// <param name="pos">Position of first token of expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::AlgorithmException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the expression</remarks>
         ExpressionParser::ExpressionTree  ExpressionParser::ReadUnaryExpression(TokenIterator& pos)
         {
            // Rule: Unary = (! / - / ~)? Value

            // Match: Operator  
            if (MatchOperator(pos, MAX_PRECEDENCE)) 
            {  
               // Read: operator  (may throw)
               const ScriptToken& op = ReadOperator(pos);
               
               // Manually convert binary-substract to unary-minus
               ScriptToken unary = (op.Text != L"-" ? op : ScriptToken(TokenType::UnaryOp, op.Start, op.End, op.Text));

               // Read: Value  (may throw)
               return ExpressionTree(new UnaryExpression(unary, ReadValue(pos)));
            }

            // Read: Value  (may throw)
            return ReadValue(pos);
         }

         /// <summary>Reads a literal or sub-expression</summary>
         /// <param name="pos">Position of literal or first token of sub-expression</param>
         /// <returns>Expression tree</returns>
         /// <exception cref="Logic::AlgorithmException">Attempted to read incorrect type of Token</exception>
         /// <exception cref="Logic::ExpressionParserException">Syntax error</exception>
         /// <remarks>Advances the iterator to beyond the end of the literal or sub-expression</remarks>
         ExpressionParser::ExpressionTree  ExpressionParser::ReadValue(TokenIterator& pos)
         {
            ExpressionTree expr = nullptr;

            // Rule: Value = Literal / '(' Expression ')'

            // Match: Literal  
            if (MatchLiteral(pos)) 
               return ExpressionTree( new LiteralValue(ReadLiteral(pos)) );

            // Read: Bracket   [nothrow]
            if (!MatchOperator(pos, L"(")) 
            {
               // Failed: Unexpected EOF
               if (pos >= InputEnd)
                  throw ExpressionParserException(HERE, --pos, L"Missing operand");

               // Failed: Unexpected token
               throw ExpressionParserException(HERE, pos, VString(L"Unexpected '%s'", pos->Text.c_str()));
            }
            
            // Read: Expression  (may throw)
            const ScriptToken& open = ReadOperator(pos);
            expr = ReadExpression(pos);   // Adv. then match

            // Read: Bracket   [nothrow]
            if (MatchOperator(pos, L")")) 
               return ExpressionTree( new BracketedExpression(open, expr, ReadOperator(pos)) );
            
            // Failure: Missing closing bracket
            if (pos >= InputEnd)
               throw ExpressionParserException(HERE, --pos, L"Missing closing bracket");
            else
               throw ExpressionParserException(HERE, pos, L"Expected closing bracket");
         }

      }
   }
}
