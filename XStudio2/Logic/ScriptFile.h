#pragma once
#include "Common.h"
#include "CommandSyntax.h"
#include "ParameterSyntax.h"
#include "ScriptCommand.h"
#include <algorithm>

namespace Logic
{
   namespace Scripts
   {
      /// <summary>Distinguishes variables from arguments</summary>
      enum class VariableType : UINT { Argument, Variable };

      /// <summary>An argument or variable used within a script</summary>
      class ScriptVariable
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         ScriptVariable(VariableType t, const wstring& name, UINT index) 
            : Type(t), Name(name), ID(index), ValueType(ParameterType::UNDETERMINED)
         {}

         // -------------------- REPRESENTATION ---------------------

         wstring        Name,
                        Description;
         ParameterType  ValueType;
         VariableType   Type;
         UINT           ID;       // 1-based index/ID
      };

      /// <summary>Vector of ScriptVariables</summary>
      typedef vector<ScriptVariable>  VariableArray;


      /// <summary></summary>
      class ScriptLabel
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         ScriptLabel(wstring name, UINT line) : Name(name), LineNumber(line)
         {}

         // -------------------- REPRESENTATION ---------------------

         wstring  Name;
         UINT     LineNumber;    // 1-based line number
      };

      /// <summary>Vector of ScriptLabels</summary>
      typedef vector<ScriptLabel>  LabelArray;


      /// <summary></summary>
      class ScriptFile
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         ScriptFile();
         virtual ~ScriptFile();

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
			
         PROPERTY_GET(wstring,CommandName,GetCommandName);

		   // ---------------------- ACCESSORS ------------------------
			
         /// <summary>Gets the name of the object command.</summary>
         /// <returns>Name of command if any, otherwise empty string</returns>
         /// <exception cref="Logic::ScriptObjectNotFoundException">Command matching ID is not present</exception>
         wstring GetCommandName() const;

		   // ----------------------- MUTATORS ------------------------

         /// <summary>Clears commands, labels and non-argument variables</summary>
         void  Clear()
         {
            // Clear labels & commands
            Commands.clear();
            Labels.clear();

            // Clear variables, keep arguments
            auto var = find_if(Variables.begin(), Variables.end(), [](const ScriptVariable& v) { return v.Type == VariableType::Variable; });
            Variables.erase(var, Variables.end());
         }

         /// <summary>Finds the index of the label that represents the scope of a line number</summary>
         /// <param name="line">The 1-based line number</param>
         /// <returns>Index into the labels array, or -1 for global scope</returns>
         int  FindScope(UINT line)
         {
            // Determine current scope
            auto scope = find_if(Labels.rbegin(), Labels.rend(), [line](ScriptLabel& l) {return line >= l.LineNumber;} );

            // Convert to index
            return distance(scope, Labels.rend()) - 1;
         }

		   // -------------------- REPRESENTATION ---------------------

         wstring        Name,
                        Description;
         UINT           Version;
         GameVersion    Game;
         bool           LiveData;
         ParameterValue CommandID;

         CommandList    Commands;
         LabelArray     Labels;
         VariableArray  Variables;
      private:
      };

   }
}

using namespace Logic::Scripts;
