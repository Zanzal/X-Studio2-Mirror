#pragma once
#include "Common.h"
#include "SyntaxFile.h"
#include "ScriptToken.h"
#include "BackgroundWorker.h"

// Syntax library singleton
#define SyntaxLib SyntaxLibrary::Instance

namespace Logic
{
   namespace Scripts
   {
      /// <summary></summary>
      class SyntaxLibrary
      {
         // ------------------------ TYPES --------------------------
      public:
         /// <summary>User customized Command group collection</summary>
         typedef set<CommandGroup>  GroupCollection;

         /// <summary>Command syntax collection organised by ID</summary>
         class CommandCollection : public multimap<UINT, CommandSyntax>
         {
         public:
            void  Add(CommandSyntax& s) 
            { 
               insert(value_type(s.ID, s)); 
            }
         };
      
      private:
         /// <summary></summary>
         class SyntaxNode
         {
            // ------------------------ TYPES --------------------------
         private:
            /// <summary>Sentinel node map key for a parameter</summary>
            const wstring VARIABLE = L"�����";

            /// <summary>Nodes keyed by token text</summary>
            typedef map<wstring, SyntaxNode> NodeMap;

            /// <summary>Shared pointer to a command syntax</summary>
            typedef shared_ptr<CommandSyntax> SyntaxPtr;
         
            // --------------------- CONSTRUCTION ----------------------
         public:
            SyntaxNode();
            SyntaxNode(const ScriptToken& t);

            // ------------------------ STATIC -------------------------

            // --------------------- PROPERTIES ------------------------
			
            // ---------------------- ACCESSORS ------------------------			

         public:
            CommandSyntax  Find(TokenIterator& pos, const TokenIterator& end, GameVersion ver, TokenArray& params) const;
            void           Print(int depth = 1) const;

         private:
            const wstring& GetKey(const ScriptToken& tok) const;
            CommandSyntax  GetSyntax() const;
            bool           HasSyntax() const;

            // ----------------------- MUTATORS ------------------------

         public:
            void  Clear();
            void  Insert(const CommandSyntax& s, TokenIterator& pos, const TokenIterator& end);

            // -------------------- REPRESENTATION ---------------------

            NodeMap      Children;
            SyntaxPtr    Syntax;
            ScriptToken  Token;
         };

         // --------------------- CONSTRUCTION ----------------------

      private:
         SyntaxLibrary();
      public:
         virtual ~SyntaxLibrary();

         // ------------------------ STATIC -------------------------

      public:
         static SyntaxLibrary  Instance;

         // --------------------- PROPERTIES ------------------------
			
		   // ---------------------- ACCESSORS ------------------------			

      public:
         GroupCollection   GetGroups() const;
         CommandSyntax     Find(UINT id, GameVersion ver) const;
         CommandSyntax     Identify(TokenIterator& pos, const TokenIterator& end, GameVersion ver, TokenArray& params) const;
         CmdSyntaxArray    Query(const wstring& str, GameVersion ver) const;

		   // ----------------------- MUTATORS ------------------------
      public:
         void  Clear();
         UINT  Enumerate(WorkerData* data);

      private:
         void  Merge(SyntaxFile&& f);

		   // -------------------- REPRESENTATION ---------------------

      private:
         CommandCollection  Commands;
         GroupCollection    Groups;
         SyntaxNode         NameTree;
      };

   }
}



using namespace Logic::Scripts;



