#pragma once

#include "Common.h"
#include <regex>
#include "StringResolver.h"      // RegEx Exception

namespace Logic
{
   
   /// <summary></summary>
   class MatchData
   {
      // ------------------------ TYPES --------------------------
   private:

      // --------------------- CONSTRUCTION ----------------------
   public:
      /// <summary>Create empty match data</summary>
      /// <param name="search">search term or expression.</param>
      /// <param name="replace">replacement term or expression</param>
      /// <param name="matchCase">match case flag.</param>
      /// <param name="matchWord">match whole word flag.</param>
      /// <param name="regEx">Whether terms are strings or regEx</param>
      /// <exception cref="Logic::Language::RegularExpressionException">Error in expression</exception>
      MatchData(const wstring& search, const wstring& replace, bool matchCase, bool matchWord, bool regEx)
         : SearchTerm(search), 
           ReplaceTerm(replace), 
           IsMatched(false), 
           Location({0,0}), 
           LineNumber(0),
           MatchCase(regEx ? matchCase : false), 
           MatchWord(regEx ? matchWord : false),
           UseRegEx(regEx),
           RegEx(GetSafeRegEx(regEx ? search : L""))
      {
         /*if (MatchWord && UseRegEx)
            throw NotSupportedException(HERE, L"Can't use match whole word only and regular expressions together. Use \\b.");*/
      }

      NO_MOVE(MatchData);	      // Immutable
      NO_COPY_ASSIGN(MatchData);	// Immutable

      // ------------------------ STATIC -------------------------
   private:
      /// <summary>Creates a regular expression.</summary>
      /// <param name="txt">expression</param>
      /// <returns></returns>
      /// <exception cref="Logic::Language::RegularExpressionException">Error in expression</exception>
      wregex  GetSafeRegEx(const wstring& txt)
      {
         try {
            return txt.length() ? wregex(txt) : wregex();
         }
         catch (regex_error& e) {
            throw RegularExpressionException(HERE, e);
         }
      }

      // --------------------- PROPERTIES ------------------------

      // ---------------------- ACCESSORS ------------------------	

      // ----------------------- MUTATORS ------------------------
   public:
      /// <summary>Clears the match.</summary>
      void  Clear()
      {
         FullPath = L"";
         IsMatched = false;
         Location = {0,0};
         LineText.empty();
         LineNumber = 0;
      }

      /// <summary>Finds the next match</summary>
      /// <param name="input">input text</param>
      /// <param name="lineBreak">line break character</param>
      /// <returns></returns>
      bool  FindNext(const GuiString& input, wchar lineBreak)
      {
         int pos = GuiString::npos, 
             len = 0;

         // Validate position in case text has shrunk
         if ((UINT)Location.cpMax < input.length())
         {
            // RegEx: Find from last match using regEx
            if (UseRegEx)
            {
               wsmatch matches;
               if (regex_search(input.cbegin()+Location.cpMax, input.cend(), matches, RegEx, regex_constants::match_default))
               {
                  pos = matches[0].first - input.cbegin();
                  len = matches[0].length();
               }
            }
            // Basic: Find from last match using wstring
            else
            {
               pos = input.Find(SearchTerm, Location.cpMax, MatchCase);
               len = SearchTerm.length();
            }
         }
         
         // Set/Clear line text, line#, location
         if (pos != GuiString::npos)
         {
            int line = count_if(input.begin(), input.begin()+pos, [lineBreak](wchar ch) {return ch==lineBreak;} );
            SetMatch(pos, len, line+1);
         }
         else
            Clear();
         
         // Return result
         return IsMatched;
      }

      /// <summary>Replaces the last match, if any</summary>
      /// <param name="input">input text</param>
      /// <param name="lineBreak">line break character</param>
      /// <returns></returns>
      bool  Replace(GuiString& input, wchar lineBreak)
      {
         int pos = GuiString::npos, 
             len = 0;

         // Validate position in case text has shrunk
         if ((UINT)Location.cpMax < input.length())
         {
            // RegEx: Find from last match using regEx
            if (UseRegEx)
            {
               wsmatch matches;
               // Verify last match
               if (regex_match(input.cbegin()+Location.cpMin, input.cbegin()+Location.cpMax, matches, RegEx))
               {
                  // Format replacement
                  auto r = matches.format(ReplaceTerm);

                  // Set location of replacement
                  pos = Location.cpMin;
                  len = r.length();

                  // Replace text
                  input.replace(pos, len, r);
               }
            }
            // Basic: Find from last match using wstring
            else
            {
               // Verify last match
               if (input.Find(SearchTerm, Location.cpMin, MatchCase) == Location.cpMin)
               {
                  // Set location of replacement
                  pos = Location.cpMin;
                  len = ReplaceTerm.length();

                  // Replace text
                  input.replace(pos, len, ReplaceTerm);
               }
            }
         }
         
         // Replaced: Update location 
         if (pos != GuiString::npos)
         {
            Location = {pos, pos+len};
            return true;
         }
         
         // Not Replaced: 
         return false;
      }

   private:
      /// <summary>Sets the match.</summary>
      /// <param name="start">start index.</param>
      /// <param name="length">length.</param>
      /// <param name="txt">text.</param>
      /// <param name="txt">1-based line number.</param>
      void  SetMatch(int start, int length, int line)
      {
         Location = {start, start+length};
         IsMatched = true;
         LineNumber = line;
         //Text = txt;
      }

      // -------------------- REPRESENTATION ---------------------
   public:
      bool        IsMatched;
      CHARRANGE   Location;
      IO::Path    FullPath;
      int         LineNumber;
      wstring     LineText;

      const wstring  SearchTerm,
                     ReplaceTerm;
      const bool     MatchCase,
                     MatchWord,
                     UseRegEx;
      const wregex   RegEx;
   };

   
}

using namespace Logic;