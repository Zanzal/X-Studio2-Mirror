#pragma once

#include "Common.h"
#include "ParameterSyntax.h"

namespace Library
{
   namespace Scripts
   {
      // DATA_TYPE_FLAGS -- Modifiers added to various types or values
      //
      #define   DTF_ENCODED           0x20000
      #define   DTF_OBJECT_DATA       0x10000
      #define   DTF_UNARY_OPERATOR    0x10000

      //
      // DATA_TYPE -- Defines the type element of a X3 script {type,value} pair
      //
      enum class DataType : UINT
      {    
         DT_NULL            = 0,   DT_UNKNOWN        = 1,   DT_VARIABLE      = 2,
         DT_CONSTANT        = 3,   DT_INTEGER        = 4,   DT_STRING        = 5,
         DT_SHIP            = 6,   DT_STATION        = 7,   DT_SECTOR        = 8,
         DT_WARE            = 9,   DT_RACE           = 10,  DT_STATIONSERIAL = 11,
         DT_OBJECTCLASS     = 12,  DT_TRANSPORTCLASS = 13,  DT_RELATION      = 14,
         DT_OPERATOR        = 15,  DT_EXPRESSION     = 16,  DT_OBJECT        = 17,
         DT_OBJECTCOMMAND   = 18,  DT_FLIGHTRETURN   = 19,  DT_DATATYPE      = 20,
         DT_ARRAY           = 21,  DT_QUEST          = 22,
         // New in Terran Conflict
         DT_WING            = 23,  DT_SCRIPTDEF      = 24,  DT_WINGCOMMAND   = 25,
         DT_PASSENGER       = 26,
         // Live Types
         DT_LIVE_CONSTANT  = DT_CONSTANT | DTF_ENCODED,
         DT_LIVE_VARIABLE  = DT_VARIABLE | DTF_ENCODED,
         DT_LIVE_OBJECT    = DT_OBJECT   | DTF_OBJECT_DATA,
         DT_LIVE_SECTOR    = DT_SECTOR   | DTF_OBJECT_DATA,
         DT_LIVE_SHIP      = DT_SHIP     | DTF_OBJECT_DATA,
         DT_LIVE_STATION   = DT_STATION  | DTF_OBJECT_DATA,
         DT_LIVE_WING      = DT_WING     | DTF_OBJECT_DATA,
         DT_UNARY_OPERATOR = DT_OPERATOR | DTF_UNARY_OPERATOR    
      };

      enum class ValueType { String, Int };

      class ParameterValue
      {
      public:
         ParameterValue(wstring sz) {};
         ParameterValue(int val) {};

         ValueType Type;
         wstring   StringVal;
         int       IntVal;
      };


      /// <summary></summary>
      class ScriptParameter
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         ScriptParameter(ParameterSyntax s, DataType t, ParameterValue val) : Syntax(s), Type(t), Value(val) {}
         ScriptParameter(ParameterSyntax s, DataType t, const wstring& val) : Syntax(s), Type(t), Value(val) {}
         ScriptParameter(ParameterSyntax s, DataType t, UINT val) : Syntax(s), Type(t), Value(val) {}
         virtual ~ScriptParameter() {}

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
			
		   // ---------------------- ACCESSORS ------------------------			

		   // ----------------------- MUTATORS ------------------------

		   // -------------------- REPRESENTATION ---------------------

         DataType       Type;
         ParameterValue Value;
         ParameterSyntax Syntax;
         wstring        Text;
      private:
      };


      typedef vector<ScriptParameter> ParameterArray;
   }
}

using namespace Library::Scripts;