#pragma once

#include "CommandSyntax.h"

namespace Logic
{
   namespace Scripts
   {
      class SyntaxFile;

      

      /// <summary></summary>
      class LogicExport SyntaxFile
      {
      public:
         /// <summary>Command syntax collection organised by ID</summary>
         class CommandCollection : public multimap<UINT, CommandSyntax>
         {
         public:
            void  Add(CommandSyntaxRef s)
            { 
               insert(value_type(s.ID, s)); 
            }
         };

         /// <summary>Maps names of parameter types to their ID</summary>
         class TypeCollection : public map<wstring,ParameterType>
         {
         public:
            bool  Add(const wstring& name, ParameterType type) 
            { 
               return insert(value_type(name, type)).second; 
            }
         }; 

         /// <summary>Maps names of command groups to their ID</summary>
         class GroupCollection : public map<wstring,CommandGroup>
         {
         public:
            bool  Add(const wstring& name, CommandGroup group) 
            { 
               return insert(value_type(name, group)).second; 
            }
         }; 

         // --------------------- CONSTRUCTION ----------------------
      public:
         SyntaxFile(const wstring& title, const wstring& version);
         virtual ~SyntaxFile();

         // --------------------- PROPERTIES ------------------------
			
		   // ---------------------- ACCESSORS ------------------------
			
		   // ------------------------ STATIC -------------------------

		   // ----------------------- MUTATORS ------------------------

		   // -------------------- REPRESENTATION ---------------------
      public:
         CommandCollection  Commands;
         GroupCollection    Groups;
         TypeCollection     Types;

         wstring            Title, 
                            Version;
      private:
      };

   }
}

using namespace Logic::Scripts;
