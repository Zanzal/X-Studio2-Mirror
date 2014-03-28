#include "stdafx.h"
#include "CommandSyntax.h"
#include "CommandLexer.h"
#include "CommandHash.h"
#include "StringLibrary.h"
#include <algorithm>

namespace Logic
{
   namespace Scripts
   {
      /// <summary>'unknown' command syntax sentinel value</summary>
      const CommandSyntax  CommandSyntax::Unrecognised;

      /// <summary>Determines whether parameter usage is scriptName</summary>
      /// <param name="s">parameter</param>
      const ParamSyntaxPredicate  CommandSyntax::IsScriptNameParam = [](const ParameterSyntax& s) 
      {
         return s.Usage == ParameterUsage::ScriptName;
      };
      
      /// <summary>Determines whether parameter usage is StringID or PageID</summary>
      /// <param name="s">parameter</param>
      const ParamSyntaxPredicate  CommandSyntax::IsStringRefParam = [](const ParameterSyntax& s) 
      {
         return s.Usage == ParameterUsage::StringID || s.Usage == ParameterUsage::PageID;
      };
      
      /// <summary>Determines whether parameter type represents interruptable syntax</summary>
      /// <param name="s">parameter</param>
      const ParamSyntaxPredicate  CommandSyntax::IsInterruptParam = [](const ParameterSyntax& s) 
      {
         return s.Type == ParameterType::INTERRUPT_RETURN_VALUE_IF || s.Type == ParameterType::RETURN_VALUE_IF_START;
      };

      
      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Create the 'unknown' command syntax sentinel value</summary>
      /// <param name="d">Declaration</param>
      CommandSyntax::CommandSyntax() 
         : ID(CMD_UNRECOGNISED), 
           Versions((UINT)GameVersion::Threat|(UINT)GameVersion::Reunion|(UINT)GameVersion::TerranConflict|(UINT)GameVersion::AlbionPrelude),
           Type(CommandType::Standard),
           Group(CommandGroup::HIDDEN),
           Execution(ExecutionType::Serial),
           VArgCount(0),
           VArgument(VArgSyntax::None),
           VArgParams(VArgMethod::None)
      {
      }

      /// <summary>Create command syntax from a declaration</summary>
      /// <param name="d">Declaration</param>
      CommandSyntax::CommandSyntax(const Declaration& d)
         : ID(d.ID),
           URL(d.URL), 
           Type(d.Type),
           Text(d.Syntax), 
           Group(d.Group),  
           Versions(d.Versions), 
           Parameters(d.Params), 
           Execution(d.Execution),
           VArgCount(d.VArgCount),
           VArgument(d.VArgument),
           VArgParams(d.VArgParams),
           Hash(GenerateHash(d.Syntax))
      {
      }

      CommandSyntax::~CommandSyntax()
      {
      }

      // -------------------------------- STATIC METHODS -------------------------------

      /// <summary>Generates hash from syntax, used when sorting syntax for display</summary>
      /// <param name="syntax">The syntax.</param>
      /// <returns></returns>
      wstring CommandSyntax::GenerateHash(const wstring& syntax)
      {
         CommandLexer lex(syntax, true);
         return CommandHash(lex.begin(), lex.end()).Hash;
      }

      /// <summary>Get command group name</summary>
      LogicExport GuiString  GetString(CommandGroup g)
      {
         return GuiString(IDS_FIRST_COMMAND_GROUP + (UINT)g);
      }

      /// <summary>Get command type name</summary>
      LogicExport GuiString  GetString(CommandType t)
      {
         switch (t)
         {
         case CommandType::Auxiliary:  return L"Auxiliary";
         case CommandType::Macro:      return L"Macro";
         case CommandType::Standard:   return L"Standard";
         }
         return L"Unrecognised";
      }

      /// <summary>Get execution type name</summary>
      LogicExport GuiString  GetString(ExecutionType t)
      {
         switch (t)
         {
         case ExecutionType::Concurrent:  return L"Concurrent";
         case ExecutionType::Either:      return L"Either";
         case ExecutionType::Serial:      return L"Serial";
         }
         return L"Unrecognised";
      }

      /// <summary>Get varg syntax name</summary>
      LogicExport GuiString  GetString(VArgSyntax s)
      {
         switch (s)
         {
         case VArgSyntax::CommaDelimited: return L"CommaDelimited";
         case VArgSyntax::NamedPairs:     return L"NamedPairs";
         case VArgSyntax::None:           return L"None";
         }
         return L"Unrecognised";
      }

      /// <summary>Get varg method name</summary>
      LogicExport GuiString  GetString(VArgMethod m)
      {
         switch (m)
         {
         case VArgMethod::Drop:     return L"Drop";
         case VArgMethod::None:     return L"None";
         case VArgMethod::PadNull:  return L"PadNull";
         }
         return L"Unrecognised";
      }

      /// <summary>Post-Increment command group</summary>
      LogicExport CommandGroup operator++(CommandGroup& g, int)
      {
         CommandGroup ret = g;
         return (++g, ret);
      }

      /// <summary>Pre-Increment command group</summary>
      LogicExport CommandGroup& operator++(CommandGroup& g)
      {
         return g = (CommandGroup)((UINT)g + 1);
      }

      /// <summary>Operate on command groups as integers</summary>
      LogicExport UINT operator-(const CommandGroup& a, const CommandGroup& b)
      {
         return (UINT)a - (UINT)b;
      }

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Get populated display text</summary>
      /// <returns></returns>
      /// <exception cref="Logic::PageNotFoundException">Parameter types Page does not exist</exception>
      /// <exception cref="Logic::StringNotFoundException">Parameter type string does not exist</exception>
      wstring  CommandSyntax::GetDisplayText() const
      {
         CommandLexer lex(Text, false);
         wstring      output(Text);

         // Iterate backwards thru tokens
         for (auto tok = lex.Tokens.rbegin(); tok < lex.Tokens.rend(); ++tok)
         {
            if (tok->Type == TokenType::Variable)
            {
               // Lookup parameter name
               auto param = Parameters[tok->Text[1]-48];
               VString name(L"<%s>", GetString(param.Type).c_str());

               // Replace $n marker with parameter text
               output.replace(tok->Start, tok->Length, name.c_str());
            }
         }

         return output;
      }

      /// <summary>Get populated display richtext</summary>
      wstring  CommandSyntax::GetRichText(bool boldParams) const
      {
         CommandLexer lex(Text, false);
         wstring      output(Text);

         if (!boldParams)
            throw NotImplementedException(HERE, L"Rich title text has not been implemented");

         // Iterate backwards thru tokens
         for (auto tok = lex.Tokens.rbegin(); tok < lex.Tokens.rend(); ++tok)
         {
            if (tok->Type == TokenType::Variable)
            {
               // Lookup parameter name
               auto param = Parameters[tok->Text[1]-48];
               VString name(L"[b]�%s�[/b]", GetString(param.Type).c_str());

               // Replace $n marker with parameter text
               output.replace(tok->Start, tok->Length, name.c_str());
            }
         }

         return L"[center]" + output + L"[/center]";
      }

      /// <summary>Gets the parameter syntax sorted by display index</summary>
      /// <returns></returns>
      ParamSyntaxArray  CommandSyntax::GetParametersByDisplay() const
      {
         ParamSyntaxArray params;

         // Re-order by display index (ascending)
         for (UINT i = 0; i < Parameters.size(); ++i)
         {
            auto p = find_if(Parameters.begin(), Parameters.end(), [i](const ParameterSyntax& ps) {return ps.DisplayIndex == i;});
            params.push_back(*p);
         }

#ifdef DEBUGGING
         // DEBUG: Verify sizes equal
         if (params.size() != Parameters.size())
            throw AlgorithmException(HERE, L"");

         // DEBUG: Verify sorted by display index
         for (UINT i = 0; i < params.size(); i++)
            if (params[i].DisplayIndex != i)
            {
               wstring debug;
               for_each(params.begin(), params.end(), [&debug](ParameterSyntax& ps)
               {
                  debug += VString(L"physical %d : display %d : %s\n", ps.PhysicalIndex, ps.DisplayIndex, GetString(ps.Type).c_str());
               });
               throw AlgorithmException(HERE, VString(L"Parameter %d of %d has display index %d instead of %d : %s\n%s", 
                                                        i+1, params.size(), params[i].DisplayIndex, i, Text.c_str(), debug.c_str()));
            }
#endif
         return params;
      }

      /// <summary>Query command ID</summary>
      bool  CommandSyntax::Is(UINT id) const
      {
         return ID == id;
      }

      /// <summary>Query command type</summary>
      bool  CommandSyntax::Is(CommandType t) const
      {
         return Type == t;
      }

      /// <summary>Determines whether the command is compatible with a game</summary>
      /// <param name="v">The game version</param>
      /// <returns></returns>
      bool  CommandSyntax::IsCompatible(GameVersion v) const
      {
         return (Versions & (UINT)v) != 0;
      }

      /// <summary>Determines whether command is a keyword.</summary>
      /// <returns></returns>
      bool  CommandSyntax::IsKeyword() const
      {
         switch (ID)
         {
         case CMD_END:
         case CMD_ELSE:
         case CMD_BREAK:
         case CMD_END_SUB:
         case CMD_RETURN:
         case CMD_CONTINUE:
         case CMD_GOTO_SUB:
         case CMD_GOTO_LABEL:  return true;
         default:              return false;
         }
      }

      /// <summary>Query whether command yields execution.</summary>
      bool  CommandSyntax::IsInterrupt() const
      {
         // Match any interruptable RetVar parameter syntax
         return any_of(Parameters.begin(), Parameters.end(), IsInterruptParam);
      }
      
      /// <summary>Determines whether command is a script call</summary>
      /// <returns></returns>
      bool  CommandSyntax::IsScriptCall() const
      {
         // Match any 'ScriptName' parameter syntax
         return any_of(Parameters.begin(), Parameters.end(), IsScriptNameParam);
      }      
      
      /// <summary>Determines whether command references a string or page ID</summary>
      /// <returns></returns>
      bool  CommandSyntax::IsStringReference() const
      {
         // Matches any 'StringID'/'PageID' parameter syntax
         return any_of(Parameters.begin(), Parameters.end(), IsStringRefParam);
      }      

      /// <summary>Query whether command has variable arguments</summary>
      bool  CommandSyntax::IsVArgument() const
      {
         return VArgument != VArgSyntax::None;
      }

		// ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

      
   }
}

