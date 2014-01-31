#include "stdafx.h"
#include "ScriptParser.h"
#include "GameObjectLibrary.h"
#include "ScriptObjectLibrary.h"

namespace Logic
{
   namespace Scripts
   {
      namespace Compiler
      {
         // -------------------------------- CONSTRUCTION --------------------------------

         /// <summary>Create root node</summary>
         ScriptParser::CommandNode::CommandNode()
            : Parent(nullptr), JumpTarget(nullptr), Command(ScriptCommand::Unknown), Logic(BranchLogic::None),
              Index(0), LineNumber(0), LineText({0,0})
         {
         }

         /// <summary>Create node from a script command</summary>
         /// <param name="cmd">script command.</param>
         /// <param name="lex">command lexer</param>
         /// <param name="line">1-based line number</param>
         ScriptParser::CommandNode::CommandNode(const ScriptCommand& cmd, const CommandLexer& lex, UINT line)
            : Parent(nullptr), JumpTarget(nullptr), Command(cmd), Logic(cmd.Logic),
              Index(0), LineNumber(line), LineText(lex.Extent), Text(lex.Input)
         {
         }

         ScriptParser::CommandNode::~CommandNode()
         {
         }

         // ------------------------------- STATIC METHODS -------------------------------

         // ------------------------------- PUBLIC METHODS -------------------------------
         
         /// <summary>Add child node</summary>
         /// <param name="cmd">The command node</param>
         /// <returns>Command node</returns>
         ScriptParser::CommandTree  ScriptParser::CommandNode::Add(CommandTree node)
         {
            // Set parent and append
            node->Parent = this;
            Children.push_back(node);
            return node;
         }

         /// <summary>Check children for presence of certain branch logic</summary>
         /// <param name="l">logic</param>
         /// <returns></returns>
         bool  ScriptParser::CommandNode::Contains(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [=](const CommandTree& t){ return t->Logic == l; }) != Children.end();
         }

         /// <summary>Finds the first child with certain branch logic</summary>
         /// <param name="l">desired logic</param>
         /// <returns>position if found, otherwise end</returns>
         ScriptParser::CommandNode::NodeIterator  ScriptParser::CommandNode::Find(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [l](const CommandTree& n) {return n->Logic == l;} );
         }

         /// <summary>Query whether this node is the root</summary>
         bool  ScriptParser::CommandNode::IsRoot() const
         {
            return Parent == nullptr;
         }

         /// <summary>Debug print</summary>
         /// <param name="depth">The depth.</param>
         void  ScriptParser::CommandNode::Print(int depth) const
         {
            wstring tab(depth, (WCHAR)L' ');

            //Console << GuiString(L"%03d: %s%s : %s", LineNumber, tab.c_str(), GetString(Logic), Text.c_str()) << ENDL;
            // Line/Indent
            Console << GuiString(L"%03d: %s", LineNumber, tab.c_str());
            
            // Logic
            switch (Logic)
            {
            case BranchLogic::If:
            case BranchLogic::While:
            case BranchLogic::Else:
            case BranchLogic::ElseIf:
            case BranchLogic::SkipIf:
            case BranchLogic::End:  
               Console << Colour::Yellow; 
               break;
            case BranchLogic::Break:
            case BranchLogic::Continue:
               Console << Colour::Green; 
               break;
            }
            
            // Text
            Console << GetString(Logic) << Colour::White << L" : " << Text << ENDL;
            
            // Children
            for (auto c : Children)
               c->Print(depth+1);
         }

         /// <summary>Verifies the node</summary>
         /// <param name="err">The error collection</param>
         void  ScriptParser::CommandNode::Verify(ErrorArray& err) const
         {
            if (!IsRoot())
            {
               // Recognise game/script objects
               VerifyObjects(err);

               // Verify branching logic
               VerifyLogic(err);
            }

            // Verify children
            for (const auto& cmd : Children)
               cmd->Verify(err);

            // Verify root
            if (IsRoot())
               VerifyScript(err);
         }

         // ------------------------------ PROTECTED METHODS -----------------------------

         /// <summary>Verifies the branching logic</summary>
         /// <param name="err">The error collection</param>
         void  ScriptParser::CommandNode::VerifyLogic(ErrorArray& err) const
         {
            CommandTree cmd;

            // Check for END
            switch (Logic)
            {
            case BranchLogic::If:
            case BranchLogic::While:
               // Ensure 'else-if' does not follow 'else'
               if (Logic == BranchLogic::If)
               {
                  auto Else = Find(BranchLogic::Else);
                  auto ElseIf = Find(BranchLogic::ElseIf);
                  if (Else != Children.end() && ElseIf != Children.end() && Else < ElseIf)
                     err += ErrorToken(L"'else-if' must come before 'else' command", (*ElseIf)->LineNumber, (*ElseIf)->LineText);
               }

               // Ensure 'end' is present
               if (Find(BranchLogic::End) == Children.end())
                  err += ErrorToken(L"missing 'end' command", LineNumber, LineText);
               break;

            case BranchLogic::End:
               // Check for parent 'if'/'while' command
               if (Parent->Logic != BranchLogic::While && Parent->Logic != BranchLogic::If)
                  err += ErrorToken(L"unexpected 'end' command", LineNumber, LineText);
               break;

            case BranchLogic::Break:
            case BranchLogic::Continue:
               // Check for a parent 'while' command
               for (const CommandNode* n = Parent; n != nullptr; n = n->Parent)
                  if (n->Logic == BranchLogic::While)
                     return;
               
               // Missing 
               err += ErrorToken(L"break/continue outside 'while' conditional", LineNumber, LineText);
               break;

            case BranchLogic::Else:
            case BranchLogic::ElseIf:
               // Ensure within 'if' command
               if (Parent->Logic != BranchLogic::If)
                  err += ErrorToken(L"else/else-if outside 'if' conditional", LineNumber, LineText);
               break;

            case BranchLogic::SkipIf:
               // Ensure child is standard command
               if (Children.size() != 0 && (cmd=Children.end()[-1])->Logic != BranchLogic::None)
                  err += ErrorToken(L"not supported within 'skip if' conditional", cmd->LineNumber, cmd->LineText);
               break;
            }
         }
         
         /// <summary>Verifies the game and script objects</summary>
         /// <param name="err">The error collection</param>
         void  ScriptParser::CommandNode::VerifyObjects(ErrorArray& err) const
         {
            // Recognise objects
            for (const auto& p : Command.Parameters)
            {
               // Verify game object
               if (p.Token.Type == TokenType::GameObject && !GameObjectLib.Contains(p.Token.ValueText))
                  err += ErrorToken(L"Unrecognised game object", LineNumber, p.Token);

               // Verify script object
               else if (p.Token.Type == TokenType::ScriptObject && !ScriptObjectLib.Contains(p.Token.ValueText))
                  err += ErrorToken(L"Unrecognised script object", LineNumber, p.Token);
            }
         }
         
         /// <summary>Verifies the root node</summary>
         /// <param name="err">The error collection</param>
         void  ScriptParser::CommandNode::VerifyScript(ErrorArray& err) const
         {
            // Ensure script has commands
            if (Children.size() == 0)
            {
               err += ErrorToken(L"No commands found", LineNumber, LineText);
               return;
            }

            // Ensure last command is RETURN
            for (auto cmd = Children.rbegin(); cmd != Children.rend(); ++cmd)
            {
               if (cmd[0]->Command.Is(CommandType::Auxiliary))
                  continue;
               else if (!cmd[0]->Command.Is(CMD_RETURN))
                  err += ErrorToken(L"Last command in script must be 'return'", cmd[0]->LineNumber, cmd[0]->LineText);
               break;
            }
         }

         // ------------------------------- PRIVATE METHODS ------------------------------
      }
   }
}

