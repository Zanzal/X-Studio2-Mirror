#include "stdafx.h"
#include "ScriptEdit.h"
#include "Logic/CommandLexer.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Controls)

   // --------------------------------- CONSTANTS ----------------------------------

   /// <summary>Background compiler timer ID</summary>
   const UINT  ScriptEdit::COMPILE_TIMER = 42;

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNAMIC(ScriptEdit, CRichEditCtrl)

   BEGIN_MESSAGE_MAP(ScriptEdit, CRichEditCtrl)
      ON_WM_TIMER()
      ON_WM_KILLFOCUS()
      ON_WM_CHAR()
      ON_WM_KEYDOWN()
      ON_WM_PAINT()
      ON_WM_HSCROLL()
      ON_WM_VSCROLL()
      ON_WM_KEYUP()
      ON_WM_SETFOCUS()
      ON_CONTROL_REFLECT(EN_CHANGE, &ScriptEdit::OnTextChange)
      ON_NOTIFY_REFLECT(EN_MSGFILTER, &ScriptEdit::OnInputMessage)
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   ScriptEdit::ScriptEdit() : Document(nullptr), SuggestionType(Suggestion::None)
   {
   }

   ScriptEdit::~ScriptEdit()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   /// <summary>Get suggestion type name</summary>
   const wchar*  ScriptEdit::GetString(Suggestion& s) 
   {
      static const wchar* str[] = { L"None", L"Variable", L"Command", L"GameObject", L"ScriptObject", L"Label" };
      return str[(UINT)s];
   }

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Toggles comment on the selected lines.</summary>
   void  ScriptEdit::CommentSelection()
   {
      GuiString  output;

      // Get first/last line of selection
      LineTextIterator first = sbegin(), last = send();
      
      // Select entire block of lines 
      SetSel(first->Start, last->End);
      FreezeWindow(true);

      // Choose state based on first line
      bool comment = !first->Commented;

      // Get selected lines
      int length = 0;
      for (auto it = first; it <= last; ++it)
      {
         GuiString txt = it->Text;

         // Comment: Add '*' to start of each line
         if (comment && !it->Commented && !it->NOP)
            txt.insert(txt.find_first_not_of(L' '), L"* ");

         // Uncomment: Remove '*' from start of each line
         else if (!comment && it->Commented)
         {
            // Erase spaces trailing '*'
            auto end = txt.find_first_not_of(L' ', txt.find(L'*')+1);
            if (end != GuiString::npos)
               txt.erase(txt.find(L'*'), end);
            else
               txt.erase(txt.find(L'*'));
         }

         // Add to output
         length += txt.length();
         output += txt;

         // CRLF  [except last line]
         if (it != last)
         {
            output += L"\r";
            length++;
         }
      }

      // Replace existing selection
      ReplaceSel(output.c_str(), TRUE);

      // Unfreeze window
      FreezeWindow(false);
      SetSel(first->Start, last->End);
   }
   
   
   /// <summary>Indents or outdents the selected lines.</summary>
   /// <param name="indent">True to indent, false to outdent.</param>
   void  ScriptEdit::IndentSelection(bool indent)
   {
      GuiString  output;

      // Get first/last line of selection
      LineTextIterator first = sbegin(), last = send();
      
      // Select entire block of lines 
      SetSel(first->Start, last->End);
      FreezeWindow(true);

      // Get selected lines
      int length = 0;
      for (auto it = first; it <= last; ++it)
      {
         GuiString txt = it->Text;

         // In/Outdent by adding/removing tab from start of each line
         if (indent)
            txt.insert(0, L"   ");
         else if (txt.Left(3) == L"   ")
            txt.erase(0, 3);

         // Add to output
         length += txt.length();
         output += txt;

         // CRLF  [except last line]
         if (it != last)
         {
            output += L"\r";
            length++;
         }
      }

      // Replace existing selection
      ReplaceSel(output.c_str(), TRUE);

      // Unfreeze window
      FreezeWindow(false);
      SetSel(first->Start, last->End);
   }


   /// <summary>Initializes the control</summary>
   /// <param name="doc">The document.</param>
   /// <exception cref="Logic::ArgumentNullException">document is null</exception>
   /// <exception cref="Logic::Win32Exception">Unable to retrieve COM pointers</exception>
   void  ScriptEdit::Initialize(ScriptDocument* doc)
   {
      REQUIRED(doc);

      // Set document
      Document = doc;

      // Initialize
      RichEditEx::Initialize(RGB(0,0,0));
   }

   /// <summary>Replace entire contents with RTF.</summary>
   /// <param name="rtf">The RTF.</param>
   /// <exception cref="Logic::InvalidOperationException">No document attached</exception>
   /// <exception cref="Logic::Win32Exception">Unable to set text</exception>
   void ScriptEdit::SetRtf(const string& rtf)
   {
      // Require document
      if (Document == nullptr)
         throw InvalidOperationException(HERE, L"Must attach document prior to displaying text");

      // Set RTF
      RichEditEx::SetRtf(rtf);

      // Adjust gutter
      SetGutterWidth(GutterRect(this).Width()+10);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Get line iterator for the specified line.</summary>
   /// <param name="line">Zero-based line number</param>
   /// <returns></returns>
   ScriptEdit::LineTextIterator  ScriptEdit::begin(int line)
   { 
      return LineTextIterator(*this, line);            
   }
   
   /// <summary>Get line iterator for the first line in the text selection, if any, otherwise the line containing the caret</summary>
   /// <returns></returns>
   ScriptEdit::LineTextIterator  ScriptEdit::sbegin() 
   { 
      return LineTextIterator(*this, HasSelection() ? LineFromChar(GetSelection().cpMin) : LineFromChar(-1)); 
   }
   
   /// <summary>Get line iterator for the last line.</summary>
   /// <returns></returns>
   ScriptEdit::LineTextIterator  ScriptEdit::end() 
   { 
      return LineTextIterator(*this, GetLineCount()-1); 
   }

   /// <summary>Get line iterator for the last line in the text selection, if any, otherwise the line containing the caret</summary>
   /// <returns></returns>
   ScriptEdit::LineTextIterator  ScriptEdit::send() 
   { 
      return LineTextIterator(*this, HasSelection() ? LineFromChar(GetSelection().cpMax) : LineFromChar(-1)); 
   }

   /// <summary>Closes the suggestion list</summary>
   void ScriptEdit::CloseSuggestions()
   {
      // Ensure exists
      if (SuggestionsList.GetSafeHwnd() == nullptr)
         throw InvalidOperationException(HERE, L"suggestion list does not exist");

      // Revert state
      State = InputState::Normal;

      // Destroy
      if (!SuggestionsList.DestroyWindow())
         throw Win32Exception(HERE, L"Unable to destroy suggestion list");
   }
   
   /// <summary>Selects and formats a token.</summary>
   /// <param name="offset">The character index of the line</param>
   /// <param name="t">The token</param>
   /// <param name="cf">The formatting characteristics</param>
   void  ScriptEdit::FormatToken(UINT offset, const TokenBase& t, CharFormat& cf)
   {
      SetSel(offset+t.Start, offset+t.End);
      SetSelectionCharFormat(cf);
   }

   /// <summary>Determines whether document is connected</summary>
   /// <returns></returns>
   bool  ScriptEdit::HasDocument() const
   {
      return Document != nullptr;
   }
   
   /// <summary>Identifies the type of suggestion to display in response to a character press</summary>
   /// <param name="ch">The character just typed</param>
   /// <returns></returns>
   Suggestion  ScriptEdit::IdentifySuggestion(wchar ch) const
   {
      switch (ch)
      {
      case '$': return Suggestion::Variable;
      case '{': return Suggestion::GameObject;
      case '[': return Suggestion::ScriptObject;
      case ' ': 
       {  
         CommandLexer lex(GetLineText(-1));
         TokenIterator pos = lex.begin();
         
         // Match: GoSub|Goto ' ' <caret>  { caret following single space after goto/gosub }
         if ((lex.Match(pos, TokenType::Keyword, L"gosub") || lex.Match(pos, TokenType::Keyword, L"goto"))
             && GetCaretIndex() == (pos-1)->End+1)
            return Suggestion::Label;
         break;
       }
      default:  
       { 
         // Ensure character is alpha-numeric
         if (!iswalpha(ch))
            return Suggestion::None;

         // Lex current line
         CommandLexer lex(GetLineText(-1));
         TokenIterator pos = lex.begin();

         // Rule: char <caret>  { first token is text. caret on 2nd letter }
         if (lex.Match(pos, TokenType::Text) && GetCaretIndex() == (pos-1)->Start+1)
            return Suggestion::Command;

         // Rule: variable '=' char <caret>  { 3 tokens: variable, equals, text. caret on 2nd letter }
         if (lex.Match(pos=lex.begin(), TokenType::Variable) && lex.Match(pos, TokenType::BinaryOp, L"=") && lex.Match(pos, TokenType::Text)
             && GetCaretIndex() == (pos-1)->Start+1)
             return Suggestion::Command;

         
         // Rule: (variable '=')? constant/variable/null '->' char <caret>
         // match (variable '=')? while accounting for  variable '->'
         if (lex.Match(pos=lex.begin(), TokenType::Variable))
         {
            // (variable '=')?   
            if (lex.Match(pos, TokenType::BinaryOp, L"="))
            {}
            // Reset position if (variable '->')
            else if (lex.Match(pos, TokenType::BinaryOp, L"->"))
               pos=lex.begin();
            else
               return Suggestion::None;
         }
         
         // constant/variable/null 
         if (lex.Match(pos, TokenType::ScriptObject) || lex.Match(pos, TokenType::Variable) || lex.Match(pos, TokenType::Null))
            // '->' char <caret>  { caret on 2nd letter }
            if (lex.Match(pos, TokenType::BinaryOp, L"->") && lex.Match(pos, TokenType::Text) && GetCaretIndex() == (pos-1)->Start+1)
               return Suggestion::Command;
       }
      }

      return Suggestion::None;
   }

   /// <summary>Inserts the current suggestion and closes the list</summary>
   void ScriptEdit::InsertSuggestion()
   {
      // Ensure exists
      if (SuggestionsList.GetSafeHwnd() == nullptr)
         throw InvalidOperationException(HERE, L"suggestion list does not exist");

      // DEBUG:
      Console << L"Inserting suggestion: " << Cons::Yellow << SuggestionsList.GetSelected() << ENDL;

      // Find token at caret
      CommandLexer lex(GetLineText(-1));
      TokenIterator pos = lex.Find(GetCaretIndex());

      // Replace token with suggestion
      if (pos != lex.end())
      {
         SetSel(LineIndex(-1)+pos->Start, LineIndex(-1)+pos->End);
         ReplaceSel(SuggestionsList.GetSelected().c_str(), TRUE);
      }
      else 
         // Error: Failed to identify token
         Console << Cons::Error << "Cannot find suggestion token: " << Cons::White << " caret at " << GetCaretIndex() << " : " << Cons::Yellow << GetLineText(-1) << ENDL;

      // Close
      CloseSuggestions();
   }
   
   /// <summary>Compiles script to highlight errors</summary>
   void ScriptEdit::OnBackgroundCompile()
   {
      CWaitCursor c;

      // Stop compiler timer
      SetCompilerTimer(false);
        
      // Freeze window
      SuspendUndo(true);
      FreezeWindow(true);

      try 
      { 
         // Feedback
         Console << Cons::Heading << "Background compiling " << Document->GetFullPath() << ENDL;

         // Parse script 
         ScriptParser parser(Document->Script, GetLines(), Document->Script.Game);
         
         // DEBUG:
         /*if (!parser.Errors.empty())
            parser.Print();*/
         Console << L"Background compiler found: " << parser.Errors.size() << L" errors" << ENDL;

         // Define error underline
         CharFormat cf(CFM_COLOR | CFM_UNDERLINE | CFM_UNDERLINETYPE, CFE_UNDERLINE);
         cf.bUnderlineType = CFU_UNDERLINEWAVE;
         cf.bUnderlineColor = 0x02;     //Undocumented underline colour
         cf.crTextColor = RGB(255,0,0);

         // Underline all errors
         for (const auto& err : parser.Errors)
         {
            FormatToken(LineIndex(err.Line-1), err, cf);
            Console << err.Line << L": " << Cons::Yellow << err.Message << Cons::White << L" : " << err.Text << ENDL;
         }

         // Raise 'Compile Complete'
         CompileComplete.Raise();
      }
      catch (std::exception& e) 
      { 
         Console.Log(HERE, e); 
      }

      // UnFreeze window
      FreezeWindow(false);
      SuspendUndo(false);
   }


   /// <summary>Shows/updates the suggestion list in response to character input</summary>
   /// <param name="nChar">The character.</param>
   /// <param name="nRepCnt">The repeat count.</param>
   /// <param name="nFlags">The flags.</param>
   void ScriptEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
   {
      // Process character
      __super::OnChar(nChar, nRepCnt, nFlags);

      try
      {
         // Update suggestions
         if (State == InputState::Suggestions)
            UpdateSuggestions();

         // display suggestions if appropriate
         else if ((SuggestionType = IdentifySuggestion(nChar)) != Suggestion::None)
         {
            //Console << L"Identified " << GetString(SuggestionType) << L" from " << (wchar)nChar << ENDL;
            ShowSuggestions();
         }
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e, GuiString(L"Unable to process '%c' character", (wchar)nChar)); 
      }
   }
   
   /// <summary>Called when ENTER is pressed.</summary>
   void ScriptEdit::OnCharNewLine()
   {
      //GroupUndo(true);  Not implemented

      // Insert newline
      ReplaceSel(L"\n", TRUE);

      // Get line now preceeding the caret
      LineProxy line(*this, LineFromChar(-1) - 1);

      // Insert similar amount of indentation
      ReplaceSel(Indent(line.Indent).c_str(), TRUE);

      //GroupUndo(false);  Not implemented
   }

   /// <summary>Replaces hard tabs with spaces.</summary>
   /// <param name="shift">whether SHIFT key is pressed</param>
   void ScriptEdit::OnCharTab(bool shift)
   {
      // No selection: Insert spaces
      if (!HasSelection())
         ReplaceSel(L"    ", TRUE);
      
      // Indent/Outdent
      else 
         IndentSelection(!shift);
   }

   /// <summary>Refreshes the line numbers after a scroll</summary>
   void ScriptEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
   {
      // Redraw the line numbers after a scroll or drag
      if (nSBCode == SB_ENDSCROLL)
         RefreshGutter();

      // Reset tooltip
      __super::OnHScroll(nSBCode, nPos, pScrollBar);
   }
   
   /// <summary>Blocks or forwards certain keys used in suggestion display</summary>
   /// <param name="pNMHDR">The notify header</param>
   /// <param name="pResult">message result</param>
   void ScriptEdit::OnInputMessage(NMHDR *pNMHDR, LRESULT *pResult)
   {
      static const UINT ALLOW_INPUT = 0, BLOCK_INPUT = 1;

      // Get character
      MSGFILTER *pFilter = reinterpret_cast<MSGFILTER *>(pNMHDR);
      wchar chr = pFilter->wParam;
   
      // Allow input by default
      *pResult = ALLOW_INPUT;

      try
      {
         if (State == InputState::Suggestions)
            switch (pFilter->msg)
            {
            case WM_CHAR:
               switch (chr)
               {
               // TAB: Insert suggestion.  Prevent focus switch
               case VK_TAB:
                  InsertSuggestion();
                  *pResult = BLOCK_INPUT;
                  break;
            
               // ESCAPE: Close suggestions
               case VK_ESCAPE:
                  CloseSuggestions();
                  *pResult = BLOCK_INPUT;
                  break;
               }
               break;

            // NAVIGATION: Update suggestions / Forward message
            case WM_KEYDOWN:
               switch (chr)
               {
               // TAB: Insert suggestion
               case VK_TAB:
                  InsertSuggestion();
                  *pResult = BLOCK_INPUT;
                  break;

               // ESCAPE: Close suggestions
               case VK_ESCAPE:
                  CloseSuggestions();
                  *pResult = BLOCK_INPUT;
                  break;

               // UP/DOWN/PAGEUP/PAGEDOWN: Navigate suggestions list
               case VK_UP:    
               case VK_DOWN:
               case VK_PRIOR: 
               case VK_NEXT:  
                  SuggestionsList.SendMessage(pFilter->msg, pFilter->wParam, pFilter->lParam);
                  *pResult = BLOCK_INPUT;
                  return;
               }
               break;

            // CLICK: Close Suggestions
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
               CloseSuggestions();
               break;

            // WHEEL: Forward to list
            case WM_MOUSEWHEEL:
               SuggestionsList.SendMessage(pFilter->msg, pFilter->wParam, pFilter->lParam);
               *pResult = BLOCK_INPUT;
               break;
            }
      }
      catch (ExceptionBase& e) { 
         Console.Log(HERE, e, GuiString(L"Unable to process input filter: message=%d wparam=%d lparam=%d char='%c'",pFilter->msg, pFilter->wParam, pFilter->lParam, chr) ); 
      }
   }

   /// <summary>Updates the suggestion list in response to caret movement</summary>
   /// <param name="nChar">The character.</param>
   /// <param name="nRepCnt">The repeat count.</param>
   /// <param name="nFlags">The flags.</param>
   void ScriptEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
   {
      // [Shift]+Tab: Invoke indentation handler   (exclude 'switch document' ctrl+Tab)
      if (nChar == VK_TAB && !HIBYTE(GetKeyState(VK_CONTROL)))
         OnCharTab(HIBYTE(GetKeyState(VK_SHIFT)) != 0);

      // Enter: Invoke newline indentation handler
      else if (nChar == VK_RETURN)
         OnCharNewLine();

      else  // Pass to RichEdit
         __super::OnKeyDown(nChar, nRepCnt, nFlags);
      
      // Suggestions: Update in response to caret movement
      if (State == InputState::Suggestions)
         try 
         {  
            switch (nChar)
            {
            // TAB/LEFT/RIGHT/HOME/END/DELETE/BACKSPACE: Update current match
            case VK_TAB:
            case VK_LEFT:
            case VK_RIGHT:
            case VK_HOME:
            case VK_END:
            case VK_DELETE:
            case VK_BACK:
               UpdateSuggestions();
               break;
            }
         } 
         catch (ExceptionBase& e) {
            Console.Log(HERE, e, GuiString(L"Unable to process '%d' key (char '%c')", nChar, (wchar)nChar)); 
         }
   }
   
   /// <summary>Not used</summary>
   /// <param name="nChar">The character.</param>
   /// <param name="nRepCnt">The repeat count.</param>
   /// <param name="nFlags">The flags.</param>
   void ScriptEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
   {
      __super::OnKeyUp(nChar, nRepCnt, nFlags);

      // Enter: Invoke newline indentation handler
      /*if (nChar == VK_RETURN)
         OnCharNewLine();*/
   }

   /// <summary>Closes the suggestion when focus lost</summary>
   /// <param name="pNewWnd">New window.</param>
   void ScriptEdit::OnKillFocus(CWnd* pNewWnd)
   {
      try
      {
         // Close suggestions, unless they're gaining focus
         if (State == InputState::Suggestions && (!pNewWnd || SuggestionsList != *pNewWnd))
            CloseSuggestions();
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e); 
      }

      // Deactivate tooltip
      __super::OnKillFocus(pNewWnd);
   }
   
   /// <summary>Draws the line number gutter</summary>
   void ScriptEdit::OnPaint()
   {
      // Paint window
      __super::OnPaint();

      // Get drawing dc
      ScriptEditDC dc(this);

      // Get first line number rectangle
      LineRect rect = dc.GetLineRect(GetFirstVisibleLine());

      // Draw all visible line numbers
      while (dc.RectVisible(rect))
      {
         // Draw 1-based line number
         auto s = GuiString(L"%d", 1+rect.LineNumber);
         dc.DrawText(s.c_str(), s.length(), rect, DT_RIGHT);

         // Move to next line
         rect.Advance();
      }
   
   }
   

   /// <summary>Supply tooltip data</summary>
   /// <param name="data">The data.</param>
   void ScriptEdit::OnRequestTooltip(CustomTooltip::TooltipData* data)
   {
      // Initialise
      auto cursor = GetCursorLocation();
      auto text = GetLineText(cursor.y);
      
      // Find token beneath cursor
      CommandLexer lex(text);
      auto tok = lex.Tokens.Find(cursor.x);
      
      // None: show nothing
      if (!tok)
         *data = CustomTooltip::NoTooltip;

      // Provide approriate data
      else switch (tok->Type)
      {
      case TokenType::Text:         data->ResetTo(CommandTooltipData(text, Document->Script.Game));   break;
      case TokenType::Label:        data->ResetTo(LabelTooltipData(*this, tok->ValueText));           break;
      case TokenType::ScriptObject: data->ResetTo(ScriptObjectTooltipData(tok->ValueText));           break;
      default:                      data->ResetTo(CustomTooltip::NoTooltip);                      break;
      }  
   }
   
   /// <summary>Performs syntax colouring on the current line</summary>
   void ScriptEdit::OnTextChange()
   {
      // Set/Reset background compiler timer
      SetCompilerTimer(true);

      // Update current line
      UpdateHighlighting(-1, -1);

      // Reset tooltip
      __super::OnTextChange();
   }
   
   /// <summary>Compiles the current text</summary>
   /// <param name="nIDEvent">The timer identifier.</param>
   void ScriptEdit::OnTimer(UINT_PTR nIDEvent)
   {
      // Background compiler
      if (nIDEvent == COMPILE_TIMER)
         OnBackgroundCompile();

      // Used by RichEdit
      __super::OnTimer(nIDEvent);
   }
   
   /// <summary>Refreshes the line numbers after a scroll</summary>
   void ScriptEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
   {
      // Redraw the line numbers after a scroll or drag
      if (nSBCode == SB_ENDSCROLL)
         RefreshGutter();

      // Reset Tooltip + Scroll
      __super::OnVScroll(nSBCode, nPos, pScrollBar);
   }

   /// <summary>Matches a token type against the current suggestion type</summary>
   /// <param name="t">The t.</param>
   /// <returns></returns>
   bool ScriptEdit::MatchSuggestionType(Compiler::TokenType t) const
   {
      switch (SuggestionType)
      {
      case Suggestion::Variable:     return t == TokenType::Variable;
      case Suggestion::GameObject:   return t == TokenType::GameObject;
      case Suggestion::ScriptObject: return t == TokenType::ScriptObject;
      case Suggestion::Label:        return t == TokenType::Label;
      case Suggestion::Command:      return t == TokenType::Text;
      }

      return false;
   }
   
   /// <summary>Pastes text from clipboard.</summary>
   /// <param name="nClipFormat">The clipboard format.</param>
   void  ScriptEdit::PasteFormat(UINT nClipFormat)
   {
      auto prevLine = LineFromChar(-1);

      // Paste
      if (CanPaste(nClipFormat))
         PasteSpecial(nClipFormat);

      // DEBUG:
      //Console << "inital line=" << prevLine << " newLine=" << LineFromChar(-1) << ENDL;

      // Highlight pasted text
      UpdateHighlighting(prevLine, LineFromChar(-1));
   }

   /// <summary>Invalidates the line number gutter.</summary>
   void ScriptEdit::RefreshGutter()
   {
      GutterRect rc(this);
      rc.right += 20;
      InvalidateRect(rc, TRUE);
      UpdateWindow();
   }

   /// <summary>Sets/resets/cancels the compiler timer.</summary>
   /// <param name="set">True to set/reset, false to cancel</param>
   void ScriptEdit::SetCompilerTimer(bool set)
   {
      // Set/Reset background compiler timer
      if (set)
         SetTimer(COMPILE_TIMER, 1500, nullptr);
      else
         KillTimer(COMPILE_TIMER);
   }

   /// <summary>Sets the width of the line number gutter.</summary>
   /// <param name="width">width in pixels</param>
   void ScriptEdit::SetGutterWidth(UINT width)
   {
      ParaFormat pf(PFM_OFFSET);
      
      // Freeze+select text
      FreezeWindow(true);
      SetSel(0, -1);

      // Calculate width in twips
      ScriptEditDC dc(this);
      pf.dxOffset = dc.TwipsToPixels(width, LOGPIXELSX);

      // Set paragraph formatting for entire text
      SetParaFormat(pf);
      SetSel(0, 0);

      // Restore
      FreezeWindow(false);
   }

   /// <summary>Shows the suggestion list</summary>
   void ScriptEdit::ShowSuggestions()
   {
      // Ensure does not exist
      if (SuggestionsList.GetSafeHwnd() != nullptr)
         throw InvalidOperationException(HERE, L"suggestion list already exists");

      // Show list
      if (!SuggestionsList.Create(this, GetCharPos(GetSelection().cpMin), SuggestionType, &Document->Script)) 
         throw Win32Exception(HERE, L"Unable to create suggestion list");

      // Update state
      State = InputState::Suggestions;
   }

   
   /// <summary>Updates the highlighting.</summary>
   /// <param name="first">first zero-based line number</param>
   /// <param name="last">last zero-based line number.</param>
   void ScriptEdit::UpdateHighlighting(int first, int last)
   {
      // Freeze window
      SuspendUndo(true);
      FreezeWindow(true);

      try 
      {
         CharFormat cf(CFM_COLOR | CFM_UNDERLINE | CFM_UNDERLINETYPE, NULL);
         
         // Caret:
         if (first == -1)
            first = last = LineFromChar(-1);

         // Format lines
         for (int i = first; i <= last; i++)
         {
            UINT offset = LineIndex(i);

            // Lex current line
            CommandLexer lex(GetLineText(i));

            // Format tokens
            for (const auto& tok : lex.Tokens)
            {
               // Set colour
               switch (tok.Type)
               {
               case TokenType::Comment:      cf.crTextColor = RGB(128,128,128);  break;
               case TokenType::Label:        cf.crTextColor = RGB(255,0,255);    break;
               case TokenType::Null:
               case TokenType::Variable:     cf.crTextColor = RGB(0,255,0);      break;
               case TokenType::Keyword:      cf.crTextColor = RGB(0,0,255);      break;
               case TokenType::Number:  
               case TokenType::String:       cf.crTextColor = RGB(255,0,0);      break;
               case TokenType::ScriptObject: cf.crTextColor = RGB(255,255,0);    break;
               case TokenType::GameObject:   cf.crTextColor = RGB(0,255,255);    break;
               default:                      cf.crTextColor = RGB(255,255,255);  break;
               }

               // Set format
               FormatToken(offset, tok, cf);
            }
         }
      }
      catch (ExceptionBase& e) { 
         Console.Log(HERE, e); 
      }

      // Restore selection
      FreezeWindow(false);
      SuspendUndo(false);
   }

   /// <summary>Scrolls/closes the suggestion list in response to caret movement or character input</summary>
   void ScriptEdit::UpdateSuggestions()
   {
      // Ensure exists
      if (SuggestionsList.GetSafeHwnd() == nullptr)
         throw InvalidOperationException(HERE, L"suggestion list does not exist");

      // Find token at caret
      CommandLexer lex(GetLineText(-1));
      TokenIterator pos = lex.Find(GetCaretIndex());

      //Console << L"Updating from token '" << (lex.Valid(pos)?pos->Text:L"<end>") << L"' at index " << GetCaretIndex() << ENDL;
      
      // Close suggestions if caret has left the token
      if (!lex.Valid(pos) || !MatchSuggestionType(pos->Type))
      {
         //Console << Cons::Error << L"Match failed" << ENDL;
         CloseSuggestions();
         return;
      }

      // Highlight best match
      //Console << Cons::Green << L"Matching suggestion" << ENDL;
      SuggestionsList.MatchSuggestion(*pos);
   }

   // ------------------------------- PRIVATE METHODS ------------------------------
   


   
NAMESPACE_END2(GUI,Controls)



