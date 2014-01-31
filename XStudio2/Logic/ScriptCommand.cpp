#include "stdafx.h"
#include "ScriptCommand.h"
#include "ScriptFile.h"
#include "CommandLexer.h"
#include "SyntaxLibrary.h"
#include <algorithm>

namespace Logic
{
   namespace Scripts
   {
      /// <summary>Unrecognised script command sentinel value</summary>
      ScriptCommand  ScriptCommand::Unknown;

      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Creates sentinel value (private ctor)</summary>
      ScriptCommand::ScriptCommand()
         : Syntax(CommandSyntax::Unknown), RefIndex(0)
      {
      }

      /// <summary>Create ANY command PARSED FROM COMMAND TEXT</summary>
      /// <param name="text">Entire command text</param>
      /// <param name="syntax">Command syntax</param>
      /// <param name="params">Parameters in DISPLAY ORDER</param>
      ScriptCommand::ScriptCommand(const CommandSyntax& syntax, const wstring& text, ParameterArray& params)
         : Syntax(syntax), RefIndex(0), Text(text), Parameters(params) 
      {
      }

      /// <summary>Create a STANDARD command READ FROM A FILE</summary>
      /// <param name="syntax">command syntax.</param>
      /// <param name="params">Parameters in PHYSICAL ORDER</param>
      ScriptCommand::ScriptCommand(const CommandSyntax& syntax, ParameterArray& params)
         : Syntax(syntax), RefIndex(0), Parameters(params) 
      {
      }

      /// <summary>Create an AUXILIARY command READ FROM A FILE</summary>
      /// <param name="syntax">command syntax.</param>
      /// <param name="ref">Index of associated standard command</param>
      /// <param name="params">Parameters in PHYSICAL ORDER</param>
      ScriptCommand::ScriptCommand(const CommandSyntax& syntax, UINT  ref, ParameterArray& params)
         : Syntax(syntax), RefIndex(ref), Parameters(params) 
      {
      }
      
      ScriptCommand::~ScriptCommand()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      /// <summary>Gets BranchLogic string</summary>
      /// <param name="l">logic</param>
      /// <returns></returns>
      const WCHAR* GetString(BranchLogic l)
      {
         static const WCHAR* str[] = {L"None", L"NOP", L"If", L"While", L"SkipIf", L"Else", L"ElseIf", L"End", L"Break", L"Continue"};
         return str[l];
      }

      // ------------------------------- PUBLIC METHODS -------------------------------
      
      /// <summary>Gets the jump destination of a goto/gosub command</summary>
      /// <returns></returns>
      /// <exception cref="Logic::InvalidOperationException">Command is not goto/gosub -or- label number parameter is missing</exception>
      UINT ScriptCommand::GetJumpDestination() const
      {
         // Validate command ID
         if (Syntax.ID != CMD_GOTO_LABEL && Syntax.ID != CMD_GOTO_SUB)
            throw InvalidOperationException(HERE, GuiString(L"Cannot get jump destination for a '%s' command", Syntax.Text.c_str()));

         // Validate label name parameter
         if (Parameters.size() == 0 || Parameters[0].Value.Type != ValueType::Int)
            throw InvalidOperationException(HERE, GuiString(L"Missing jump destination parameter"));

         return Parameters[0].Value.Int;
      }
      
      /// <summary>Gets the name of the label.</summary>
      /// <returns></returns>
      /// <exception cref="Logic::InvalidOperationException">Command is not label name definition -or- name parameter is missing</exception>
      wstring  ScriptCommand::GetLabelName() const
      {
         // Validate command ID
         if (Syntax.ID != CMD_DEFINE_LABEL)
            throw InvalidOperationException(HERE, GuiString(L"Cannot get label name for a '%s' command", Syntax.Text.c_str()));

         // Validate label name parameter
         if (Parameters.size() == 0 || Parameters[0].Value.Type != ValueType::String)
            throw InvalidOperationException(HERE, GuiString(L"Missing label name parameter"));

         return Parameters[0].Value.String;
      }

      /// <summary>Identify the type of branching conditional used (if any)</summary>
      /// <returns></returns>
      BranchLogic  ScriptCommand::GetLogic() const
      {
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
         
         default:
            // Find Return parameter, if any
            auto it = find_if(Parameters.begin(), Parameters.end(), [](const ScriptParameter& p) { return p.Syntax.IsRetVar(); });
            
            // RetVar/Discard
            if (it == Parameters.end() || it->Value.Type == ValueType::String)
               return BranchLogic::None;

            // RetVar/Discard
            ReturnValue retVar(it->Value.Int);
            if (retVar.ReturnType == ReturnType::ASSIGNMENT || retVar.ReturnType == ReturnType::DISCARD)
               return BranchLogic::None;
               
            // Conditional
            switch (retVar.Conditional)
            {
            case Conditional::IF:      
            case Conditional::IF_NOT:        return BranchLogic::If;
            case Conditional::WHILE:         
            case Conditional::WHILE_NOT:     return BranchLogic::While;

            case Conditional::ELSE_IF:  
            case Conditional::ELSE_IF_NOT:   return BranchLogic::ElseIf;

            case Conditional::SKIP_IF:  
            case Conditional::SKIP_IF_NOT:   return BranchLogic::SkipIf;

            default:
               return BranchLogic::None;
            }
         }
      }
      
      /// <summary>Compare command ID</summary>
      /// <param name="ID">command id.</param>
      /// <returns></returns>
      bool  ScriptCommand::Is(UINT ID) const
      {
         return Syntax.ID == ID;
      }

      /// <summary>Compare command type</summary>
      /// <param name="t">command type.</param>
      /// <returns></returns>
      bool  ScriptCommand::Is(CommandType t) const
      {
         return Syntax.Type == t;
      }

      /// <summary>Replaces a label number parameter with a label name parameter.</summary>
      /// <param name="name">The name of the label</param>
      /// <exception cref="Logic::InvalidOperationException">Command is not goto/gosub</exception>
      void  ScriptCommand::SetLabelName(const wstring& name)
      {
         // Validate command ID
         if (Syntax.ID != CMD_GOTO_LABEL && Syntax.ID != CMD_GOTO_SUB)
            throw InvalidOperationException(HERE, GuiString(L"Cannot set label name for a '%s' command", Syntax.Text.c_str()));

         // Replace parameters
         Parameters.clear();
         Parameters.push_back( ScriptParameter(ParameterSyntax::LabelDeclaration, DataType::STRING, name) );
      }

      /// <summary>Translates the text of a command READ FROM FILE</summary>
      /// <param name="f">Script used for variable names lookup</param>
      void ScriptCommand::Translate(ScriptFile& f)
      {
         CommandLexer lex(Syntax.Text, false);
         bool         Param = false;

         // Translate parameters
         for (ScriptParameter& p : Parameters)
            p.Translate(f);

         // Format command
         for (const ScriptToken& tok : lex.Tokens)
         {
            if (tok.Type == TokenType::Variable || tok.Type == TokenType::Comment)
            {
               int i = (tok.Text[1]-48);
               Text.append( Parameters[i].Text );
            }
            else
               Text.append(tok.Text);
         }

         // Expressions: Print remaining syntax
         if (Syntax.ID == CMD_EXPRESSION)
            for (UINT i = 2; i < Parameters.size(); ++i)
               Text.append(Parameters[i].Text);

         // Script Calls: Print remaining syntax
         else if (Syntax.ID == CMD_CALL_SCRIPT_VAR_ARGS)
            for (UINT i = 3; i < Parameters.size(); ++i)
            {
               Text.append(GuiString(L" argument%d=", i-2));
               Text.append(Parameters[i].Text);
            }
      }

		// ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

   }
}

