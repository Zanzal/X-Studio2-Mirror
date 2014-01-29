#include "stdafx.h"
#include "ScriptParameter.h"
#include "ScriptFile.h"
#include "StringLibrary.h"
#include "GameObjectLibrary.h"
#include "ScriptObjectLibrary.h"

namespace Logic
{
   namespace Scripts
   {
      // -------------------------------- CONSTRUCTION --------------------------------

      ScriptParameter::ScriptParameter(ParameterSyntax s, const ScriptToken& t) : Syntax(s), Value(0), Token(t)
      {}
      ScriptParameter::ScriptParameter(ParameterSyntax s, Conditional c) : Syntax(s), Value(0) 
      {}
   
      ScriptParameter::ScriptParameter(ParameterSyntax s, DataType t, ParameterValue val) : Syntax(s), Type(t), Value(val) 
      { 
         //Translate(); 
      }

      ScriptParameter::ScriptParameter(ParameterSyntax s, DataType t, const wstring& val) : Syntax(s), Type(t), Value(val) 
      { 
         //Translate(); 
      }

      ScriptParameter::ScriptParameter(ParameterSyntax s, DataType t, UINT val) : Syntax(s), Type(t), Value(val) 
      { 
         //Translate(); 
      }

      ScriptParameter::~ScriptParameter()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      /// <summary>Get datatype string</summary>
      GuiString  GetString(DataType d)
      {
         return StringLib.Contains(KnownPage::DATA_TYPES, (UINT)d) ? StringLib.Find(KnownPage::DATA_TYPES, (UINT)d).Text : GuiString(L"Missing string %d", d);
      }

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Translates the specified f.</summary>
      /// <param name="f">The f.</param>
      void   ScriptParameter::Translate(ScriptFile& f)
      {
         const WCHAR* format;

         switch (Type)
         {
         case DataType::VARIABLE:
            format = (Syntax.IsRetVar() ? L"$%s =" : L"$%s");

            // Commented: name
            if (Value.Type == ValueType::String)
               Text = StringResource::Format(format, Value.String.c_str());

            // Default: conditional/variable
            else switch (ReturnValue(Value.Int).ReturnType)
            {
            case ReturnType::ASSIGNMENT:     Text = StringResource::Format(format, f.Variables[Value.Int].Name.c_str());   break;
            case ReturnType::DISCARD:        Text = L"";   break;
            case ReturnType::JUMP_IF_TRUE:
            case ReturnType::JUMP_IF_FALSE:  Text = StringLib.Find(KnownPage::CONDITIONALS, (UINT)ReturnValue(Value.Int).Conditional).Text;  break;

            default:  throw InvalidValueException(HERE, GuiString(L"Unrecognised return type 0x%x", Value.Int));
            }
            break;

         case DataType::STRING:
            if (Syntax.Type == ParameterType::COMMENT || Syntax.Type == ParameterType::LABEL_NAME)
               Text = Value.String;
            else
               Text = StringResource::Format(L"'%s'", Value.String.c_str());
            break;

         case DataType::INTEGER:
            Text = StringResource::Format(L"%d", Value.Int);
            break;

         case DataType::Null:
            Text = L"null";
            break;

         // Various: Strip HIWORD, then lookup ID
         case DataType::OPERATOR:
         case DataType::CONSTANT:
         case DataType::DATATYPE:
            Text = ScriptObjectLib.Find(ScriptObject::IdentifyGroup(Type), Value.LowWord).DisplayText;   
            break;

         // Various: Lookup ID
         case DataType::FLIGHTRETURN:     
         case DataType::OBJECTCLASS:      
         case DataType::OBJECTCOMMAND:    
         case DataType::RACE:             
         case DataType::SCRIPTDEF:        
         case DataType::SECTOR:   
         case DataType::STATIONSERIAL:    
         case DataType::TRANSPORTCLASS:   
         case DataType::WINGCOMMAND:      
            Text = ScriptObjectLib.Find(ScriptObject::IdentifyGroup(Type), Value.Int).DisplayText;   
            break;

         // Relation: Map ID then convert
         case DataType::RELATION:
            Text = ScriptObjectLib.Find(ScriptObject::IdentifyGroup(Type), RelationIDConverter::ToStringID((Relation)Value.Int)).DisplayText;   
            break;

         // Ware
         case DataType::WARE:
            Text = GameObjectLib.Find(Value.Int).DisplayText;
            break;
         }

         // RefObj: Append indirection operator
         if (Syntax.IsRefObj())
            Text += L" ->";
      }

		// ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

   }
}

