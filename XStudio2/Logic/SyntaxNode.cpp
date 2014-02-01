#include "stdafx.h"
#include "SyntaxLibrary.h"
#include "CommandLexer.h"

namespace Logic
{
   namespace Scripts
   {

      // -------------------------------- CONSTRUCTION --------------------------------

      SyntaxLibrary::SyntaxNode::SyntaxNode() : Syntax(nullptr)
      {
      }

      SyntaxLibrary::SyntaxNode::SyntaxNode(const ScriptToken& t) : Token(t), Syntax(nullptr)
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      // ------------------------------- PUBLIC METHODS -------------------------------
      
      /// <summary>Clears syntax tree</summary>
      void  SyntaxLibrary::SyntaxNode::Clear()
      {
         Children.clear();
      }

      /// <summary>Finds syntax by text</summary>
      /// <param name="pos">First token</param>
      /// <param name="end">End of tokens</param>
      /// <param name="ver">Desired game version</param>
      /// <param name="params">Parameter tokens</param>
      /// <returns>Requested syntax if found/compatible, otherwise sentinel syntax</returns>
      CommandSyntax SyntaxLibrary::SyntaxNode::Find(TokenIterator& pos, const TokenIterator& end, GameVersion ver, TokenArray& params) const
      {
         // EOF: Return syntax @ this node (if any)
         if (pos >= end)
            return HasSyntax() && Syntax->IsCompatible(ver) ? *Syntax : CommandSyntax::Unknown;

         // VARG SCRIPT-CALL: Terminate early to prevent analysing arguments
         else if (HasSyntax() && Syntax->ID == CMD_CALL_SCRIPT_VAR_ARGS)
            return *Syntax;

         // PARAM: Store token
         if (pos->IsParameter())
            params += *pos;

         // Lookup next token
         auto pair = Children.find( GetKey(*pos) );
               
         // Not found: Return sentinel
         if (pair == Children.end())
            return CommandSyntax::Unknown;

         // Found: Search children
         return pair->second.Find(++pos, end, ver, params);
      };

      /// <summary>Inserts new syntax into the tree</summary>
      /// <param name="s">The syntax</param>
      /// <param name="pos">First token</param>
      /// <param name="end">End of tokens</param>
      /// <exception cref="Logic::GenericException">Syntax conflicts with existing syntax</exception>
      void  SyntaxLibrary::SyntaxNode::Insert(const CommandSyntax& s, TokenIterator& pos, const TokenIterator& end)
      {
         if (pos < end)
         {
            // (Insert/Lookup) next child
            auto pair = Children.insert( NodeMap::value_type(GetKey(*pos), SyntaxNode(*pos)) );

            // Check child against next token
            pair.first->second.Insert(s, ++pos, end);
         }
         else
         {
            // Ensure not duplicate
            if (HasSyntax())
               throw GenericException(HERE, GuiString(L"The command syntax '%s' (id:%d) is already present", Syntax->Text.c_str(), Syntax->ID));

            // EndOfInput: Store syntax here
            Syntax = SyntaxPtr(new CommandSyntax(s));
         }
      }

      /// <summary>Prints node and all children to the console</summary>
      /// <param name="depth">Current depth</param>
      void SyntaxLibrary::SyntaxNode::Print(int depth) const
      {
         // DEBUG: Abort
         if (depth == 3)
            return;

         // Print node
         wstring indent(depth, L' ');
         Console << indent << L"Token: " << Colour::Yellow << Token.Text << Colour::White << L"   Syntax: " << Colour::Yellow << GetSyntax().Text << ENDL;

         // Print children
         for (auto c : Children)
            c.second.Print(depth+1);
      }

      // ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

      /// <summary>Gets the node map lookup key for a token</summary>
      /// <param name="tok">The token</param>
      /// <returns></returns>
      const wstring&  SyntaxLibrary::SyntaxNode::GetKey(const ScriptToken& tok) const
      {
         return tok.IsParameter() ? tok.Text : VARIABLE;
      }

      /// <summary>Get the syntax at this node, if any, otherwise sentinel syntax</summary>
      /// <returns></returns>
      CommandSyntax  SyntaxLibrary::SyntaxNode::GetSyntax() const
      {
         return HasSyntax() ? *Syntax : CommandSyntax::Unknown;
      }

      /// <summary>Determine whether node has syntax</summary>
      /// <returns></returns>
      bool  SyntaxLibrary::SyntaxNode::HasSyntax() const
      {
         return Syntax != nullptr;
      }
   }
}

