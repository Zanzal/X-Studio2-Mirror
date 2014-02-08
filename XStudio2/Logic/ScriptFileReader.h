#pragma once
#include "Common.h"
#include "XmlReader.h"
#include "ScriptFile.h"

namespace Logic
{
   namespace IO
   {
      
      /// <summary>Reader for MSCI scripts</summary>
      class ScriptFileReader : protected XmlReader
      {
      protected:
         /// <summary>Base class for MSCI script command readers</summary>
         class ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         protected:
            ScriptCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : Reader(r), Script(s), Command(cmd), NodeIndex(0) {}
         
            // ----------------------- MUTATORS ------------------------

         public:
            virtual ScriptCommand  ReadCommand() PURE;

         protected:
            wstring         ReadStringNode(const wchar* help);
            int             ReadIntNode(const wchar* help);
            ParameterValue  ReadValueNode(const wchar* help);
            DataType        ReadTypeNode(const wchar* help);

            ScriptParameter ReadParameter(ParameterSyntax s, const wchar* help);

            // -------------------- REPRESENTATION ---------------------
         protected:
            ScriptFileReader&  Reader;
            ScriptFile&        Script;
            XmlNodePtr&        Command;

         private:
            int   NodeIndex;
         };

         /// <summary>Reads typical auxiliary commands</summary>
         class AuxiliaryCommandReader : public ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            AuxiliaryCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : ScriptCommandReader(r, s, cmd) {}

            // ----------------------- MUTATORS ------------------------

            ScriptCommand  ReadCommand();
         };

         /// <summary>Reads command comments</summary>
         class CommentedCommandReader : public ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            CommentedCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : ScriptCommandReader(r, s, cmd) {}

            // ----------------------- MUTATORS ------------------------

            ScriptCommand  ReadCommand();
         };

         /// <summary>Reads variable argument script calls</summary>
         class ScriptCallCommandReader : public ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            ScriptCallCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : ScriptCommandReader(r, s, cmd) {}

            // ----------------------- MUTATORS ------------------------

            ScriptCommand  ReadCommand();
         };

         /// <summary>Reads expression commands</summary>
         class ExpressionCommandReader : public ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            ExpressionCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : ScriptCommandReader(r, s, cmd) {}

            // ----------------------- MUTATORS ------------------------

            ScriptCommand  ReadCommand();
         };

         /// <summary>Reads typical standard commands</summary>
         class StandardCommandReader : public ScriptCommandReader
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            StandardCommandReader(ScriptFileReader& r, ScriptFile& s, XmlNodePtr& cmd) : ScriptCommandReader(r, s, cmd) {}

            // ----------------------- MUTATORS ------------------------

            ScriptCommand  ReadCommand();
         };

      protected:
         /// <summary>Unique command reader pointer</summary>
         typedef unique_ptr<ScriptCommandReader>  ReaderPtr;

         // --------------------- CONSTRUCTION ----------------------

      public:
         ScriptFileReader(StreamPtr in);
         virtual ~ScriptFileReader();

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
			
		   // ---------------------- ACCESSORS ------------------------
			
		   // ----------------------- MUTATORS ------------------------

      public:
         ScriptFile     ReadFile(Path path, bool justProperties);

      protected:
         XmlNodePtr     GetChild(XmlNodePtr& parent, UINT index, const WCHAR* help);
         ReaderPtr      GetCommandReader(ScriptFile& script, CommandType type, XmlNodePtr& cmdBranch);

         int            ReadArray(XmlNodePtr& parent, UINT index, const WCHAR* help);
         ScriptFile     ReadExternalScript(const wstring& name);
         int            ReadInt(XmlNodePtr& parent, UINT index, const WCHAR* help);
         wstring        ReadString(XmlNodePtr& parent, UINT index, const WCHAR* help);
         ParameterValue ReadValue(XmlNodePtr& parent, UINT index, const WCHAR* help);

         void  ReadCommands(ScriptFile&  script, XmlNodePtr& stdBranch, XmlNodePtr& auxBranch);
         void  ReadVariables(ScriptFile&  script, XmlNodePtr& varBranch, XmlNodePtr& argBranch);

		   // -------------------- REPRESENTATION ---------------------
      protected:
         Path  Folder;
      };

      
   }
}

using namespace Logic::IO;
