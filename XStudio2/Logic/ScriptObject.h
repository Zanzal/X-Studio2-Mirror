#pragma once

#include "Common.h"
#include "LanguagePage.h"
#include "ParameterTypes.h"

namespace Logic
{
   namespace Scripts
   {
      /// <summary>Defines script object group</summary>
      enum class ScriptObjectGroup
      {
         Constant, DataType, FlightReturn, ObjectClass, Race, Relation, ParameterType, 
         Sector, StationSerial, ObjectCommand, WingCommand, Signal, TransportClass, Operator
      };

      /// <summary>Get script object group name</summary>
      GuiString GetString(const ScriptObjectGroup& g);

      /// <summary>Post-Increment script object group</summary>
      ScriptObjectGroup operator++(ScriptObjectGroup& g, int);

      /// <summary>Pre-Increment script object group</summary>
      ScriptObjectGroup& operator++(ScriptObjectGroup& g);

      /// <summary>Substract script object group as int</summary>
      UINT operator-(const ScriptObjectGroup& a, const ScriptObjectGroup& b);
      
      
      /// <summary>Occurs when a MSCI script object is unrecognised</summary>
      class ScriptObjectNotFoundException : public ExceptionBase
      {
      public:
         /// <summary>Create a ScriptObjectNotFoundException</summary>
         /// <param name="src">Location of throw</param>
         /// <param name="txt">Desired object text</param>
         ScriptObjectNotFoundException(wstring  src, wstring txt) 
            : ExceptionBase(src, GuiString(L"Cannot find script object '%s'", txt.c_str()))
         {}

         /// <summary>Create a ScriptObjectNotFoundException</summary>
         /// <param name="src">Location of throw</param>
         /// <param name="group">object group</param>
         /// <param name="id">object id</param>
         ScriptObjectNotFoundException(wstring  src, ScriptObjectGroup group, UINT id) 
            : ExceptionBase(src, GuiString(L"Cannot find %s with id %d", GetString(group).c_str(), id))
         {}
      };

      class ScriptObject;

      /// <summary>Constant script object reference</summary>
      typedef const ScriptObject&  ScriptObjectRef;

      /// <summary>Represents a script object</summary>
      class ScriptObject 
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         ScriptObject(UINT id, KnownPage page, const GuiString& txt, GameVersion ver);
         ScriptObject(ScriptObject&& r);
         ~ScriptObject();

      private:
         ScriptObject(const ScriptObject& obj, const GuiString& newText);
         

      public:
         DEFAULT_COPY(ScriptObject);	// Default copy semantics
		   DEFAULT_MOVE(ScriptObject);	// Default move semantics

         // ------------------------ STATIC -------------------------

         /// <summary>Identifies the script object group</summary>
         static ScriptObjectGroup  IdentifyGroup(KnownPage page, UINT id);
         static ScriptObjectGroup  IdentifyGroup(DataType type);

         // --------------------- PROPERTIES ------------------------

         PROPERTY_GET(GuiString,DisplayText,GetDisplayText);

         // ---------------------- ACCESSORS ------------------------	
      public:
         /// <summary>Get data-type from group</summary>
         DataType  GetDataType() const;

         /// <summary>Get text formatted for display</summary>
         GuiString  GetDisplayText() const;

         /// <summary>Get whether object should be hidden from GUI</summary>
         bool  IsHidden() const;

         /// <summary>Get whether object cannot be resolved by lookup</summary>
         bool  CanLookup() const;

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Appends an object ID</summary>
         /// <param name="id">The id</param>
         /// <returns>script object with new text</returns>
         ScriptObject operator+(UINT id);

         /// <summary>Appends a game version acronym</summary>
         /// <param name="v">The version</param>
         /// <returns>script object with new text</returns>
         ScriptObject operator+(GameVersion v);

         /// <summary>Appends a group acronym</summary>
         /// <param name="g">The group</param>
         /// <returns>script object with new text</returns>
         /// <exception cref="Logic::ArgumentException">Incompatible page</exception>
         ScriptObject operator+(ScriptObjectGroup g);

         // -------------------- REPRESENTATION ---------------------
      public:
         const UINT              ID;
         const ScriptObjectGroup Group;
         GuiString               Text;
         GameVersion             Version;
      };


      /// <summary>Vector of script objects</summary>
      typedef vector<ScriptObject>  ScriptObjectArray;
   
   }
}

using namespace Logic::Scripts;
