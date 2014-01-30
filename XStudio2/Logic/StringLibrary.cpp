#include "stdafx.h"
#include "StringLibrary.h"
#include "LanguageFileReader.h"
#include "StringParser.h"

namespace Logic
{
   namespace Language
   {
      StringLibrary  StringLibrary::Instance;

      // -------------------------------- CONSTRUCTION --------------------------------

      StringLibrary::StringLibrary()
      {
      }


      StringLibrary::~StringLibrary()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Populates the library with all the language files in the 't' subfolder</summary>
      /// <param name="vfs">The VFS to search</param>
      /// <param name="lang">The language of strings to search for</param>
      /// <param name="data">Background worker data</param>
      /// <returns>Number of files found</returns>
      UINT  StringLibrary::Enumerate(XFileSystem& vfs, GameLanguage lang, WorkerData* data)
      {
         list<XFileInfo> results;

         // Clear previous contents
         Clear();

         // Feedback
         data->SendFeedback(ProgressType::Info, 1, L"Enumerating language files");
         Console << ENDL << Colour::Cyan << L"Enumerating language files" << ENDL;

         // Enumerate non-foreign language files
         for (XFileInfo& f : vfs.Browse(XFolder::Language))
         {
            LanguageFilenameReader fn(f.FullPath.FileName);

            // Add if language matches or is unspecified
            if (fn.Valid && (fn.Language == lang || !fn.HasLanguage))
               results.push_back(f);
         }

         // Read/Store each file
         for (XFileInfo& f : results)
         {
            try
            {
               // Feedback
               data->SendFeedback(ProgressType::Info, 2, GuiString(L"Reading language file '%s'...", (const WCHAR*)f.FullPath));
               Console << L"Reading language file: " << f.FullPath << L"...";

               // Parse language file
               LanguageFile file = LanguageFileReader(f.OpenRead()).ReadFile(f.FullPath.FileName);

               // Skip files that turn out to be foreign
               if (file.Language == lang)
                  Files.insert(file);

               Console << Colour::Green << L"Success" << ENDL;
            }
            catch (ExceptionBase& e)
            {
               data->SendFeedback(ProgressType::Warning, 3, GuiString(L"Failed: ") + e.Message);
               Console << Colour::Red << L"Failed: " << e.Message << ENDL;
            }
         }

         return Files.size();
      }

      /// <summary>Clears the library of all files/strings</summary>
      void  StringLibrary::Clear()
      {
         Files.clear();
      }

      /// <summary>Queries whether a string is present</summary>
      /// <param name="page">The page id</param>
      /// <param name="id">The string id.</param>
      /// <returns></returns>
      bool  StringLibrary::Contains(UINT page, UINT id) const
      {
         // Search files in descending file ID
         for (const LanguageFile& f : Files)
            if (f.Contains(page, id))
               return true;

         // Not found
         return false;
      }

      /// <summary>Finds the specified string</summary>
      /// <param name="page">The page id</param>
      /// <param name="id">The string id</param>
      /// <returns></returns>
      /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
      /// <exception cref="Logic::StringNotFoundException">String does not exist</exception>
      LanguageString  StringLibrary::Find(UINT page, UINT id) const
      {
         // Search files in descending file ID
         for (const LanguageFile& f : Files)
            if (f.Contains(page, id))
               return f.Find(page, id);

         // Not found
         throw StringNotFoundException(HERE, page, id);
      }

      /// <summary>Finds a string, resolves the substrings and removes the comments</summary>
      /// <param name="page">The page id</param>
      /// <param name="id">The string id</param>
      /// <returns>Resolved text</returns>
      /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
      /// <exception cref="Logic::StringNotFoundException">String does not exist</exception>
      wstring  StringLibrary::Resolve(UINT page, UINT id) const
      {
         return Find(page,id).ResolvedText;
      }

		// ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

   }
}
