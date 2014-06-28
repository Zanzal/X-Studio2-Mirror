#pragma once

#include "CommandSyntax.h"
#include "ScriptParameter.h"
#include "ScriptToken.h"
#include "RtfWriter.h"

namespace Logic
{
   namespace Scripts
   {
      class ScriptFile;

      /// <summary>Branching logic implemented by a script command</summary>
      enum BranchLogic   { None, NOP, If, While, SkipIf, Else, ElseIf, End, Break, Continue };

      /// <summary>Get branch logic name</summary>
      LogicExport const WCHAR* GetString(BranchLogic l);


      /// <summary>Varg script-call parameter {name,value} pair</summary>
      typedef pair<wstring,ScriptParameter> ArgumentPair;

      /// <summary>Script Parameter lamda predicates</summary>
      typedef function<bool (const ScriptParameter&)>  ParameterPredicate;
      typedef function<bool (const ArgumentPair& a)>   ArgumentPairPredicate;


      
      /// <summary>String ID referenced by a script command</summary>
      class LogicExport StringID
      {
      public:
         /// <summary>Create string reference</summary>
         /// <param name="page">page id parameter.</param>
         /// <param name="id">string id parameter.</param>
         StringID(ScriptParameter page, ScriptParameter id) : Page(page), String(id)
         {}

      public:
         ScriptParameter Page, 
                         String;
      };


      /// <summary>An MSCI script command</summary>
      class LogicExport ScriptCommand
      {
         // --------------------- CONSTRUCTION ----------------------
#ifdef LOGIC_LIB_BUILD_FIX
      public:
         ScriptCommand() : Syntax(CommandSyntax::Unrecognised)  { THROW_LOGIC_LIB_BUILD_FIX; }
#endif
         
      private:
         ScriptCommand(void*);
      public:
         ScriptCommand(const wstring& text, CommandSyntaxRef syntax, ParameterArray& params, bool cmdComment);
         ScriptCommand(const wstring& text, CommandSyntaxRef syntax, ParameterArray& infix, ParameterArray& postfix, bool cmdComment);
         ScriptCommand(CommandSyntaxRef syntax, ParameterArray& params, bool comment);
         ScriptCommand(CommandSyntaxRef syntax, UINT  ref, ParameterArray& params, bool comment);
         virtual ~ScriptCommand();

         // ------------------------ STATIC -------------------------
      public:
         static ScriptCommand  Unrecognised;

         static const ParameterPredicate IsScriptNameParam;
         static const ParameterPredicate IsStringIDParam;
         static const ParameterPredicate IsPageIDParam;

      private:
         static const ArgumentPairPredicate IsArgumentNull;
         static const ParameterPredicate    IsParameterNull;

         // --------------------- PROPERTIES ------------------------
			
         PROPERTY_GET(BranchLogic,Logic,GetLogic);

		   // ---------------------- ACCESSORS ------------------------			
      public:
         BranchLogic  GetLogic() const;
         wstring      GetLabelName() const;
         UINT         GetJumpDestination() const;
         wstring      GetScriptCallName() const;
         StringID     GetStringReference() const;

         bool  Is(UINT ID) const;
         bool  Is(CommandType t) const;
         
#ifdef LOGIC_LIB_BUILD_FIX
         operator bool() const  { THROW_LOGIC_LIB_BUILD_FIX; }
         const ScriptCommand& operator==(const ScriptCommand& r) const  { THROW_LOGIC_LIB_BUILD_FIX; }
#endif

      public:
         bool  FindInteger(UINT index, int& val) const;
         bool  FindOperator(UINT index, Operator& op) const;
         bool  FindRetVar(wstring& varName) const;
         bool  FindString(UINT index, wstring& str) const;
         bool  FindVariable(UINT index, wstring& var) const;
         
         bool  MatchInteger(UINT index, int val) const;
         bool  MatchOperator(UINT index, Operator op) const;
         bool  MatchVariable(UINT index, const wstring& var) const;

         bool  MatchDimAllocation(wstring& varName, int& size) const;
         bool  MatchDimAssignment(const wstring& array, int element, const ScriptParameter*& param) const;
         
         bool  MatchForEachAdvance(const wstring& iterator) const;
         bool  MatchForEachAccess(const wstring& iterator, const wstring& array, wstring& item_iterator) const;
         bool  MatchForEachCondition(const wstring& iterator) const;
         bool  MatchForEachInitialize(wstring& iterator, wstring& array) const;

         bool  MatchForLoopAdvance(const wstring& iterator, const int step) const;
         bool  MatchForLoopCondition(const wstring& iterator, const int step, const ScriptParameter*& limit) const;
         bool  MatchForLoopInitialize(wstring& iterator, const ScriptParameter*& initial, int& step) const;

		   // ----------------------- MUTATORS ------------------------
      public:
         void  SetLabelName(const wstring& name);
         void  Translate(ScriptFile& f);

		   // -------------------- REPRESENTATION ---------------------
      public:
         CommandSyntaxRef Syntax;         // Syntax
         bool             Commented;      // Whether command is commented  (false for comments)
         ParameterArray   Parameters;     // Parameters in physical syntax order
         GuiString        Text;           // Translated/Generated command Text
         UINT             RefIndex;       // [Auxiliary] Index of associated standard command
      };

      /// <summary>Vector of script commands</summary>
      typedef vector<ScriptCommand> CommandArray;
   }
}

using namespace Logic::Scripts;
