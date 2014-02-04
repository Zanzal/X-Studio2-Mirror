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
         CommandNode::CommandNode(CommandNode* parent, CommandNode* target)
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
            if (Parent != nullptr)
            {
               // Perform linking
               LinkCommands();

               // Compile parameters
               CompileParameters(script);
            }

            // Recurse into children
            for (auto c : Children)
               c->Compile(script);
         }

         /// <summary>Debug print</summary>
         /// <param name="depth">The depth.</param>
         void  CommandNode::Print(int depth) const
         {
            // Line/Indent
            wstring   tab(depth, (WCHAR)L' ');
            GuiString line(L"%03d: %s", LineNumber, tab.c_str()),
                      logic(GetString(Logic)),
                      txt; //(LineText);
            Colour    colour(Colour::White);
            
            // Logic
            switch (Logic)
            {
            // Conditional:
            default: 
               colour = Colour::White;
               if (JumpTarget)
                  txt = GuiString(L"Jump-if-false: %d", JumpTarget->LineNumber);
               break;

            // NOP:
            case BranchLogic::NOP:
               colour = Colour::Yellow;
               break;

            // Command:
            case BranchLogic::None:
               colour = Colour::Yellow;
               if (Syntax.Is(CMD_HIDDEN_JUMP))
               {
                  colour = Colour::Green;
                  logic = L"JMP";
                  txt = GuiString(L"Unconditional Jump: %d", JumpTarget->LineNumber);
               }
               else
               {
                  colour = Colour::Cyan;
                  logic = L"Cmd";
               }
               break;
            }

            // Print
            Console << line << colour << logic << Colour::White << L" : " << colour << txt << ENDL;

            // Print Children
            for (auto c : Children)
               c->Print(depth+1);
         }

         /// <summary>Populates the script with label and variable names</summary>
         /// <param name="script">script.</param>
         void  CommandNode::Populate(ScriptFile& script) 
         {
            // Skip root
            if (Parent != nullptr)
            {
               // Add label definitions to script
               if (Syntax.Is(CMD_DEFINE_LABEL) && Parameters.size() > 0 && Parameters[0].Syntax.Type == ParameterType::LABEL_NAME)
                  script.Labels.Add(Parameters[0].Value.String, LineNumber);

               // Add variable names to script
               for (const auto& p : Parameters)
                  if (p.Type == DataType::VARIABLE && p.Value.Type == ValueType::String)
                     script.Variables.Add(p.Value.String);
            }

            // Examine children
            for (const auto& cmd : Children)
               cmd->Populate(script);
         }
         
         /// <summary>Verifies the entire tree</summary>
         /// <param name="script">script</param>
         /// <param name="errors">errors collection</param>
         void  CommandNode::Verify(const ScriptFile& script, ErrorArray& errors) const 
         {
            // NODE: Verify commands
            if (Parent != nullptr)
            {
               // Verify parameters
               VerifyParameters(script, errors);

               // verify branching logic
               VerifyLogic(errors);
            }

            // NODE/ROOT: Verify children
            for (const auto& cmd : Children)
               cmd->Verify(script, errors);

            // ROOT: Verify unique
            if (Parent == nullptr)
            {
               // Ensure script has commands
               if (Children.size() == 0)
                  errors += ErrorToken(L"No commands found", LineNumber, Extent);
            
               // Ensure last std command is RETURN
               else for (auto node = Children.rbegin(); node != Children.rend(); ++node)
               {
                  if (node[0]->Syntax.Is(CommandType::Auxiliary))
                     continue;
                  else if (!node[0]->Syntax.Is(CMD_RETURN))
                     errors += ErrorToken(L"Last command in script must be 'return'", node[0]->LineNumber, node[0]->Extent);
                  break;
               }
            }
         }

         // ------------------------------ PROTECTED METHODS -----------------------------
         
         // ------------------------------- PRIVATE METHODS ------------------------------

         /// <summary>Compiles the parameters.</summary>
         /// <param name="script">The script.</param>
         void  CommandNode::CompileParameters(ScriptFile& script)
         {
         }

         /// <summary>Check children for presence of certain branch logic</summary>
         /// <param name="l">logic</param>
         /// <returns></returns>
         bool  CommandNode::Contains(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [=](const CommandNodePtr& t){ return t->Logic == l; }) != Children.end();
         }
         
         /// <summary>Finds the first child with certain branch logic</summary>
         /// <param name="l">desired logic</param>
         /// <returns>position if found, otherwise end</returns>
         CommandNode::NodeIterator  CommandNode::Find(BranchLogic l) const
         {
            return find_if(Children.begin(), Children.end(), [l](const CommandNodePtr& n) {return n->Logic == l;} );
         }

         /// <summary>Find a child node by value</summary>
         /// <param name="child">desired child</param>
         /// <returns></returns>
         CommandNode::NodeIterator CommandNode::Find(const CommandNode* child) const
         {
            return find_if(Children.begin(), Children.end(), [child](const CommandNodePtr& n) {return child == n.get();} );
         }

         /// <summary>Finds the first standard command following this node</summary>
         /// <returns></returns>
         CommandNode* CommandNode::FindNextSibling() const
         {
            // Find next sibling node containing a standard command
            for (auto node = Parent->Find(this)+1; node < Parent->Children.end(); ++node)
               if ((*node)->Syntax.Is(CommandType::Standard))
                  return node->get();
            
            // No more siblings: continue search from grandparent
            return Parent->FindNextSibling();
         }

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

         /// <summary>Inserts an unconditional jump command as the last child</summary>
         /// <param name="target">Command to target</param>
         /// <returns></returns>
         void  CommandNode::InsertJump(NodeIterator pos, CommandNode* target)
         {
            Console << "DEBUG: ChildCount=" << Children.size() << ENDL;
            Children.insert(pos, new CommandNode(this, target));
         }

         /// <summary>Perform command linking</summary>
         void  CommandNode::LinkCommands() 
         {
            NodeIterator ElseIf;

            switch (Logic)
            {
            // Jump-if-false: ELSE-IF/ELSE/next-sibling. 
            case BranchLogic::If: 
               ElseIf = std::min(Find(BranchLogic::ElseIf), Find(BranchLogic::Else));
               // if has else/else-if
               if (ElseIf != Children.end())
               {  
                  JumpTarget = ElseIf->get();     // JIF: Else/ElseIf 
                  InsertJump(ElseIf, FindNextSibling());  // JMP: next-sibling
               }
               else
                  JumpTarget = FindNextSibling();  // JMP: next-sibling
               break;

            // Jump-if-false: ELSE-IF/ELSE/next-sibling. 
            case BranchLogic::ElseIf:
               ElseIf = Parent->Find(this)+1;
               // if not final else/else-if
               if (ElseIf != Parent->Children.end() && ((*ElseIf)->Logic == BranchLogic::Else || (*ElseIf)->Logic == BranchLogic::ElseIf))
               {  
                  JumpTarget = ElseIf->get();             // JIF: Else/ElseIf
                  InsertJump(Children.end(), Parent->FindNextSibling());  // JMP: next-sibling(IF)
               }
               else
                  JumpTarget = Parent->FindNextSibling();  // JMP: next-sibling(IF)
               break;
            
            // Jump-if-false: next-sibling(IF)
            case BranchLogic::Else:
               JumpTarget = Parent->FindNextSibling();
               break;               

            // Jump-if-false: next-sibling
            case BranchLogic::SkipIf:
               JumpTarget = FindNextSibling();
               break;

            // Jump-if-false: next-sibling. 
            case BranchLogic::While:
               JumpTarget = FindNextSibling();
               InsertJump(Children.end(), this); // JMP: SELF (to create loop)
               break;

            // JMP: next-sibling(WHILE)
            case BranchLogic::Break:
               JumpTarget = FindAncestor(BranchLogic::While)->FindNextSibling();
               break;

            // JMP: WHILE
            case BranchLogic::Continue:
               JumpTarget = FindAncestor(BranchLogic::While);
               break;
            }
         }

         /// <summary>Verifies the branching logic</summary>
         void  CommandNode::VerifyLogic(ErrorArray& errors) const
         {
            // Check for END
            switch (Logic)
            {
            case BranchLogic::If:
            case BranchLogic::While:
               if (Logic == BranchLogic::If)
               {
                  auto Else = Find(BranchLogic::Else);
                  auto ElseIf = Find(BranchLogic::ElseIf);

                  // Ensure 'else-if' does not follow 'else'
                  if (Else != Children.end() && ElseIf != Children.end() && Else < ElseIf)
                     errors += ErrorToken(L"'else-if' must come before 'else' command", (*ElseIf)->LineNumber, (*ElseIf)->Extent);
               }

               // Ensure 'end' is present
               if (Find(BranchLogic::End) == Children.end())
                  errors += ErrorToken(L"missing 'end' command", LineNumber, Extent);
               break;

            case BranchLogic::End:
               // Check for parent 'if'/'while' command
               if (Parent->Logic != BranchLogic::While && Parent->Logic != BranchLogic::If)
                  errors += ErrorToken(L"unexpected 'end' command", LineNumber, Extent);
               break;

            case BranchLogic::Break:
            case BranchLogic::Continue:
               // Check for a parent 'while' command
               if (!FindAncestor(BranchLogic::While))
                  errors += ErrorToken(L"break/continue outside 'while' conditional", LineNumber, Extent);
               break;

            case BranchLogic::Else:
            case BranchLogic::ElseIf:
               // Ensure within 'if' command
               if (Parent->Logic != BranchLogic::If)
                  errors += ErrorToken(L"else/else-if outside 'if' conditional", LineNumber, Extent);
               break;

            case BranchLogic::SkipIf:
               // Ensure command present
               if (Children.size() == 0)
                  errors += ErrorToken(L"missing command from 'skip if' conditional", Children[0]->LineNumber, Children[0]->Extent);

               // Ensure command is standard
               else 
               {
                  auto cmd = Children.end()[-1];
                  if (!cmd->Syntax.Is(CommandType::Standard) && !cmd->Syntax.Is(CMD_CONTINUE) && !cmd->Syntax.Is(CMD_BREAK))
                     errors += ErrorToken(L"incompatible with 'skip if' conditional", cmd->LineNumber, cmd->Extent);
               }
               break;
            }
         }
         
         /// <summary>Converts parameter tokens into ordered list of script parameters</summary>
         /// <param name="script">script</param>
         /// <param name="errors">errors collection</param>
         void  CommandNode::VerifyParameters(const ScriptFile& script, ErrorArray& errors) const
         {
            // Skip for unrecognised commands
            if (Syntax == CommandSyntax::Unknown)
               return;

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
         }

      }
   }
}

