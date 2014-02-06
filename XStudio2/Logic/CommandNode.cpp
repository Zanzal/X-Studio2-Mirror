#include "stdafx.h"
#include "CommandNode.h"
#include "GameObjectLibrary.h"
#include "ScriptObjectLibrary.h"
#include "ExpressionParser.h"
#include "SyntaxLibrary.h"
#include "ScriptFile.h"

#undef min
#undef max

namespace Logic
{
   namespace Scripts
   {
      namespace Compiler
      {
         /// <summary>Checks whether commands are standard</summary>
         CommandNode::NodeDelegate  CommandNode::isStandardCommand = [](const CommandNodePtr& n) 
         { 
            return n->Is(CommandType::Standard); 
         };

         /// <summary>Checks whether commands are compatible with 'skip-if' conditional</summary>
         CommandNode::NodeDelegate  CommandNode::isSkipIfCompatible = [](const CommandNodePtr& n) 
         { 
            return n->Is(CommandType::Standard) || n->Is(CMD_BREAK) || n->Is(CMD_CONTINUE); 
         };
         

         // -------------------------------- CONSTRUCTION --------------------------------

         /// <summary>Create root node</summary>
         CommandNode::CommandNode()
            : Syntax(CommandSyntax::Unknown), 
              Condition(Conditional::NONE),
              Parent(nullptr), 
              JumpTarget(nullptr), 
              Index(0), 
              LineNumber(0),
              Extent({0,0})
         {}

         /// <summary>Create node for a hidden jump command</summary>
         /// <param name="parent">parent node</param>
         /// <param name="target">target node</param>
         CommandNode::CommandNode(CommandNode* parent, const CommandNode* target)
            : Syntax(SyntaxLib.Find(CMD_HIDDEN_JUMP, GameVersion::Threat)),
              Condition(Conditional::NONE),
              JumpTarget(target),
              Parent(parent),
              LineNumber(0),
              Extent({0,0}),
              Index(0)
         {
            REQUIRED(parent);
            REQUIRED(target);

            // Add empty jump parameter
            Parameters += ScriptParameter(ParameterSyntax::LabelNumberParameter, DataType::INTEGER, 0xffff);
         }

         /// <summary>Create node for a script command</summary>
         /// <param name="cnd">conditional.</param>
         /// <param name="syntax">command syntax.</param>
         /// <param name="params">parameters.</param>
         /// <param name="lex">lexer.</param>
         /// <param name="line">1-based line number</param>
         CommandNode::CommandNode(Conditional cnd, CommandSyntaxRef syntax, ParameterArray& params, const CommandLexer& lex, UINT line)
            : Syntax(syntax),
              Condition(cnd),
              Parameters(move(params)),
              LineNumber(line), 
              Extent(lex.Extent), 
              LineText(lex.Input),
              Parent(nullptr), 
              JumpTarget(nullptr), 
              Index(0)
         {}

         /// <summary>Create node for an expression</summary>
         /// <param name="cnd">conditional.</param>
         /// <param name="syntax">command syntax.</param>
         /// <param name="params">infix parameters and retVar</param>
         /// <param name="params">postfix parameters.</param>
         /// <param name="lex">lexer.</param>
         /// <param name="line">1-based line number</param>
         CommandNode::CommandNode(Conditional cnd, CommandSyntaxRef syntax, ParameterArray& infix, ParameterArray& postfix, const CommandLexer& lex, UINT line)
            : Syntax(syntax),
              Condition(cnd),
              Parameters(move(infix)),
              Postfix(move(postfix)),
              LineNumber(line), 
              Extent(lex.Extent), 
              LineText(lex.Input),
              Parent(nullptr), 
              JumpTarget(nullptr), 
              Index(0)
         {}

         CommandNode::~CommandNode()
         {}

         // ------------------------------- STATIC METHODS -------------------------------

         // ------------------------------- PUBLIC METHODS -------------------------------
         
         /// <summary>Add child node</summary>
         /// <param name="cmd">The command node</param>
         /// <returns>Command node</returns>
         CommandNodePtr  CommandNode::Add(CommandNodePtr node)
         {
            // Set parent and append
            node->Parent = this;
            Children.push_back(node);
            return node;
         }

         /// <summary>Compiles the script.</summary>
         /// <param name="script">The script.</param>
         void  CommandNode::Compile(ScriptFile& script)
         {
            // Perform linking
            LinkCommands();

            // Index commands
            UINT index = 0;
            IndexCommands(index);
            
            // Compile commands
            GenerateCommands(script);
         }
         
         /// <summary>Identifies branch logic</summary>
         BranchLogic  CommandNode::GetBranchLogic() const
         {
            // Command
            switch (Syntax.ID)
            {
            case CMD_END:      return BranchLogic::End;
            case CMD_ELSE:     return BranchLogic::Else;
            case CMD_BREAK:    return BranchLogic::Break;
            case CMD_CONTINUE: return BranchLogic::Continue;

            case CMD_COMMAND_COMMENT:
            case CMD_COMMENT: 
            case CMD_NOP:     
               return BranchLogic::NOP;
         
            // Conditional
            default:
               switch (Condition)
               {
               case Conditional::IF:      
               case Conditional::IF_NOT:        
                  return BranchLogic::If;

               case Conditional::WHILE:         
               case Conditional::WHILE_NOT:     
                  return BranchLogic::While;

               case Conditional::ELSE_IF:  
               case Conditional::ELSE_IF_NOT:   
                  return BranchLogic::ElseIf;

               case Conditional::SKIP_IF:  
               case Conditional::SKIP_IF_NOT:   
                  return BranchLogic::SkipIf;

               default:
                  return BranchLogic::None;
               }
            }
         }
         
         /// <summary>Debug print</summary>
         /// <param name="depth">The depth.</param>
         void  CommandNode::Print(int depth) const
         {
            // Line#/Logic/Text
            GuiString line(Is(CommandType::Standard) ? L"%03d: " : L"   : ", Index),      // Line#: <index> 
                      logic(GetString(Logic)),
                      txt(LineText);
            Colour    colour(Colour::White);
            
            // Logic
            switch (Logic)
            {
            // Conditional:
            default: 
               colour = Colour::Cyan;
               if (JumpTarget)
                  txt = GuiString(L"Jump-if-false: %d", JumpTarget->Index);
               if (JumpTarget && !JumpTarget->Index)
                  txt += L" " + JumpTarget->LineText;
               break;

            // NOP:
            case BranchLogic::NOP:
               colour = Colour::Yellow;
               break;

            // Command:
            case BranchLogic::None:
               if (Is(CMD_HIDDEN_JUMP) || Is(CMD_GOTO_LABEL) || Is(CMD_GOTO_SUB))
               {
                  colour = Colour::Green; //Is(CMD_HIDDEN_JUMP) ? Colour::Green : Colour::Yellow;
                  logic = Is(CMD_HIDDEN_JUMP) ? L"JMP" : L"GOTO";
                  txt = GuiString(Is(CMD_HIDDEN_JUMP) ? L"Unconditional Jump: %d" : L"Goto label %d", JumpTarget?JumpTarget->Index:0xffff);
               }
               else if (Is(CMD_DEFINE_LABEL))
               {
                  colour = Colour::Red;
                  logic = L"PROC";
               }
               else 
                  logic = (Parent ? L"Cmd" : L"Root");
               break;
            }

            // Print
            Console << line+wstring(depth, L' ') << colour << logic << Colour::White << L" : " << colour << txt.TrimLeft(L" ") << ENDL;

            // Print Children
            for (auto c : Children)
               c->Print(depth+1);
         }

         /// <summary>Verifies the entire tree</summary>
         /// <param name="script">script</param>
         /// <param name="errors">errors collection</param>
         void  CommandNode::Verify(ScriptFile& script, ErrorArray& errors) 
         {
            // Identify labels/variables
            IdentifyVariables(script);

            // parameters
            VerifyParameters(script, errors);

            // branching logic
            VerifyLogic(errors);

            // Ensure script has commands
            if (count_if(Children.begin(), Children.end(), isStandardCommand) == 0)
               errors += ErrorToken(L"No executable commands found", LineNumber, Extent);
            
            // Ensure last std command is RETURN
            else if (find_if(Children.rbegin(), Children.rend(), isStandardCommand) == Children.rend())
            {
               auto last = Children.end()[-1];
               if (!last->Is(CMD_RETURN))
                  errors += ErrorToken(L"Last command in script must be 'return'", last->LineNumber, last->Extent);
            }

            /*else for (auto node = Children.rbegin(); node != Children.rend(); ++node)
            {
               if (node[0]->Is(CommandType::Auxiliary))
                  continue;
               else if (!node[0]->Is(CMD_RETURN))
                  errors += ErrorToken(L"Last command in script must be 'return'", node[0]->LineNumber, node[0]->Extent);
               break;
            }*/
         }

         // ------------------------------ PROTECTED METHODS -----------------------------
         
         // ------------------------------- PRIVATE METHODS ------------------------------

         /// <summary>Check children for presence of certain branch logic</summary>
         /// <param name="l">logic</param>
         /// <returns></returns>
         /*bool  CommandNode::Contains(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [=](const CommandNodePtr& t){ return t->Logic == l; }) != Children.end();
         }*/
         
         /// <summary>Finds an ancestor with a given branch logic</summary>
         /// <returns>Parent if found, otherwise nullptr</returns>
         CommandNode*  CommandNode::FindAncestor(BranchLogic l) const
         {
            // Check for a parent 'while' command
            for (CommandNode* n = Parent; n != nullptr; n = n->Parent)
               if (n->Logic == l)
                  return n;

            // Not found
            return nullptr;
         }

         /// <summary>Finds the first child with certain branch logic</summary>
         /// <param name="l">desired logic</param>
         /// <returns>position if found, otherwise end</returns>
         /*CommandNode::NodeIterator  CommandNode::FindChild(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [l](const CommandNodePtr& n) {return n->Logic == l;} );
         }*/

         /// <summary>Find a child node by value</summary>
         /// <param name="child">desired child</param>
         /// <returns></returns>
         CommandNode::NodeIterator CommandNode::FindChild(const CommandNode* child) const
         {
            return find_if(Children.begin(), Children.end(), [child](const CommandNodePtr& n) {return child == n.get();} );
         }

         /// <summary>Find label definition</summary>
         /// <param name="name">Label name</param>
         /// <returns></returns>
         CommandNode*  CommandNode::FindLabel(const wstring& name) const
         {
            // Check node
            if (Is(CMD_DEFINE_LABEL) && Parameters[0].Value.String == name)
               return const_cast<CommandNode*>(this);
            
            // Check children
            for (const auto& c : Children)
               if (CommandNode* label = c->FindLabel(name))
                  return label;

            // Should never reach here
            return nullptr;
         }
         
         /// <summary>Finds the standard command immediately following this one in the execution order</summary>
         /// <returns></returns>
         CommandNode* CommandNode::FindNextCommand() const
         {
            // Find next sibling node containing a standard command
            for (auto node = Parent->FindChild(this)+1; node < Parent->Children.end(); ++node)
               if ((*node)->Is(CommandType::Standard))
                  return node->get();
            
            // No more siblings: continue search from grandparent
            return Parent->FindNextCommand();
         }

         /// <summary>Finds the standard command immediately following this one in the execution order</summary>
         /// <returns></returns>
         CommandNode* CommandNode::FindNextConditional() const
         {
            //auto node = find_if(Parent->FindChild(this)+1, Parent->Children.end(), isAlternateConditional);
            // Find next sibling node containing a standard command
            for (auto node = Parent->FindChild(this)+1; node < Parent->Children.end(); ++node)
               if ((*node)->Is(CommandType::Standard) || (*node)->Is(CMD_ELSE))
                  return node->get();
            
            // Error
            throw AlgorithmException(HERE, L"Conditional has no alternatives");
         }

         /// <summary>Finds the next sibling of this node</summary>
         /// <returns></returns>
         CommandNode* CommandNode::FindNextSibling() const
         {
            auto node = Parent->FindChild(this);
            return node+1 != Parent->Children.end() ? node[1].get() : nullptr;
         }

         /// <summary>Finds the prev sibling of this node</summary>
         /// <returns></returns>
         CommandNode* CommandNode::FindPrevSibling() const
         {
            auto node = Parent->FindChild(this);
            return node != Parent->Children.begin() ? node[-1].get() : nullptr;
         }

         /// <summary>Finds the root node</summary>
         /// <returns>Root</returns>
         CommandNode*  CommandNode::FindRoot() const
         {
            CommandNode* n;
            for (n = const_cast<CommandNode*>(this); n->Parent != nullptr; n = n->Parent)
            {}
            return n;
         }
         
         /// <summary>Compiles the parameters/commands into the script</summary>
         /// <param name="script">The script.</param>
         void  CommandNode::GenerateCommands(ScriptFile& script)
         {
            ParameterArray params;

            if (!IsRoot())
            {
               // Compile parameters
               for (auto& p : Parameters)
                  p.Generate(script, JumpTarget ? JumpTarget->Index : 0xffff);

               // Generate & insert command
               if (!Syntax.Is(CMD_EXPRESSION))
                  script.Commands.AddOutput(ScriptCommand(LineText, Syntax, Parameters));
               else
               {
                  // Compile postfix parameters
                  for (auto& p : Postfix)
                     p.Generate(script, JumpTarget ? JumpTarget->Index : 0xffff);

                  // Generate & insert command
                  script.Commands.AddOutput(ScriptCommand(LineText, Syntax, Parameters, Postfix));
               }
            }

            // Recurse into children
            for (auto& c : Children)
               c->GenerateCommands(script);
         }

         /// <summary>Maps each variable name to a unique ID, and locates all label definitions</summary>
         /// <param name="script">script.</param>
         void  CommandNode::IdentifyVariables(ScriptFile& script) 
         {
            // Add label definitions to script
            if (Is(CMD_DEFINE_LABEL) && Parameters.size() > 0) 
               script.Labels.Add(Parameters[0].Value.String, LineNumber);

            // Add variable names to script
            for (const auto& p : Parameters)
               if (p.Type == DataType::VARIABLE && p.Value.Type == ValueType::String)
                  script.Variables.Add(p.Value.String);

            // Examine children
            for (const auto& cmd : Children)
               cmd->IdentifyVariables(script);
         }
         
         /// <summary>Calculates the standard command index</summary>
         /// <param name="next">Next index to use</param>
         void CommandNode::IndexCommands(UINT& next)
         {
            // Standard
            if (Is(CommandType::Standard))
               Index = next++;

            // Recurse into children
            for (auto c : Children)
               c->IndexCommands(next);
         }

         /// <summary>Inserts an unconditional jump command as the last child</summary>
         /// <param name="target">Command to target</param>
         /// <returns></returns>
         void  CommandNode::InsertJump(NodeIterator pos, const CommandNode* target)
         {
            Children.insert(pos, new CommandNode(this, target));
         }
         
         /// <summary>Query command syntax ID</summary>
         bool  CommandNode::Is(UINT ID) const
         {
            return Syntax.Is(ID);
         }

         /// <summary>Query command syntax type</summary>
         bool  CommandNode::Is(CommandType t) const
         {
            return Syntax.Is(t);
         }

         /// <summary>Query whether node is rood</summary>
         bool  CommandNode::IsRoot() const
         {
            return Parent == nullptr;
         }
         
         /// <summary>Perform command linking</summary>
         void  CommandNode::LinkCommands() 
         {
            CommandNode* n;

            switch (Logic)
            {
            // JIF: ELSE-IF/ELSE/END
            case BranchLogic::If: 
            case BranchLogic::ElseIf: 
               // JIF: else-if/else/next-std-sibling
               JumpTarget = FindNextConditional();
               
               // preceeds ELSE-IF/ELSE: Append child JMP-> next-std-sibling
               if ((n=FindNextSibling()) && (n->Logic == BranchLogic::Else || n->Logic == BranchLogic::ElseIf))
                  InsertJump(Children.end(), FindConditionalEnd());
               break;

            // <nothing>
            case BranchLogic::Else:
               break;    

            // JIF: next-std-sibling
            case BranchLogic::SkipIf:
               JumpTarget = FindNextCommand();
               break;

            // JIF: next-std-sibling
            case BranchLogic::While:
               JumpTarget = FindNextCommand();
               InsertJump(Children.end(), this); // JMP: WHILE (to create loop)
               break;

            // JMP: WHILE->next-std-sibling
            case BranchLogic::Break:
               JumpTarget = FindAncestor(BranchLogic::While)->FindNextCommand();
               InsertJump(Children.begin(), JumpTarget);
               break;

            // JMP: WHILE
            case BranchLogic::Continue:
               JumpTarget = FindAncestor(BranchLogic::While);
               InsertJump(Children.begin(), JumpTarget);
               break;

            // JMP: LABEL
            case BranchLogic::None:
               if (Is(CMD_GOTO_LABEL) || Is(CMD_GOTO_SUB))
                  JumpTarget = FindRoot()->FindLabel(Parameters[0].Value.String);
               break;
            }

            // Recurse into children
            for (auto& c : Children)
               c->LinkCommands();
         }

         /// <summary>Verifies the branching logic</summary>
         void  CommandNode::VerifyLogic(ErrorArray& errors) const
         {
            CommandNode* n;
            switch (Logic)
            {
            // IF: Must preceed ELSE-IF/ELSE/END
            case BranchLogic::If:
               // EOF?
               if ((n=FindNextSibling()) == nullptr)
                  errors += ErrorToken(L"missing 'end' command", LineNumber, Extent);
               // preceeds End/Else/Else-if?
               else if (n->Logic != BranchLogic::End && n->Logic != BranchLogic::Else && n->Logic != BranchLogic::ElseIf)
                  errors += ErrorToken(L"expected 'else', 'else if' or 'end'", n->LineNumber, n->Extent);
               break;

            // WHILE: Must preceed END
            case BranchLogic::While:
               // EOF?
               if ((n=FindNextSibling()) == nullptr)
                  errors += ErrorToken(L"missing 'end' command", LineNumber, Extent);
               // preceed END?
               else if (n->Logic != BranchLogic::End)
                  errors += ErrorToken(L"expected 'end'", n->LineNumber, n->Extent);
               break;

            // ELSE: Must follow IF/ELSE-IF.  Must preceed END
            case BranchLogic::Else:
               // follow IF/ELSE-IF?
               if ((n=FindPrevSibling()) == nullptr || (n->Logic != BranchLogic::If && n->Logic != BranchLogic::ElseIf))
                  errors += ErrorToken(L"unexpected 'else'", LineNumber, Extent);

               // EOF?
               else if ((n=FindNextSibling()) == nullptr)
                  errors += ErrorToken(L"missing 'end' command", LineNumber, Extent);
               // preceed END?
               else if (n->Logic != BranchLogic::End)
                  errors += ErrorToken(L"expected 'end'", n->LineNumber, n->Extent);
               break;

            // ELSE-IF: Must follow IF/ELSE-IF. Must preceed ELSE-IF/ELSE/END
            case BranchLogic::ElseIf:
               // follow IF/ELSE-IF?
               if ((n=FindPrevSibling()) == nullptr || (n->Logic != BranchLogic::If && n->Logic != BranchLogic::ElseIf))
                  errors += ErrorToken(L"unexpected 'else-if'", LineNumber, Extent);
               
               // EOF?
               else if ((n=FindNextSibling()) == nullptr)
                  errors += ErrorToken(L"missing 'end' command", LineNumber, Extent);
               // preceed ELSE-IF/ELSE/END?
               else if (n->Logic != BranchLogic::Else && n->Logic != BranchLogic::ElseIf && n->Logic != BranchLogic::End)
                  errors += ErrorToken(L"expected 'else', 'else if' or 'end'", n->LineNumber, n->Extent);
               break;

            // END: Must follow IF/WHILE/ELSE-IF/ELSE
            case BranchLogic::End:
               // follow IF/WHILE/ELSE-IF/ELSE?
               if ((n=FindPrevSibling()) == nullptr || (n->Logic != BranchLogic::If && n->Logic != BranchLogic::While && n->Logic != BranchLogic::ElseIf && n->Logic != BranchLogic::Else))
                  errors += ErrorToken(L"unexpected 'end' command", LineNumber, Extent);
               break;

            // SKIP-IF: Must not preceed SKIP-IF. Must contain 1 standard command
            case BranchLogic::SkipIf:
               // not preceed SKIP-IF?
               if ((n=FindPrevSibling()) != nullptr && n->Logic == BranchLogic::SkipIf)
                  errors += ErrorToken(L"'skip-if' cannot be nested", LineNumber, Extent);

               // Ensure command present
               //if (!cmd->Is(CommandType::Standard) && !cmd->Is(CMD_CONTINUE) && !cmd->Is(CMD_BREAK))
               if (count_if(Children.begin(), Children.end(), isSkipIfCompatible) != 1)
                  errors += ErrorToken(L"must contain single command without conditional", LineNumber, Extent);
               break;

            // BREAK/CONTINUE: Must be decendant of WHILE
            case BranchLogic::Break:
            case BranchLogic::Continue:
               // Check for a parent 'while' command
               if (!FindAncestor(BranchLogic::While))
                  errors += ErrorToken(L"break/continue cannot appear outside 'while'", LineNumber, Extent);
               break;
            }

            // Recurse into children
            for (auto& c : Children)
               c->VerifyLogic(errors);
         }
         
         /// <summary>Converts parameter tokens into ordered list of script parameters</summary>
         /// <param name="script">script</param>
         /// <param name="errors">errors collection</param>
         void  CommandNode::VerifyParameters(const ScriptFile& script, ErrorArray& errors) const
         {
            // Skip for unrecognised commands
            if (Syntax != CommandSyntax::Unknown)
               // Static type check
               for (const ScriptParameter& param : Parameters)
               {
                  // Recognise game/script objects
                  switch (param.Token.Type)
                  {
                  // GameObject: Ensure exists
                  case TokenType::GameObject:
                     if (!GameObjectLib.Contains(param.Value.String))
                        errors += ErrorToken(L"Unrecognised game object", LineNumber, param.Token);
                     break;

                  // ScriptObject: Ensure exists 
                  case TokenType::ScriptObject:
                     if (!ScriptObjectLib.Contains(param.Value.String))
                        errors += ErrorToken(L"Unrecognised script object", LineNumber, param.Token);
                     break;

                  // Label: Ensure exists
                  case TokenType::Label:
                     if (!script.Labels.Contains(param.Value.String))
                        errors += ErrorToken(L"Unrecognised label", LineNumber, param.Token);
                     break;
                  }
               
                  // Static type check
                  if (!param.Syntax.Verify(param.Type))
                     errors += ErrorToken(GuiString(L"'%s' is not a valid %s", param.Text.c_str(), GetString(param.Syntax.Type).c_str())
                                                                             , LineNumber, param.Token);
               }

            // Recurse into children
            for (auto& c : Children)
               c->VerifyParameters(script, errors);
         }

      }
   }
}

