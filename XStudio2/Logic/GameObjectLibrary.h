#pragma once

#include "Common.h"
#include "TFile.hpp"
#include "GameObject.h"
#include <regex>

namespace Logic
{
   namespace Types
   {
      /// <summary></summary>
      class TFilenameReader
      {
      public:
         TFilenameReader(Path p) : Valid(true), Type(MainType::Unknown)
         {
            // Get lowercase filename without extension
            GuiString name = GuiString(p.RemoveExtension().FileName).ToLower();

            // Identify file
            if (name == L"tlaser")
               Type = MainType::Laser;
            else if (name == L"tmissiles")
               Type = MainType::Missile;
            else if (name == L"tships")
               Type = MainType::Ship;
            else if (name == L"tdocks")
               Type = MainType::Dock;
            else if (name == L"tshields")
               Type = MainType::Shield;
            else if (name == L"tfactories")
               Type = MainType::Factory;
            else if (name == L"twaret")
               Type = MainType::TechWare;
            else if (name == L"twareb")
               Type = MainType::BioWare;
            else if (name == L"twaree")
               Type = MainType::EnergyWare;
            else if (name == L"twarem")
               Type = MainType::MineralWare;
            else if (name == L"twaren")
               Type = MainType::NaturalWare;
            else if (name == L"twaref")
               Type = MainType::FoodWare;
            else
               Valid = false;
         }

         bool      Valid;
         MainType  Type;
      };

      /// <summary></summary>
      class GameObjectLibrary
      {
		   // ------------------------ TYPES --------------------------
      public:
	      /// <summary>Defines a {MainType,SubType} pair</summary>
         class ObjectID
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            ObjectID() : Type(MainType::Unknown), SubType(0)
            {}
            ObjectID(MainType t, UINT subtype) : Type(t), SubType(subtype)
            {}

            // ---------------------- ACCESSORS ------------------------	
         public:
            bool operator==(const ObjectID& r) const {
               return Type == r.Type && SubType == r.SubType;
            }
            bool operator<(const ObjectID& r) const {
               return Type < r.Type || (Type == r.Type && SubType < r.SubType);
            }

            // -------------------- REPRESENTATION ---------------------
         public:
            MainType Type;
            UINT     SubType;
         };

         /// <summary>Collection of game objects sorted by SubType</summary>
         class ObjectCollection : public map<ObjectID, GameObject, less<ObjectID>>
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            // ---------------------- ACCESSORS ------------------------			

            /// <summary>Query whether an object is present</summary>
            /// <param name="main">The main type</param>
            /// <param name="subtype">The subtype</param>
            /// <returns></returns>
            bool  Contains(MainType main, UINT subtype) const
            {
               return find(ObjectID(main, subtype)) != end();
            }

            /// <summary>Finds a game object by SubType</summary>
            /// <param name="type">Main type.</param>
            /// <param name="subtype">SubType.</param>
            /// <returns></returns>
            /// <exception cref="Logic::GameObjectNotFoundException">Object not found</exception>
            GameObjectRef  Find(MainType type, UINT subtype) const
            {
               const_iterator it;
               // Lookup and return string
               if ((it = find(ObjectID(type, subtype))) != end())
                  return it->second;

               // Error: Not found
               throw GameObjectNotFoundException(HERE, type, subtype);
            }

            /// <summary>Attempts to find a game object by subtype</summary>
            /// <param name="type">Main type.</param>
            /// <param name="subtype">SubType.</param>
            /// <param name="obj">object</param>
            /// <returns>true if found, false otherwise</returns>
            bool  TryFind(MainType type, UINT subtype, const GameObject* &obj) const
            {
               const_iterator it;

               // Lookup object
               obj = (it = find(ObjectID(type, subtype))) != end() ? &it->second : nullptr;
               // Return true if found
               return obj != nullptr;
            }

            // ----------------------- MUTATORS ------------------------
         public:
            /// <summary>Adds an object to the collection</summary>
            /// <param name="obj">The object</param>
            /// <returns>True if successful, false if MainType/SubType combination already present</returns>
            bool  Add(const GameObject& obj)
            {
               return insert(value_type(ObjectID(obj.Type, obj.SubType), obj)).second;
            }
         };

      public:
         /// <summary>Collection of game objects sorted by text</summary>
         class LookupCollection : public map<GuiString, GameObject, less<GuiString>>
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            // ---------------------- ACCESSORS ------------------------			

            /// <summary>Check whether an object is present</summary>
            /// <param name="sz">The text</param>
            /// <returns></returns>
            bool  Contains(const GuiString& sz) const
            {
               return find(sz) != end();
            }

            /// <summary>Finds an object by text (Search is case sensitive)<summary>
            /// <param name="sz">The text</param>
            /// <returns>Object</returns>
            /// <exception cref="Logic::GameObjectNotFoundException">Object not found</exception>
            GameObjectRef  Find(const GuiString& sz) const
            {
               const_iterator it;
               // Lookup and return string
               if ((it = find(sz)) != end())
                  return it->second;

               // Error: Not found
               throw GameObjectNotFoundException(HERE, sz);
            }
            
            /// <summary>Tries to find a game object by name</summary>
            /// <param name="name">object name</param>
            /// <param name="obj">object</param>
            /// <returns>true if found, false otherwise</returns>
            bool  TryFind(const GuiString& name, const GameObject* &obj) const
            {
               const_iterator it;

               // Lookup object, Set/clear result
               obj = (it=find(name)) != end() ? &it->second : nullptr;
               return obj != nullptr;
            }

            // ----------------------- MUTATORS ------------------------
         public:
            /// <summary>Adds an object</summary>
            /// <param name="obj">The object.</param>
            /// <returns>True if successful, false if key already present</returns>
            bool  Add(const GameObject& obj)
            {
               return insert(value_type(obj.Name, obj)).second;
            }

            /// <summary>Removes an object from the collection  (Case sensitive)</summary>
            /// <param name="sz">The text</param>
            void  Remove(const GuiString& sz)
            {
               erase(sz);
            }
         };

         // --------------------- CONSTRUCTION ----------------------

      public:
         GameObjectLibrary();
         virtual ~GameObjectLibrary();
		 
		   NO_COPY(GameObjectLibrary);	// No copy semantics
		   NO_MOVE(GameObjectLibrary);	// No move semantics

         // ------------------------ STATIC -------------------------
      public:
         static GameObjectLibrary  Instance;

      private:
         static const wregex  PlaceHolder;

         // --------------------- PROPERTIES ------------------------
			
         // ---------------------- ACCESSORS ------------------------			
      public:
         bool  Contains(UINT value) const;
         bool  Contains(const GuiString& name) const;
         bool  Contains(MainType main, UINT subtype) const;
         bool  ParsePlaceholder(const GuiString& name, ObjectID& id) const;

         // ----------------------- MUTATORS ------------------------
      public:
         void            Clear();
         UINT            Enumerate(const XFileSystem& vfs, WorkerData* data);
         //const TObject*  Find(MainType main, UINT subtype) const;
         GameObjectRef   Find(UINT value) const;
         GameObjectRef   Find(const GuiString& name) const;
         GameObjectRef   Find(MainType main, UINT subtype) const;
         GameObjectArray Query(const GuiString& search) const;
         bool            TryFind(const GuiString& name, const GameObject* &obj) const;

      protected:
         UINT            PopulateObjects(WorkerData* data);
         
         // -------------------- REPRESENTATION ---------------------
      private:
         vector<TFilePtr>  Files;
         ObjectCollection  Objects;
         LookupCollection  Lookup;
      };
   
      // Access to Game object library singleton
      #define GameObjectLib  GameObjectLibrary::Instance
   }
}

using namespace Logic::Types;
