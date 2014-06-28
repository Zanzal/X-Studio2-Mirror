#pragma once

#include "ParameterSyntax.h"
#include "ReturnValue.h"
#include "ScriptToken.h"
#include "ParameterValue.h"
#include "ParameterArray.h"

namespace Logic
{
   namespace Scripts
   {
      class ScriptFile;
         
      /// <summary>Sentinel value for unspecified jump destination</summary>
      const unsigned short EMPTY_JUMP = 0xffff;
      
      /// <summary>Represents a script command parameter</summary>
      class LogicExport ScriptParameter
      {
         // ------------------------- TYPES -------------------------
      protected:
         /// <summary>Adds/Removes apostrophes from strings</summary>
         class StringConverter
         {
            // --------------------- CONSTRUCTION ----------------------
         protected:
            StringConverter();
            virtual ~StringConverter();

            DEFAULT_COPY(StringConverter);	// Default copy semantics
            DEFAULT_MOVE(StringConverter);	// Default move semantics

            // ------------------------ STATIC -------------------------
         public:
            static wstring Generate(const wstring& str);
            static wstring Translate(const wstring& str);
         };

         // --------------------- CONSTRUCTION ----------------------
#ifdef LOGIC_LIB_BUILD_FIX
      public:
         ScriptParameter()  { THROW_LOGIC_LIB_BUILD_FIX; }
#endif
      public:
         ScriptParameter(const ScriptToken& name, const ScriptToken& val);
         ScriptParameter(const ParameterSyntax& s, const ScriptToken& t);
         ScriptParameter(const ParameterSyntax& s, Conditional c);
         ScriptParameter(const ParameterSyntax& s, DataType t, ParameterValue val);
         ScriptParameter(const ParameterSyntax& s, DataType t, const wstring& val);
         ScriptParameter(const ParameterSyntax& s, DataType t, UINT val);
         virtual ~ScriptParameter();

         // ------------------------ STATIC -------------------------
      protected:
         static DataType  IdentifyDataType(ScriptToken tok);

         // --------------------- PROPERTIES ------------------------
      public:
         PROPERTY_GET(GuiString,DisplayText,GetDisplayText);

		   // ---------------------- ACCESSORS ------------------------			
      public: 
         /// <summary>Re-Generate display text from value</summary>
         /// <returns></returns>
         GuiString GetDisplayText() const;

         /// <summary>Determines whether this an uncommented variable</summary>
         /// <returns></returns>
         bool IsVariable() const
         {
            return Type == DataType::VARIABLE 
                && Value.Type == ValueType::Int 
                && ReturnValue(Value.Int).ReturnType == ReturnType::ASSIGNMENT;
         }

         /// <summary>Compare type and value only</summary>
         bool operator==(const ScriptParameter& r) const
         {
            return Type==r.Type && Value==r.Value; // && Syntax==r.Syntax && Text==r.Text && Token==r.Token;
         }

		   // ----------------------- MUTATORS ------------------------
      public:
         void  Generate(ScriptFile& script, UINT jumpDestination, bool commented);
         void  Identify(ScriptFile& script);
         void  Translate(ScriptFile& script);

#ifdef LOGIC_LIB_BUILD_FIX
         ScriptParameter& operator=(const ScriptParameter& r) const  { THROW_LOGIC_LIB_BUILD_FIX; }
#endif
		   // -------------------- REPRESENTATION ---------------------
      public:
         DataType        Type;
         ParameterValue  Value;
         ParameterSyntax Syntax;
         wstring         Text;
         ScriptToken     ArgName,
                         Token;
      };

      /// <summary>Write script parameter to the console</summary>
      LogicExport ConsoleWnd& operator<<(ConsoleWnd& c, const ScriptParameter& p);

   }
}

using namespace Logic::Scripts;
