#pragma once
#include "RichEditEx.h"
#include "SuggestionList.h"
#include "CustomTooltip.h"
#include "ScriptDocument.h"
#include "Logic/ScriptParser.h"
#include "Logic/DescriptionLibrary.h"
#include "Logic/SyntaxLibrary.h"

/// <summary>Forward declaration</summary>
FORWARD_DECLARATION2(GUI,Documents,class ScriptDocument)

/// <summary>User interface controls</summary>
NAMESPACE_BEGIN2(GUI,Controls)

   
   /// <summary>Script editing control</summary>
   class ScriptEdit : public RichEditEx
   {
   protected:
      friend class SuggestionList;
      class LineTextIterator;

      // ------------------------ TYPES --------------------------
   protected:
      /// <summary>Defines whether suggestions visible</summary>
      enum class InputState : UINT { Normal, Suggestions };

      /// <summary>Device context containing same size font as ScriptEdit</summary>
      class FontDC : public CClientDC
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create DC containing same size font as script edit.</summary>
         /// <param name="wnd">script edit.</param>
         /// <param name="font">custom font.</param>
         FontDC(ScriptEdit* wnd, const wchar* font = nullptr) : CClientDC(wnd)
         {
            // Copy ScriptEdit font
            auto lf = PrefsLib.ScriptViewFont;
            if (font)
               StringCchCopy(lf.lfFaceName, LF_FACESIZE, font);
            Font.CreateFontIndirectW(&lf);

            // Set font
            OldFont = SelectObject(&Font);
         }
         ~FontDC()
         {
            SelectObject(OldFont);
         }
         
         // -------------------- REPRESENTATION ---------------------
      protected:
         CFont Font;
         CFont* OldFont;
      };

      /// <summary>Defines the logical co-ordinates of the line number gutter</summary>
      class GutterRect : public ClientRect
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create gutter rectangle, with space for four digits in Arial.</summary>
         /// <param name="wnd">The WND.</param>
         GutterRect(ScriptEdit* wnd) : ClientRect(wnd)
         {
            FontDC dc(wnd, L"Arial");
            right = dc.GetTextExtent(CString(L"0000")).cx;
         }
      };

      /// <summary>Defines the logical co-ordinates of a single line number in the gutter</summary>
      class LineRect : public GutterRect
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create a line number indicator rectangle</summary>
         /// <param name="wnd">script edit.</param>
         /// <param name="line">zero-based line number</param>
         /// <param name="height">height in pixels</param>
         LineRect(ScriptEdit* wnd, int line, int height) : GutterRect(wnd), Line(line), Height(height)
         {
            bottom = Height;
            OffsetRect(0, Line*Height);
         }

         // --------------------- PROPERTIES ------------------------
	  
         PROPERTY_GET(int,LineNumber,GetLineNumber);

         // ---------------------- ACCESSORS ------------------------	

         /// <summary>Get the 0-based line number</summary>
         int  GetLineNumber() const
         {
            return Line;
         }

         // ----------------------- MUTATORS ------------------------

         /// <summary>Advance to the next line</summary>
         void  Advance()
         {
            OffsetRect(0, Height);
            ++Line;
         }

         // -------------------- REPRESENTATION ---------------------
      protected:
         int Line;
         int Height;
      };

      /// <summary>Device context for drawing onto script edit</summary>
      class ScriptEditDC : public FontDC
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create script edit DC</summary>
         /// <param name="wnd">script edit.</param>
         /// <param name="font">custom font.</param>
         ScriptEditDC(ScriptEdit* wnd, const wchar* font = nullptr) : FontDC(wnd, font)
         {
            SCROLLINFO horz, vert;
      
            // Clip to client window
            ClientRect rc(wnd);
            IntersectClipRect(rc);

            // Get scroll-bar info
            wnd->GetScrollInfo(SB_HORZ, &horz);
            wnd->GetScrollInfo(SB_VERT, &vert);

            // Define drawing origin
            horz.nPos = (horz.nTrackPos ? horz.nTrackPos : horz.nPos);
            vert.nPos = (vert.nTrackPos ? vert.nTrackPos : vert.nPos);
            SetViewportOrg(static_cast<int>(-0.5f*horz.nPos), static_cast<int>(-0.5f*vert.nPos));

            // Set colour
            SetBkColor(PrefsLib.BackgroundColour);
            SetTextColor(PrefsLib.TextColour);
         }

         // --------------------- PROPERTIES ------------------------
	  
         // ---------------------- ACCESSORS ------------------------	

         /// <summary>Gets the indicator rectangle for a line</summary>
         /// <param name="line">Zero-based line number</param>
         /// <returns>Indicator rectangle in DEVICE co-ordinates</returns>
         LineRect GetLineRect(int line)
         {
            ScriptEdit* wnd = dynamic_cast<ScriptEdit*>(GetWindow());
            int height = 0;

            // Get line height
            if (wnd->GetLineCount() == 1)
            {
               // Calculate from character height (Isn't accurate for some reason)
               CharFormat cf(CFM_OFFSET|CFM_SIZE, NULL);
               wnd->GetDefaultCharFormat(cf);
               height = (cf.yHeight/10);  //TwipsToPixels(cf.yHeight, LOGPIXELSY);  [Should be Twips->Pixels, but /10 seems to work better..)
            }
            else
               // Calculate from character positions
               height = wnd->PosFromChar(wnd->LineIndex(1)).y - wnd->PosFromChar(wnd->LineIndex(0)).y;

            // Get line rect in device co-ordinates  
            LineRect rc(wnd, line, height); 
            LPtoDP(rc);
            return rc;
         }

         /// <summary>Convert pixels to twips.</summary>
         /// <param name="pixels">pixels.</param>
         /// <param name="direction">LOGPIXELSX or LOGPIXELSY</param>
         /// <returns>Twips</returns>
         /// <exception cref="ArgumentException">Invalid direction</exception>
         int PixelsToTwips(int pixels, int direction) const
         {
            if (direction != LOGPIXELSX && direction != LOGPIXELSY)
               throw ArgumentException(HERE, L"direction", L"Direction must be LOGPIXELSX or LOGPIXELSY");

            return pixels * 1440 / GetDeviceCaps(direction);
         }

         /// <summary>Convert twips to pixels.</summary>
         /// <param name="twips">twips</param>
         /// <param name="direction">LOGPIXELSX or LOGPIXELSY</param>
         /// <returns>Pixels</returns>
         /// <exception cref="ArgumentException">Invalid direction</exception>
         int TwipsToPixels(int twips, int direction) const
         {
            if (direction != LOGPIXELSX && direction != LOGPIXELSY)
               throw ArgumentException(HERE, L"direction", L"Direction must be LOGPIXELSX or LOGPIXELSY");

            return twips * GetDeviceCaps(direction) / 1440;
         }

         // ----------------------- MUTATORS ------------------------

         // -------------------- REPRESENTATION ---------------------
      };

      /// <summary>Proxy for manipulating line text</summary>
      class LineProxy
      {
         friend class LineTextIterator;
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Creates a line proxy</summary>
         /// <param name="edit">edit.</param>
         /// <param name="line">zero-based line number</param>
         LineProxy(ScriptEdit& edit, int line) : Edit(edit), LineNumber(line)
         {}
         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
		public:
         PROPERTY_GET_SET(wstring,Text,GetText,SetText);
         PROPERTY_GET(bool,Commented,GetCommented);
         PROPERTY_GET(int,End,GetEnd);
         PROPERTY_GET(int,Indent,GetIndent);
         PROPERTY_GET(int,Length,GetLength);
         PROPERTY_GET(int,Line,GetLine);
         PROPERTY_GET(bool,NOP,GetNOP);
         PROPERTY_GET(int,Start,GetStart);

         // ---------------------- ACCESSORS ------------------------			
      public:
         
         /// <summary>Gets whether line is commented</summary>
         /// <returns></returns>
         bool GetCommented() const
         {
            auto txt = GetText();
            // Check for '*' as first character
            int pos = txt.find_first_not_of(L' ');
            return pos != wstring::npos && txt[pos] == L'*';
         }

         /// <summary>Gets the amount of indentation.</summary>
         /// <returns></returns>
         int GetIndent() const
         {
            // Find first non-space
            auto txt = GetText();
            int pos = txt.find_first_not_of(L' ');

            // Return position / entire line
            return pos != wstring::npos ? pos : txt.length();
         }

         /// <summary>Gets the character index of the end of the line</summary>
         /// <returns></returns>
         int GetEnd() const
         {
            return Edit.GetLineEnd(LineNumber);
         }
         
         /// <summary>Gets the line length</summary>
         /// <returns></returns>
         int GetLength() const
         {
            return Edit.GetLineLength(LineNumber);
         }

         /// <summary>Gets the zero-based line number</summary>
         /// <returns></returns>
         int GetLine() const
         {
            return LineNumber;
         }

         /// <summary>Gets whether command is NOP</summary>
         /// <returns></returns>
         bool GetNOP() const
         {
            return Text.find_first_not_of(' ') == wstring::npos;
         }

         /// <summary>Gets the character index of the start of the line</summary>
         /// <returns></returns>
         int GetStart() const
         {
            return Edit.GetLineStart(LineNumber);
         }

         /// <summary>Gets the line text (Does not alter the edit selection)</summary>
         /// <returns></returns>
         wstring GetText() const
         {
            return Edit.GetLineText(LineNumber);
         }

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Sets the line text and redraws it (Alters the edit selection)</summary>
         /// <param name="str">Text</param>
         void SetText(const wstring& str)
         {
            SetText(str, true, true);
         }

         /// <summary>Sets the text (Alters the edit selection)</summary>
         /// <param name="str">text.</param>
         /// <param name="canUndo">Whether can be undone.</param>
         /// <param name="redraw">Whether to redraw.</param>
         void SetText(const wstring& str, bool canUndo, bool redraw)
         {
            Edit.FreezeWindow(true);
            Edit.SelectLine(LineNumber);
            Edit.ReplaceSel(str.c_str(), canUndo ? TRUE : FALSE);
            Edit.FreezeWindow(false, redraw);
         }
      private:
         /// <summary>Changes the line number.</summary>
         /// <param name="line">zero-based line number</param>
         void SetLine(int line)
         {
            LineNumber = line;
         }

         // -------------------- REPRESENTATION ---------------------
      private:
         ScriptEdit& Edit;
         int         LineNumber;
      };

      /// <summary>Bi-directional line text iterator</summary>
      class LineTextIterator : public std::iterator<std::bidirectional_iterator_tag, LineProxy>
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         LineTextIterator(ScriptEdit& edit, int line) : Line(edit,line), Edit(edit), LineNumber(line)
         {}
         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
			
         // ---------------------- ACCESSORS ------------------------			
      public:
         const LineProxy& operator*() const    { return Line;  }
         const LineProxy* operator->() const   { return &Line; }
         LineProxy& operator*()                { return Line;  }
         LineProxy* operator->()               { return &Line; }

         bool operator==(const LineTextIterator& r) const  { return Edit.m_hWnd==r.Edit.m_hWnd && LineNumber==r.LineNumber; }
         bool operator!=(const LineTextIterator& r) const  { return Edit.m_hWnd!=r.Edit.m_hWnd || LineNumber!=r.LineNumber; }
         bool operator<(const LineTextIterator& r) const   { return LineNumber < r.LineNumber;  }
         bool operator>(const LineTextIterator& r) const   { return LineNumber > r.LineNumber;  }
         bool operator<=(const LineTextIterator& r) const  { return LineNumber <= r.LineNumber; }
         bool operator>=(const LineTextIterator& r) const  { return LineNumber >= r.LineNumber; }
         /*distance_type operator-(const LineTextIterator& r) const  { return LineNumber - r.LineNumber; }
         distance_type operator+(const LineTextIterator& r) const  { return LineNumber + r.LineNumber; }*/

         // ----------------------- MUTATORS ------------------------
      public:
         LineTextIterator& operator++() 
         { 
            Line.SetLine(++LineNumber); 
            return *this; 
         }

         LineTextIterator& operator--()
         { 
            Line.SetLine(--LineNumber); 
            return *this; 
         }

         LineTextIterator operator++(int) 
         {
            LineTextIterator tmp(*this); 
            operator++(); 
            return tmp;
         }

         LineTextIterator operator--(int) 
         {
            LineTextIterator tmp(*this); 
            operator--(); 
            return tmp;
         }

         // -------------------- REPRESENTATION ---------------------
      private:
         LineProxy   Line;
         ScriptEdit& Edit;
         int         LineNumber;
      };

      /// <summary>Tooltip data for a script command</summary>
      class CommandTooltipData : public CustomTooltip::TooltipData
      {
      public:
         /// <summary>Create script command tooltip</summary>
         /// <param name="lineText">The line text.</param>
         /// <param name="ver">The version</param>
         CommandTooltipData(wstring lineText, GameVersion ver) : TooltipData(IDR_GAME_OBJECTS, 32)
         {
            try
            {
               // Identify command
               auto cmd = ScriptParser::Parse(lineText, ver);

               // Unknown: Display nothing
               if (cmd == CommandSyntax::Unrecognised)
                  ResetTo(CustomTooltip::NoTooltip);
               else
               {  
                  // Lookup description 
                  DescriptionSource = DescriptionLib.Commands.Find(cmd);
                  LabelSource = cmd.GetRichText(true);

                  // TODO: Supply icon
               }
            }
            // No description: Hasn't been documented
            catch (ExceptionBase& e) {
               Console.Log(HERE, e);
               ResetTo(CustomTooltip::NoDocumentationCmd);
            }
         }
      };

      /// <summary>Tooltip data for a label reference</summary>
      class LabelTooltipData : public CustomTooltip::TooltipData
      {
      public:
         /// <summary>Creates label reference tooltip data.</summary>
         /// <param name="edit">script edit.</param>
         /// <param name="label">label name WITHOUT operator</param>
         LabelTooltipData(ScriptEdit& edit, wstring label) : TooltipData(0, 0)
         {
            try
            {
               // Label: declaration line text
               UINT declaration = edit.Document->Script.Labels[label].LineNumber-1;
               LabelSource = edit.GetLineText(declaration);
            
               // Prepare
               list<wstring> desc;
               DescriptionSource.clear();
               
               // Description: preceeding line comments
               for (auto line = edit.begin(declaration-1); line >= edit.begin(0) && line->Commented; --line)
                  DescriptionSource.insert(0, line->Text + L"\r\n");
            }
            catch (ExceptionBase& e) {
               Console.Log(HERE, e);
               ResetTo(CustomTooltip::NoTooltip);
            }
         }
      };
      
      /// <summary>Tooltip data for a script-object</summary>
      class ScriptObjectTooltipData : public CustomTooltip::TooltipData
      {
      public:
         /// <summary>Creates script object tooltip data.</summary>
         /// <param name="name">object name</param>
         ScriptObjectTooltipData(wstring name) : TooltipData(IDR_SCRIPT_OBJECTS, 32)
         {
            try
            {
               // Identify object
               auto obj = ScriptObjectLib.Find(name);

               // Lookup description 
               DescriptionSource = DescriptionLib.Constants.Find(obj);
               LabelSource = GuiString(L"[center][b]%s[/b][/center]", obj.Text.c_str());

               // TODO: Supply icon
            }
            // No description: Hasn't been documented
            catch (ExceptionBase& e) {
               Console.Log(HERE, e);
               ResetTo(CustomTooltip::NoDocumentationObj);
            }
         }
      };

      // --------------------- CONSTRUCTION ----------------------
   public:
      ScriptEdit();
      virtual ~ScriptEdit();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNAMIC(ScriptEdit)
      DECLARE_MESSAGE_MAP()

   public:
      static const wchar*  GetString(Suggestion& s);

   protected:
      static const UINT    COMPILE_TIMER;

      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
   public:
      void      PasteFormat(UINT nClipFormat) override;

   protected:
      bool       HasDocument() const;
      Suggestion IdentifySuggestion(wchar ch) const;
      bool       MatchSuggestionType(Compiler::TokenType t) const;
      
      // ----------------------- MUTATORS ------------------------
   public:
      void   CommentSelection();
      void   IndentSelection(bool indent);
      void   Initialize(ScriptDocument* doc);
      void   SetRtf(const string& rtf) override;

   protected:
      LineTextIterator begin(int line = 0);
      LineTextIterator sbegin();
      LineTextIterator end();
      LineTextIterator send();

      void   CloseSuggestions();
      void   FormatToken(UINT offset, const TokenBase& t, CharFormat& cf);
      void   InsertSuggestion();
      void   RefreshGutter();
      void   SetCompilerTimer(bool set);
      void   SetGutterWidth(UINT twips);
      void   ShowSuggestions();
      void   UpdateHighlighting(int first, int last);
      void   UpdateSuggestions();
      
      handler void OnBackgroundCompile();
      afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
      handler void OnCharNewLine();
      handler void OnCharTab(bool shift);
      afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
      afx_msg void OnInputMessage(NMHDR *pNMHDR, LRESULT *pResult) override;
      afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
      afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
      afx_msg void OnKillFocus(CWnd* pNewWnd);
      afx_msg void OnPaint();
      handler void OnRequestTooltip(CustomTooltip::TooltipData* data) override;
      afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
      afx_msg void OnTextChange() override;
      afx_msg void OnTimer(UINT_PTR nIDEvent);
      afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	  
      // -------------------- REPRESENTATION ---------------------
   public:
      SimpleEvent     CompileComplete;

   protected:
      InputState      State;
      Suggestion      SuggestionType;
      SuggestionList  SuggestionsList;
      ScriptDocument* Document;
};
   

NAMESPACE_END2(GUI,Controls)
