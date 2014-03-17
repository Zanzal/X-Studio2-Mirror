#pragma once

#include "ProjectItem.h"

namespace Logic
{
   namespace Projects
   {
      
      /// <summary></summary>
      class LogicExport ProjectFile
      {
         // ------------------------ TYPES --------------------------
      public:

         // --------------------- CONSTRUCTION ----------------------
      public:
         ProjectFile(const Path& path) : Root(path.FileName)
         {
            Root.FullPath = path;
         }

         virtual ~ProjectFile()
         {}

         DEFAULT_COPY(ProjectFile);	// Default copy semantics
         DEFAULT_MOVE(ProjectFile);	// Default move semantics

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------

         // ---------------------- ACCESSORS ------------------------			
      public:
         /// <summary>Iterate thru items</summary>
         ProjectItemList::const_iterator begin() const  { return Root.Children.begin(); }
         ProjectItemList::const_iterator end() const    { return Root.Children.end();   }

         /// <summary>Query existence of item by path.</summary>
         /// <param name="path">Full path.</param>
         /// <returns></returns>
         bool Contains(Path path) const
         {
            return Root.Contains(path);
         }

         /// <summary>Finds item by path.</summary>
         /// <param name="path">Full path.</param>
         /// <returns>Item if found, otherwise nullptr</returns>
         ProjectItem*  Find(Path path) const
         {
            return Root.Find(path);
         }

         /// <summary>Finds item by name and type.</summary>
         /// <param name="type">Type.</param>
         /// <param name="name">Name.</param>
         /// <returns>Item if found, otherwise nullptr</returns>
         ProjectItem*  Find(ProjectItemType type, const wstring& name) const
         {
            return Root.Find(type, name);
         }

          /// <summary>Finds root folder for a filetype.</summary>
         /// <param name="type">File Type.</param>
         /// <returns>Item if found, otherwise nullptr</returns>
         ProjectItem*  FindFolder(FileType type) const
         {
            switch (type)
            {
            case FileType::Script:    return Root.Find(ProjectItemType::Folder, L"MSCI Scripts");
            case FileType::Language:  return Root.Find(ProjectItemType::Folder, L"Language Files");
            case FileType::Mission:   return Root.Find(ProjectItemType::Folder, L"MD Scripts");
            default:                  return Root.Find(ProjectItemType::Folder, L"Other Files");
            }
         }

         /// <summary>Get all items as a list.</summary>
         /// <returns></returns>
         list<ProjectItem*>  ToList() const
         {
            list<ProjectItem*> l;
            Root.ToList(l);
            return l;
         }
         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Iterate thru items</summary>
         ProjectItemList::iterator begin()   { return Root.Children.begin(); }
         ProjectItemList::iterator end()     { return Root.Children.end();   }

         /// <summary>Add base folder</summary>
         /// <param name="p">item</param>
         /// <exception cref="Logic::ArgumentException">Item is not fixed</exception>
         ProjectItem&  Add(const ProjectItem& p)
         {
            return Root.Add(p);
         }

         //ProjectItem& operator[](UINT index)
         //{
         //   UINT i = 0;

         //   // Linear search
         //   for (auto& item : Items)
         //      if (index == i++)
         //         return item;
         //   
         //   // Invalid index
         //   throw IndexOutOfRangeException(HERE, index, Items.size());
         //}

         /// <summary>Finds an removes an item</summary>
         /// <param name="p">item</param>
         /// <returns>Item removed, or nullptr</returns>
         /// <exception cref="Logic::ArgumentException">Item is fixed</exception>
         bool  Remove(ProjectItem& p)
         {
            // Cannot remove base folders
            if (p.Fixed)
               throw ArgumentException(HERE, L"p", VString(L"Cannot remove fixed item '%s'", p.Name.c_str()));

            // Remove
            return Root.Remove(p);
         }

         // -------------------- REPRESENTATION ---------------------
      public:
         ProjectItem  Root;
         
      };

   }
}

using namespace Logic::Projects;
