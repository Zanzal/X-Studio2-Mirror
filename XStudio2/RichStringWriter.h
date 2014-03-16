#pragma once

#include "../Logic/LanguagePage.h"
#include "../Logic/RichStringParser.h"
#include "LanguageEdit.h"

namespace GUI
{
   namespace Utils
   {

      /// <summary></summary>
      class RichStringWriter
      {
         // ------------------------ TYPES --------------------------
      protected:
         /// <summary>List of tags</summary>
         class TagList : public list<TagType>
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            TagList()
            {}

            // ---------------------- ACCESSORS ------------------------

            /// <summary>Query presence of tag</summary>
            /// <param name="t">tag</param>
            /// <returns></returns>
            bool Contains(TagType t)
            {
               return any_of(begin(), end(), [t](TagType tag) {return tag==t;} );
            }

            // ----------------------- MUTATORS ------------------------

            /// <summary>Adds a tag to the back of the list</summary>
            TagList& operator+=(TagType t)
            {
               push_back(t);
               return *this;
            }
         };

         /// <summary>Formatting tag stack</summary>
         class TagStack : protected list<TagType>
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            TagStack()
            {}

            // ---------------------- ACCESSORS ------------------------

            /// <summary>Query stack empty</summary>
            bool empty() const
            {
               return __super::empty();
            }

            // ----------------------- MUTATORS ------------------------

            /// <summary>Clear stack</summary>
            void Clear() 
            {
               __super::clear();
            }

            /// <summary>Pushes stack</summary>
            /// <param name="t">The t.</param>
            void Push(TagType t)
            {
               push_front(t);
            }

            /// <summary>Pops stack.</summary>
            /// <returns></returns>
            /// <exception cref="Logic::InvalidOperationException">Stack is empty</exception>
            TagType Pop()
            {
               if (empty())
                  throw InvalidOperationException(HERE, L"Stack is empty");

               auto t = front();
               pop_front();
               return t;
            }
         };

         /// <summary>Defines character state.</summary>
         class CharState
         {
            // --------------------- CONSTRUCTION ----------------------
         public:
            /// <summary>Create character state from font and paragraph data.</summary>
            /// <param name="f">font.</param>
            /// <param name="p">paragraph.</param>
            CharState(TextFontPtr f, TextParaPtr p)
            {
               // Formatting
               Bold      = f->Bold == TOM::tomTrue;
               Italic    = f->Italic == TOM::tomTrue;
               Underline = f->Underline == TOM::tomTrue;
               
               // Colour
               Colour = RichStringWriter::FromRGB(f->ForeColor);

               // Alignment
               switch (p->Alignment)
               {
               case TOM::tomAlignLeft:     Alignment = TagType::Left;     break;
               case TOM::tomAlignCenter:   Alignment = TagType::Centre;   break;
               case TOM::tomAlignRight:    Alignment = TagType::Right;    break;
               case TOM::tomAlignJustify:  Alignment = TagType::Justify;  break;
               }
            }
            // ------------------------ STATIC -------------------------

            // --------------------- PROPERTIES ------------------------

            PROPERTY_GET(TagList,FormatTags,GetFormatTags);

            // ---------------------- ACCESSORS ------------------------			

            /// <summary>Gets all colour and formatting tags</summary>
            /// <returns></returns>
            TagList  GetFormatTags() const
            {
               TagList tags;

               // Formatting
               if (Bold)
                  tags += TagType::Bold;
               if (Italic)
                  tags += TagType::Italic;
               if (Underline)
                  tags += TagType::Underline;

               // Colour
               tags += Colour;

               return tags;
            }

            /// <summary>Compares formatting, colour and alignment.</summary>
            /// <param name="r">The r.</param>
            /// <returns></returns>
            bool operator==(const CharState& r) const
            {
               return Bold==r.Bold && Italic==r.Italic && Underline==r.Underline && Colour==r.Colour && Alignment==r.Alignment;
            }
            bool operator!=(const CharState& r) const
            {
               return !operator==(r);
            }

            /// <summary>Get whether another state has character formatting this instance does not</summary>
            /// <param name="r">The r.</param>
            /// <returns></returns>
            bool operator<(const CharState& r) const
            {
               return r.Bold && !Bold || r.Italic && !Italic || r.Underline && !Underline || r.Colour != Colour;
            }

            /// <summary>Get whether this instance has formatting another state does not</summary>
            /// <param name="r">The r.</param>
            /// <returns></returns>
            bool operator>(const CharState& r) const
            {
               return Bold && !r.Bold || Italic && !r.Italic || Underline && !r.Underline || Colour != r.Colour;
            }

            /// <summary>Get formatting in this instance not present in another</summary>
            /// <param name="r">The r.</param>
            /// <returns></returns>
            TagList operator-(const CharState& r) const
            {
               TagList diff;

               // Formatting
               if (Bold && !r.Bold)
                  diff += TagType::Bold;
               if (Italic && !r.Italic)
                  diff += TagType::Italic;
               if (Underline && !r.Underline)
                  diff += TagType::Underline;

               // Colour
               if (Colour != r.Colour)
                  diff += Colour;

               return diff;
            }
            
            // ----------------------- MUTATORS ------------------------

            // -------------------- REPRESENTATION ---------------------
         public:
            bool     Bold, 
                     Italic, 
                     Underline;
            TagType  Colour;
            TagType  Alignment;
         };

         // --------------------- CONSTRUCTION ----------------------
      public:
         RichStringWriter(LanguageEdit& edit, TextDocumentPtr& doc, ColourTag tags);
         virtual ~RichStringWriter();

         DEFAULT_COPY(RichStringWriter);	// Default copy semantics
         DEFAULT_MOVE(RichStringWriter);	// Default move semantics

         // ------------------------ STATIC -------------------------
      public:
         static TagType  FromRGB(COLORREF c);
         static wstring  GetTagString(TagType t);
         static LPCWSTR  GetUnixCode(TagType t);

      protected:
         /// <summary>Object replacement character.</summary>
         const static wchar OBJ = 0xfffc;

         // --------------------- PROPERTIES ------------------------
      protected:
         PROPERTY_GET(long,InputLength,GetInputLength);

         // ---------------------- ACCESSORS ------------------------			
      protected:
         long  GetInputLength() const;

         // ----------------------- MUTATORS ------------------------
      public:
         wstring  Write();

      protected:
         void   OnFormattingLost(TagList& lost);
         void   OnFormattingGained(const TagList& lost);
         void   OnParagraphOpened(CharState s);
         void   OnParagraphClosed(TagType para);
         void   WriteButton(UINT charIndex);
         void   WriteChar(TextRangePtr chr);
         void   WriteTag(TagType t, bool open);

         // -------------------- REPRESENTATION ---------------------
      protected:
         TextDocumentPtr Input;
         LanguageEdit&   Edit;
         wstring         Output;
         ColourTag       ColourTags;
         TagStack        Formatting;
      };

   }
}

using namespace Logic::IO;
