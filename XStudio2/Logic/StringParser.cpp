#include "stdafx.h"
#include "StringParser.h"
#include <algorithm>

namespace Logic
{
   namespace Language
   {
      /// <summary>Matches an opening tag (at beginning of string) and captures the name</summary>
      const wregex  StringParser::IsOpeningTag = wregex(L"^\\[([a-z]+)(?:\\s+[a-z]+\\s*=\\s*'\\w+')*\\]");

      /// <summary>Matches any closing tag (at beginning of string) and captures the name</summary>
      const wregex  StringParser::IsClosingTag = wregex(L"^\\[/?([a-z]+)\\]");
      
      /// <summary>Matches any opening/closing tag (at beginning of string) without properties and captures the name</summary>
      const wregex  StringParser::IsBasicTag = wregex(L"^\\[/?([a-z]+)\\]");

      /// <summary>Matches multiple tag properties captures both name and value</summary>
      const wregex  StringParser::IsTagProperty = wregex(L"\\s+([a-z]+)\\s*=\\s*'(\\w+)'");

      /// <summary>Matches opening and closing [author] tags (at beginning of string) and captures the text</summary>
      const wregex  StringParser::IsAuthorDefition = wregex(L"^[author](.*)[/author]");

      /// <summary>Matches opening and closing [title] tags (at beginning of string) and captures the text</summary>
      const wregex  StringParser::IsTitleDefition = wregex(L"^[title](.*)[/title]");
   
      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Creates a rich-text parser from an input string</summary>
      /// <param name="str">string containing rich-text tags</param>
      /// <exception cref="Logic::AlgorithmException">Error in parsing algorithm</exception>
      /// <exception cref="Logic::ArgumentException">Error in parsing algorithm</exception>
      /// <exception cref="Logic::Language::RichTextException">Error in formatting tags</exception>
      StringParser::StringParser(const wstring& str) : Input(str), Alignments(TagClass::Paragraph)
      {
         Parse();
      }


      StringParser::~StringParser()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------

      /// <summary>Gets the associated paragraph alignment of a paragraph tag</summary>
      /// <param name="t">tag type</param>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Not a paragraph tag</exception>
      Alignment StringParser::GetAlignment(TagType t) 
      {
         switch (t)
         {
         case TagType::Left:     return Alignment::Left;
         case TagType::Right:    return Alignment::Right;
         case TagType::Centre:   return Alignment::Centre;
         case TagType::Justify:  return Alignment::Justify;
         }
         
         // Not paragraph tag
         throw ArgumentException(HERE, L"t", GuiString(L"Cannot get alignment for %s tag", GetString(t).c_str()));
      }

      /// <summary>Gets the class of a tag</summary>
      /// <param name="t">tag type</param>
      /// <returns></returns>
      /// <exception cref="Logic::ArgumentException">Tag is unrecognised</exception>
      StringParser::TagClass  StringParser::GetClass(TagType t)
      {
         switch (t)
         {
         case TagType::Bold:
         case TagType::Underline:
         case TagType::Italic:
            return TagClass::Character;

         case TagType::Left:
         case TagType::Right:
         case TagType::Centre:
         case TagType::Justify:
            return TagClass::Paragraph;

         case TagType::Text:
         case TagType::Select:
         case TagType::Author:
         case TagType::Title:
         case TagType::Rank:
            return TagClass::Special;

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
            return TagClass::Colour;
         }

         throw ArgumentException(HERE, L"t", L"Unrecognised tags have no class");
      }

      /// <summary>Convert paragraph alignment string</summary>
      /// <param name="a">alignment</param>
      /// <returns></returns>
      wstring  GetString(Alignment a)
      {
         switch (a)
         {
         case Alignment::Left:    return L"Left";
         case Alignment::Centre:  return L"Centre";
         case Alignment::Right:   return L"Right";
         case Alignment::Justify: return L"Justify";
         }
         return L"Invalid";
      }
      
      /// <summary>Convert tag class to string</summary>
      /// <param name="c">tag class.</param>
      /// <returns></returns>
      wstring  StringParser::GetString(TagClass c)
      {
         const wchar* str[] = { L"Character", L"Paragraph", L"Colour", L"Special" };

         return str[(UINT)c];
      }
      
      /// <summary>Convert tag type to string</summary>
      /// <param name="t">tag type.</param>
      /// <returns></returns>
      wstring  StringParser::GetString(TagType t)
      {
         const wchar* str[] = { L"Bold", L"Underline", L"Italic", L"Left", L"Right", L"Centre", L"Justify", L"Text", L"Select", L"Author", 
                                L"Title", L"Rank",L"Black", L"Blue", L"Cyan", L"Green", L"Magenta", L"Orange", L"Red", L"Silver", L"Yellow", 
                                L"White", L"Unrecognised"  };

         return str[(UINT)t];
      }

      /// <summary>Identifies a tag type from it's name</summary>
      /// <param name="name">tag name WITHOUT square brackets</param>
      /// <returns></returns>
      StringParser::TagType  StringParser::IdentifyTag(const wstring& name)
      {
         switch (name.length())
         {
         case 1:
            if (name == L"b")
               return TagType::Bold;
            else if (name == L"i")
               return TagType::Italic;
            else if (name == L"u")
               return TagType::Underline;
            break;

         case 3:
            if (name == L"red")
               return TagType::Red;
            break;

         case 4:
            if (name == L"left")
               return TagType::Left;
            else if (name == L"blue")
               return TagType::Blue;
            else if (name == L"cyan")
               return TagType::Cyan;
            else if (name == L"text")
               return TagType::Text;
            else if (name == L"rank")
               return TagType::Rank;
            break;

         case 5:
            if (name == L"right")
               return TagType::Right;
            else if (name == L"title")
               return TagType::Title;
            else if (name == L"green")
               return TagType::Green;
            else if (name == L"black")
               return TagType::Black;
            else if (name == L"white")
               return TagType::White;
            break;

         case 6:
            if (name == L"center")
               return TagType::Centre;
            else if (name == L"select")
               return TagType::Select;
            else if (name == L"author")
               return TagType::Author;
            else if (name == L"orange")
               return TagType::Orange;
            else if (name == L"yellow")
               return TagType::Yellow;
            else if (name == L"silver")
               return TagType::Silver;
            break;

         case 7:
            if (name == L"magenta")
               return TagType::Magenta;
            else if (name == L"justify")
               return TagType::Justify;
            break;
         }

         // Unrecognised;
         return TagType::Unrecognised;
      }


      /// <summary>Output rich-text element to the console</summary>
      ConsoleWnd& operator<<(ConsoleWnd& c, const RichElement& e)
      {
         if (e.Type == ElementType::Button)
            return c << (const RichButton&)e;

         else if (e.Type == ElementType::Character)
            return c << (const RichCharacter&)e;
         
         return c;
      }

      /// <summary>Output rich-text character to the console</summary>
      ConsoleWnd& operator<<(ConsoleWnd& c, const RichCharacter& e)
      {
         c << e.Colour;
         if (e.Format & CFE_BOLD)
            c << Cons::Bold;

         c << e.Char;

         if (e.Format & CFE_BOLD)
            c << Cons::Normal;

         return c;
      }

      /// <summary>Output rich-text button to the console</summary>
      ConsoleWnd& operator<<(ConsoleWnd& c, const RichButton& e)
      {
         return c << "{RichButton Id=" << e.ID << " Text=";

         // Text
         for (const auto& el : e.Text)
            c << *el;
         
         return c << "}";
      }

      /// <summary>Output rich-text paragraph to the console</summary>
      ConsoleWnd& operator<<(ConsoleWnd& c, const RichParagraph& p)
      {
         // Properties
         c << "{RichParagraph Align=" << GetString(p.Align);

         // Elements
         c << ENDL << "Content=";
         for (const auto& el : p.Content)
            c << *el;
         
         return c << "}";
      }

      /// <summary>Output rich-text string to the console</summary>
      ConsoleWnd& operator<<(ConsoleWnd& c, const RichString& s)
      {
         // Properties
         c << "{RichString Author=" << s.Author << " Title=" << s.Title << " Cols=" << (int)s.Columns << " Width=" << s.Width << " Spacing=" << s.Spacing;

         // Elements
         c << ENDL << "Paragraphs=";
         for (const auto& el : s.Paragraphs)
            c << el;
         
         return c << "}";
      }

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Parses input string</summary>
      /// <exception cref="Logic::AlgorithmException">Error in parsing algorithm</exception>
      /// <exception cref="Logic::ArgumentException">Error in parsing algorithm</exception>
      /// <exception cref="Logic::Language::RichTextException">Error in formatting tags</exception>
      void  StringParser::Parse()
      {
         RichParagraph* Paragraph = &FirstParagraph;     // Current paragraph (output)
         Colour         TextColour = Colour::Default;    // Current colour
         bool           Escaped = false;                 // Whether current char is escaped

         // Iterate thru input
         for (auto ch = Input.begin(); ch != Input.end(); ++ch)
         {
            switch (*ch)
            {
            // Escape/ColourCode: 
            case '\\': 
               // ColourCode: Read and override existing stack colour, if any
               if (MatchColourCode(ch))
                  TextColour = ReadColourCode(ch);
               else
               {  // Backslash: Append as text, escape next character
                  Escaped = true;
                  *Paragraph += new RichCharacter('\\', TextColour, Formatting.Current);
               }
               continue;

            // Open/Close tag:
            case '[':
               // Escaped/Not-a-tag: Append as text
               if (Escaped || !MatchTag(ch))
                  *Paragraph += new RichCharacter('[', TextColour, Formatting.Current);
               else 
               {
                  // RichTag: Read entire tag. Adjust colour/formatting/paragraph
                  RichTag tag = ReadTag(ch);
                  switch (tag.Class)
                  {
                  // Paragraph: Add/Remove alignment. Append new paragraph?
                  case TagClass::Paragraph:
                     Alignments.PushPop(tag);
                     if (tag.Opening)
                     {
                        // Special case: Ignore initial 'left' paragraph
                        if (GetAlignment(tag.Type) == Alignment::Left && Output.Paragraphs.size() == 1 && FirstParagraph.Content.empty())
                           continue;

                        // Append
                        Output += RichParagraph(GetAlignment(tag.Type));
                        Paragraph = &FirstParagraph;
                     }
                     break;

                  // Formatting: Add/Remove current formatting
                  case TagClass::Character:
                     Formatting.PushPop(tag);
                     break;

                  // Colour: Add/Remove current colour. Reset any colourCode override
                  case TagClass::Colour:
                     Colours.PushPop(tag);
                     TextColour = Colours.Current;
                     break;

                  // Special: 
                  case TagClass::Special:
                     switch (tag.Type)
                     {
                     // Author/Title: Insert into output
                     case TagType::Author: Output.Author = tag.Text; break;
                     case TagType::Title:  Output.Title = tag.Text;  break;

                     // Button:
                     case TagType::Select: *Paragraph += CreateButton(tag);  break;
                     case TagType::Text:   SetColumnInfo(tag);               break;
                     }
                     break;
                  }
               }
               break;

            // Char: Append to current paragraph
            default:
               *Paragraph += new RichCharacter(*ch, TextColour, Formatting.Current);
               break;
            }

            // Revert to de-escaped
            Escaped = false;
         }
      }

      // ------------------------------ PROTECTED METHODS -----------------------------

      // ------------------------------- PRIVATE METHODS ------------------------------
   
      /// <summary>Creates a button from a select tag</summary>
      /// <param name="tag">select tag.</param>
      /// <returns></returns>
      /// <exception cref="Logic::InvalidOperation">Not a 'select' tag</exception>
      /// <exception cref="Logic::Language::RichTextException">Invalid tag property</exception>
      RichButton*  StringParser::CreateButton(const RichTag& tag) const
      {
         // Ensure tag is 'select'
         if (tag.Type != TagType::Select)
            throw InvalidOperationException(HERE, GuiString(L"Cannot create a button from a '%s' tag", GetString(tag.Type).c_str()) );

         // Recursively parse text  (To enable character/colour formatting)
         StringParser btn(tag.Text);
                        
         // Get ID
         wstring id;
         if (tag.Properties.empty())
         {
            // Ensure property is 'value'
            if (tag.Properties.front().first != L"value")
               throw RichTextException(HERE, GuiString(L"Unrecognised button property '%s'", tag.Properties.front().first.c_str()) );

            id = tag.Properties.front().second;
         }
                           
         // Append button     [Extract only 1st paragraph of button text, to strip paragraph alignment]
         return new RichButton(id, btn.FirstParagraph.Content);
      }
      

      /// <summary>Gets the first paragraph.</summary>
      /// <returns></returns>
      RichParagraph& StringParser::GetFirstParagraph() 
      {
         return Output.Paragraphs.back();
      }

      /// <summary>Matches a unix style colour code.</summary>
      /// <param name="pos">position of backslash</param>
      /// <returns></returns>
      bool  StringParser::MatchColourCode(CharIterator pos) const
      {
         // Check for EOF?
         if (pos+5 >= Input.end())
            return false;

         // Match '\\033'
         if (pos[0] != '\\' || pos[1] != '0' || pos[2] != '3' || pos[3] != '3')
            return false;

         // Match colour character
         switch (pos[4])
         {
         case 'A':   // Silver
         case 'B':   // Blue
         case 'C':   // Cyan
         case 'G':   // Green
         case 'O':   // Orange
         case 'M':   // Magenta
         case 'R':   // Red
         case 'W':   // White
         case 'X':   // Default
         case 'Y':   // Yellow
         case 'Z':   // Black
            return true;
         }

         // Failed
         return false;
      }

      /// <summary>Matches any opening or closing tag</summary>
      /// <param name="pos">position of opening bracket</param>
      /// <returns></returns>
      bool  StringParser::MatchTag(CharIterator pos) const
      {
         wsmatch match;

         // Match tag using RegEx
         if (regex_search(pos, Input.end(), match, IsOpeningTag) || regex_search(pos, Input.end(), match, IsClosingTag))
            // Ignore unrecognised tags
            return IdentifyTag(match[1].str()) != TagType::Unrecognised;

         // Not a tag
         return false;
      }


      /// <summary>Reads the unix style colour code and advances the iterator</summary>
      /// <param name="pos">position of backslash</param>
      /// <returns></returns>
      /// <exception cref="Logic::AlgorithmException">Previously matched colour code is unrecognised</exception>
      Colour  StringParser::ReadColourCode(CharIterator& pos)
      {
         Colour c;

         // Match colour character
         switch (*pos)
         {
         case 'A':   c = Colour::Silver;  break;
         case 'B':   c = Colour::Blue;    break;
         case 'C':   c = Colour::Cyan;    break;
         case 'G':   c = Colour::Green;   break;
         case 'O':   c = Colour::Orange;  break;
         case 'M':   c = Colour::Purple;  break;
         case 'R':   c = Colour::Red;     break;
         case 'W':   c = Colour::White;   break;
         case 'X':   c = Colour::Default; break;
         case 'Y':   c = Colour::Yellow;  break;
         case 'Z':   c = Colour::Black;   break;
         default:
            throw AlgorithmException(HERE, GuiString(L"Previously matched colour code '%c' is unrecognised", *pos));
         }

         // Consume + return colour
         pos += 5;
         return c;
      }

      /// <summary>Reads the entire tag and advances the iterator</summary>
      /// <param name="pos">position of opening bracket</param>
      /// <returns></returns>
      /// <exception cref="Logic::Language::RichTextException">Closing tag doesn't match currently open tag</exception>
      StringParser::RichTag  StringParser::ReadTag(CharIterator& pos)
      {
         wsmatch matches;

         // BASIC: Open/Close Tag without properties
         if (regex_search(pos, Input.cend(), matches, IsBasicTag))
         {
            // Identify open/close
            bool opening = (pos[1] != '\\');
            wstring name = matches[1].str();
            
            // Identify type
            switch (TagType type = IdentifyTag(name))
            {
            // Author: Return author
            case TagType::Author:
               // Match [author](text)[/author]
               if (!regex_search(pos, Input.cend(), matches, IsAuthorDefition))
                  throw RichTextException(HERE, L"Invalid author tag");

               // Advance iterator.  Return author text
               pos += matches[0].length();
               return RichTag(TagType::Author, matches[1].str());

            // Title: Return title
            case TagType::Title:
               // Match [title](text)[/title]
               if (!regex_search(pos, Input.cend(), matches, IsTitleDefition))
                  throw RichTextException(HERE, L"Invalid title tag");

               // Advance iterator.  Return title text
               pos += matches[0].length();
               return RichTag(TagType::Title, matches[1].str());

            // Default: Advance iterator to ']' + return
            default:
               pos += matches[0].length();
               return RichTag(type, opening);
            }
         }
         // COMPLEX: Open tag with properties
         else
         {
            PropertyList props;

            // Match tag name, identify start/end of properties
            regex_search(pos, Input.cend(), matches, IsOpeningTag);

            // Match properties
            for (wsregex_iterator it(pos+matches[1].length(), pos+matches[0].length(), IsTagProperty), eof; it != eof; ++it)
               // Extract {name,value} from 2st/3rd match
               props.push_back( Property((*it)[1].str(), (*it)[2].str()) );
            
            // Advance Iterator + Return tag
            pos += matches[0].length();
            return RichTag(IdentifyTag(matches[1].str()), props);
         }
      }
      
      /// <summary>Extracts column information from a text tag</summary>
      /// <param name="tag">text tag</param>
      /// <exception cref="Logic::InvalidOperation">Not a 'text' tag</exception>
      /// <exception cref="Logic::Language::RichTextException">Invalid tag property</exception>
      void  StringParser::SetColumnInfo(const RichTag& tag)
      {
         // Ensure tag is 'text'
         if (tag.Type != TagType::Text)
            throw InvalidOperationException(HERE, GuiString(L"Cannot extract column info from a '%s' tag", GetString(tag.Type).c_str()) );

         // Extract properties direct into output
         for (const Property& p : tag.Properties)
         {
            // Column count
            if (p.first == L"cols")
            {
               // Verify
               auto cols = _ttoi(p.second.c_str());
               if (cols < 0 || cols > 3)
                  throw RichTextException(HERE, GuiString(L"Cannot arrange text in %d columns", cols));
               Output.Columns = static_cast<ColumnType>(cols);
            }
            // Column width
            else if (p.first == L"colwidth")
               Output.Width = _ttoi(p.second.c_str());
            // Column spacing
            else if (p.first == L"colspacing")
               Output.Width = _ttoi(p.second.c_str());
            else
               // Unrecognised
               throw RichTextException(HERE, GuiString(L"Invalid 'text' tag property '%s'", p.first.c_str()));
         }
      }

   }
}
