#pragma once
#include "Common.h"
#include "LanguageFile.h"
#include "XmlReader.h"

using namespace Library::IO;
using namespace Library::Language;

namespace Library
{
   namespace IO
   {
     

      /// <summary>Reads strings and pages of an X3 language xml file</summary>
      class LanguageFileReader : XmlReader
      {
         // --------------------- CONSTRUCTION ----------------------

      public:
         LanguageFileReader(StreamPtr in);
         ~LanguageFileReader();

         // ------------------------ STATIC -------------------------

      private:
         static GameLanguage  ParseLanguageID(wstring id);

         // --------------------- PROPERTIES ------------------------
			
			// ---------------------- ACCESSORS ------------------------

			// ----------------------- MUTATORS ------------------------

      public:
         LanguageFile   ReadFile();

      private:
         GameLanguage   ReadLanguageTag(XML::IXMLDOMNodePtr&  element);
         LanguagePage   ReadPage(XML::IXMLDOMNodePtr&  element);
         LanguageString ReadString(XML::IXMLDOMNodePtr&  element);

         // -------------------- REPRESENTATION ---------------------
      };

   }
}