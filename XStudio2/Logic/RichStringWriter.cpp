#include "stdafx.h"
#include "RichStringWriter.h"
#include "RtfStringWriter.h"

namespace Logic
{
   namespace IO
   {
   
      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Creates a rich text source code writer for a TOM text document</summary>
      /// <param name="doc">The document.</param>
      /// <param name="tags">Type of colour tags to use</param>
      /// <exception cref="Logic::ArgumentNullException">Document is null</exception>
      RichStringWriter::RichStringWriter(TextDocumentPtr& doc, ColourTag tags) : Input(doc), ColourTags(ColourTag::Unix)
      {
         REQUIRED(doc);
      }


      RichStringWriter::~RichStringWriter()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      /// <summary>Converts RGB to language tag.</summary>
      /// <param name="c">colour</param>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Unrecognised colour</exception>
      TagType  RichStringWriter::FromRGB(COLORREF c)
      {
         switch ((LanguageColour)c)
         {
         case LanguageColour::Black:   return TagType::Black;
         case LanguageColour::Default: return TagType::Default;
         case LanguageColour::White:   return TagType::White;  
         case LanguageColour::Silver:  return TagType::Silver;

         case LanguageColour::Grey:    return TagType::Grey;
         case LanguageColour::Blue:    return TagType::Blue;
         case LanguageColour::Cyan:    return TagType::Cyan;
         case LanguageColour::Green:   return TagType::Green;
         case LanguageColour::Orange:  return TagType::Orange;
         case LanguageColour::Purple:  return TagType::Magenta;
         case LanguageColour::Red:     return TagType::Red;
         case LanguageColour::Yellow:  return TagType::Yellow;
         }

         throw ArgumentException(HERE, L"c", GuiString(L"Unrecognised RGB colour: 0x%x", c));
      }
      
      /// <summary>Get tag name from enum</summary>
      /// <param name="t">tag type.</param>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Invalid tag</exception>
      wstring  RichStringWriter::GetTagString(TagType t)
      {
         const wchar* str[] = { L"b", L"u", L"i", 
                                L"left", L"right", L"center", L"justify", 
                                L"text", L"select", L"author", L"title", L"ranking", 
                                L"black", L"blue", L"cyan", L"green", L"grey", L"magenta", L"orange", L"red", L"silver", L"yellow", L"white", 
                                L"default", L"unrecognised" };

         // Validate
         if ((UINT)t > (UINT)TagType::Unrecognised)
            throw ArgumentException(HERE, L"t", GuiString(L"Unrecognised Tag id=%d", t));
         else if ((UINT)t >= (UINT)TagType::Default)
            throw ArgumentException(HERE, L"t", GuiString(L"Tag type '%s' has no string representation", GetString(t).c_str()));

         return str[(UINT)t];
      }
      
      /// <summary>Converts colour tag to unix colour code.</summary>
      /// <param name="t">tag</param>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Not a colour tag</exception>
      const wchar* RichStringWriter::GetUnixCode(TagType t)
      {
         switch (t)
         {
         case TagType::Silver:   return L"\\033A";   //Silver
         case TagType::Blue:     return L"\\033B";   //Blue
         case TagType::Cyan:     return L"\\033C";   //Cyan
         case TagType::Green:    return L"\\033G";   //Green
         case TagType::Orange:   return L"\\033O";   //Orange
         case TagType::Magenta:  return L"\\033M";   //Magenta
         case TagType::Red:      return L"\\033R";   //Red
         case TagType::White:    return L"\\033W";   //White
         case TagType::Default:  return L"\\033X";   //Default
         case TagType::Yellow:   return L"\\033Y";   //Yellow
         case TagType::Black:    return L"\\033Z";   //Black
         }

         throw ArgumentException(HERE, L"t", GuiString(L"'%s' has no associated colour code", GetString(t).c_str()));
      }

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Generates richText source code for the input document</summary>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Unrecognised colour</exception>
      /// <exception cref="Logic::ComException">COM error</exception>
      /// <exception cref="Logic::InvalidOperationException">Algorithm error</exception>
      wstring  RichStringWriter::Write()
      {
         try
         {
            if (InputLength > 0)
            {
               TextRangePtr thisChar = Input->Range(0, 1),
                            prevChar(thisChar->Duplicate);
               CharState    thisState(thisChar->Font, thisChar->Para),
                            prevState(thisState);

               // Open paragraph
               OnParagraphOpened(thisState);
               WriteChar(thisChar);

               // Examine each character
               for (int i = 1, end = InputLength; i < end; ++i)
               {
                  // Advance this
                  thisChar->Move(TOM::tomCharacter, 1);
                  thisState = CharState(thisChar->Font, thisChar->Para);

                  // Paragraph change: Close open tags, Open new paragraph with same tags
                  if (thisState.Alignment != prevState.Alignment)
                  {
                     OnParagraphClosed(prevState.Alignment);
                     OnParagraphOpened(thisState);
                  }
                  // Format change: 
                  else if (thisState != prevState)
                  {
                     // Formatting lost: Close tags
                     if (thisState < prevState)
                        OnFormattingLost(prevState - thisState);
                     
                     // Formatting gained: Open tags
                     if (thisState > prevState)
                        OnFormattingGained(thisState - prevState);
                  }
                  
                  // Write character
                  WriteChar(thisChar);

                  // Re-sync prev 
                  prevChar->Move(TOM::tomCharacter, 1);
                  prevState = thisState;
               }

               // Close paragraph
               OnParagraphClosed(thisState.Alignment);
            }

            return Output;
         }
         catch (_com_error& e) {
            throw ComException(HERE, e);
         }
      }

      // ------------------------------ PROTECTED METHODS -----------------------------
      
      /// <summary>Gets the length of the input.</summary>
      /// <returns></returns>
      long  RichStringWriter::GetInputLength() const
      {
         return Input->Range(0, 0)->EndOf(TOM::tomStory, 1);
      }

      /// <summary>Closes the lost tags while maintaining existing formatting by closing/re-opening intermediate tags</summary>
      /// <param name="lost">List of lost formatting tags.</param>
      void  RichStringWriter::OnFormattingLost(TagList& lost)
      {
         TagList unchanged;
         
         // Close all open tags until all lost formatting has been closed
         while (!lost.empty())
         {
            if (Formatting.empty())
               throw AlgorithmException(HERE, L"Formatting stack is empty");

            // Close tag
            auto tag = Formatting.Pop();
            WriteTag(tag, false);

            // Remove/preserve tag
            if (lost.Contains(tag))
               lost.remove(tag);
            else
               unchanged += tag;
         }

         // Re-open tags whoose formatting was unchanged
         for (TagType t : unchanged)
         {
            WriteTag(t, true);
            Formatting.Push(t);
         }
      }

      /// <summary>Opens new formatting tags</summary>
      /// <param name="gained">List of new formatting tags.</param>
      void  RichStringWriter::OnFormattingGained(const TagList& gained)
      {
         // Open tags
         for (TagType t : gained)
         {
            WriteTag(t, true);
            Formatting.Push(t);
         }
      }

      /// <summary>Opens a new paragraph and any necessary formatting tags</summary>
      /// <param name="s">state of first character in the paragraph.</param>
      void  RichStringWriter::OnParagraphOpened(CharState s)
      {
         // Open paragraph
         WriteTag(s.Alignment, true);

         // Open tags (if any)
         for (TagType t : s.FormatTags)
         {
            WriteTag(t, true);
            Formatting.Push(t);
         }
      }

      /// <summary>Closes the paragraph and all open formatting tags</summary>
      /// <param name="para">Paragraph alignment.</param>
      void  RichStringWriter::OnParagraphClosed(TagType para)
      {
         // Close all tags
         while (!Formatting.empty())
            WriteTag(Formatting.Pop(), false);
         
         // Close paragraph
         WriteTag(para, false);
      }

      /// <summary>Writes the first character in a range</summary>
      /// <param name="chr">range</param>
      void  RichStringWriter::WriteChar(TextRangePtr chr)
      {
         switch (wchar ch = (wchar)chr->Char)
         {
         case '\r':  break;
         case '\v':  Output += '\n';  break;
         default:    Output += ch;    break;
         }
      }

      /// <summary>Writes an opening or closing tag.</summary>
      /// <param name="t">Tag</param>
      /// <param name="open">Whether opening or closing</param>
      void  RichStringWriter::WriteTag(TagType t, bool open)
      {
         switch (t)
         {
         // Default: Has no named representation
         case TagType::Default:
            break;

         case TagType::Black:
         case TagType::Blue:
         case TagType::Cyan:
         case TagType::Green:
         case TagType::Magenta:
         case TagType::Orange:
         case TagType::Red:
         case TagType::Silver:
         case TagType::Yellow:
         case TagType::White:
            // Named: Output normally
            if (ColourTags == ColourTag::Message)
               Output += GuiString(open ? L"[%s]" : L"[/%s]", GetTagString(t).c_str());
            else  
               // UNIX: Use unix codes and /033X instead of a closing tag
               Output += GetUnixCode(open ? t : TagType::Default);
            break;

         // Formatting/Paragraph: Output normally
         default:
            Output += GuiString(open ? L"[%s]" : L"[/%s]", GetTagString(t).c_str());
         }  
      }

      // ------------------------------- PRIVATE METHODS ------------------------------
   
   }
}
