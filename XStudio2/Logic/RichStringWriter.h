#pragma once
#include "Common.h"
#include "LanguagePage.h"

namespace Logic
{
   namespace IO
   {

      /// <summary></summary>
      class RichStringWriter
      {
         // ------------------------ TYPES --------------------------
      private:

         // --------------------- CONSTRUCTION ----------------------

      public:
         RichStringWriter(TextDocumentPtr& doc, ColourTag tags);
         virtual ~RichStringWriter();

         DEFAULT_COPY(RichStringWriter);	// Default copy semantics
         DEFAULT_MOVE(RichStringWriter);	// Default move semantics

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------

         // ---------------------- ACCESSORS ------------------------			

         // ----------------------- MUTATORS ------------------------
      public:
         wstring  Write();

      protected:
         void  WriteChar(wchar ch);
         void  WriteChar(TextRangePtr chr, TextRangePtr prev);
         void  WriteState(TextRangePtr chr, bool open);

         // -------------------- REPRESENTATION ---------------------

      protected:
         TextDocumentPtr Input;
         wstring         Output;
         ColourTag       TagType;
      };

   }
}

using namespace Logic::IO;
