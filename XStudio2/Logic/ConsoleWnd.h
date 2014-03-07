#pragma once

namespace Logic
{
   
   /// <summary>Console manipulators</summary>
   enum class Cons { UserAction,
                     Heading, 
                     Normal, 
                     Error,
                     Exception,
                     Success,
                     Failure,
                     Bold, 
                     Endl,
                     Reset,
                     Push,
                     Pop,
                     
                     White,
                     Blue,
                     Cyan,
                     Green,
                     Red,
                     Yellow,
                     Purple
   };

   /// <summary>Shorthand for console end-of-line manipulator</summary>
   const Cons ENDL = Cons::Endl;

   /// <summary>Provides a debugging console</summary>
   class ConsoleWnd
   {
		// ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
   private:
      ConsoleWnd();
   public:
      virtual ~ConsoleWnd();
		 
		NO_COPY(ConsoleWnd);	// Cannot be copied
		NO_MOVE(ConsoleWnd);	// Cannot be moved

      // ------------------------ STATIC -------------------------
   
      // --------------------- PROPERTIES ------------------------
   public:
      PROPERTY_GET_SET(bool,Visible,IsVisible,Show);

   protected:
      PROPERTY_GET_SET(WORD,Attributes,GetAttributes,SetAttributes);

      // ---------------------- ACCESSORS ------------------------			
   public:
      /// <summary>Determines whether console is visible.</summary>
      /// <returns></returns>
      bool IsVisible() const
      {
         return IsWindowVisible(GetConsoleWindow()) != FALSE;
      }

   protected:
      /// <summary>Gets the attributes.</summary>
      /// <returns></returns>
      WORD GetAttributes()
      {
         CONSOLE_SCREEN_BUFFER_INFO info;

         // Ensure connected
         if (Handle == INVALID_HANDLE_VALUE)
            return 0;
         
         // Get attributes
         GetConsoleScreenBufferInfo(Handle, &info);
         return info.wAttributes;
      }

      /// <summary>Sets the attributes.</summary>
      /// <param name="attr">The attribute.</param>
      void SetAttributes(WORD attr)
      {
         if (Handle != INVALID_HANDLE_VALUE)
            SetConsoleTextAttribute(Handle, attr);
      }

      // ----------------------- MUTATORS ------------------------
   public:
      /// <summary>Logs an exception to the console.</summary>
      /// <param name="src">Handler location</param>
      /// <param name="e">error</param>
      /// <param name="msg">Handler error message</param>
      void  Log(const GuiString& src, const ExceptionBase&e, const GuiString& msg)
      {
         *this << ENDL 
               << Cons::Exception << L"ERROR: " 
               << Cons::Error     << msg.Remove(L"\r\n")
               << Cons::White     << L"...  Source: " 
               << Cons::Yellow    << src << ENDL;
         *this << Cons::Exception << L"CAUSE: " 
               << Cons::Error     << e.Message.Remove(L"\r\n")
               << Cons::White     << L"...  Source: " 
               << Cons::Yellow    << e.Source << ENDL;
      }

      /// <summary>Logs an exception to the console.</summary>
      /// <param name="src">Handler location</param>
      /// <param name="e">error</param>
      void  Log(const GuiString& src, const ExceptionBase&e)
      {
         *this << ENDL 
               << Cons::Exception << L"EXCEPTION: " 
               << Cons::Error     << e.Message.Remove(L"\r\n") 
               << Cons::White     << L"...  Source: " 
               << Cons::Yellow    << src << ENDL;
         *this << Cons::Exception << L"SOURCE: " 
               << Cons::Yellow    << e.Source << ENDL;
      }

      /// <summary>Logs an STL exception to the console.</summary>
      /// <param name="src">Handler location</param>
      /// <param name="e">error</param>
      void  Log(const GuiString& src, const exception&e)
      {
         *this << ENDL 
               << Cons::Exception << L"STL EXCEPTION: " 
               << Cons::Error     << e.what()
               << Cons::White     << L"...  Source: " 
               << Cons::Yellow    << src << ENDL;
      }

      /// <summary>Shows/Hides the console.</summary>
      /// <param name="show">The show.</param>
      void  Show(bool show)
      {
         ShowWindow(GetConsoleWindow(), show ? SW_SHOW : SW_HIDE);
      }

      /// <summary>Inserts associated text colour manipulator, if any</summary>
      /// <param name="c">Colour</param>
      ConsoleWnd& operator<<(Colour c)
      {
         switch (c)
         {
         // Supported
         case Colour::Blue:   return *this << Cons::Blue;
         case Colour::Green:  return *this << Cons::Green;
         case Colour::Red:    return *this << Cons::Red;
         case Colour::Cyan:   return *this << Cons::Cyan;
         case Colour::Purple: return *this << Cons::Purple;
         case Colour::Orange:
         case Colour::Yellow: return *this << Cons::Yellow;
         case Colour::Default:
         case Colour::Silver:
         case Colour::Black:
         case Colour::White:  return *this << Cons::White;
         }

         // Unsupported
         return *this;
      }

      /// <summary>Inserts text manipulator</summary>
      /// <param name="cl">manipulator</param>
      ConsoleWnd& operator<<(Cons c)
      {
         switch (c)
         {
         // Bold: Add bold
         case Cons::Bold:   
            Attributes = Attributes|FOREGROUND_INTENSITY; 
            break;

         // Normal: Remove bold
         case Cons::Normal: 
            Attributes = Attributes & ~FOREGROUND_INTENSITY; 
            break;

         // Reset: White + Normal
         case Cons::Reset:
            return *this << Cons::Normal << Cons::White;

         // Endl: Reset + Linebreak 
         case Cons::Endl:   
            return *this << Cons::Reset << L"\n";

         // Heading: Linebreak + Cyan
         case Cons::Heading:  
            return *this << ENDL << Cons::Cyan;

         // User Action: Linebreak + Bold + Cyan
         case Cons::UserAction:  
            return *this << ENDL << Cons::Bold << Cons::Cyan;

         // Exception: Bold + Purple
         case Cons::Exception:  
            return *this << Cons::Bold << Cons::Purple;

         // Error: Bold + Red
         case Cons::Error:  
            return *this << Cons::Bold << Cons::Red;

         // Error: Bold + Green
         case Cons::Success:  
            return *this << Cons::Bold << Cons::Green << "Success";

         // Error: Bold + Red
         case Cons::Failure:  
            return *this << Cons::Bold << Cons::Red << "Failed: ";

         // Push attributes: Save current attributes
         case Cons::Push:
            AttributeStack.push_back(Attributes);
            return *this;

         // Push attributes: Restore previously saved attributes 
         case Cons::Pop:
            if (!AttributeStack.empty())
            {
               Attributes = AttributeStack.back();
               AttributeStack.pop_back();
            }
            return *this;

         // Colour
         default:
            WORD bold = (Attributes & FOREGROUND_INTENSITY);

            switch (c)
            {
            case Cons::Blue:   Attributes = bold|FOREGROUND_BLUE;                   break;
            case Cons::Green:  Attributes = bold|FOREGROUND_GREEN;                  break;
            case Cons::Red:    Attributes = bold|FOREGROUND_RED;                    break;
            case Cons::Cyan:   Attributes = bold|FOREGROUND_BLUE|FOREGROUND_GREEN;  break;
            case Cons::Purple: Attributes = bold|FOREGROUND_BLUE|FOREGROUND_RED;    break;
            case Cons::Yellow: Attributes = bold|FOREGROUND_GREEN|FOREGROUND_RED;   break;
            case Cons::White:  Attributes = bold|FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE; break;
            }
         }

         return *this;
      }

      /// <summary>Writes text to the console</summary>
      /// <param name="txt">Text</param>
      ConsoleWnd& operator<<(const WCHAR* txt)
      {
         WriteText(txt);
         return *this;
      }

      /// <summary>Writes ansi text to the console</summary>
      /// <param name="txt">Text</param>
      ConsoleWnd& operator<<(const char* txt)
      {
         return *this << StringResource::Convert(txt, CP_ACP);
      }

      /// <summary>Writes a character to the console</summary>
      /// <param name="ch">Character</param>
      ConsoleWnd& operator<<(wchar ch)
      {
         wchar buf[2] = {ch, NULL};
         return *this << buf;
      }

      /// <summary>Writes signed integer to the console</summary>
      /// <param name="i">Number</param>
      ConsoleWnd& operator<<(int i)
      {
         Writef(L"%d", i);
         return *this;
      }

      /// <summary>Writes unsigned integer to the console</summary>
      /// <param name="i">number</param>
      ConsoleWnd& operator<<(UINT i)
      {
         Writef(L"%u", i);
         return *this;
      }

      /// <summary>Writes text to the console</summary>
      /// <param name="txt">Text</param>
      ConsoleWnd& operator<<(const wstring& txt)
      {
         WriteText(txt);
         return *this;
      }

      /// <summary>Writes ansi text to the console</summary>
      /// <param name="txt">Text</param>
      ConsoleWnd& operator<<(const string& txt)
      {
         return *this << StringResource::Convert(txt, CP_ACP);
      }

      /// <summary>Writes pointer to the console</summary>
      /// <param name="p">Pointer</param>
      ConsoleWnd& operator<<(const void* p)
      {
         Writef(L"0x%x", p);
         return *this;
      }

      /// <summary>Writes a path to the console in yellow</summary>
      /// <param name="path">path</param>
      ConsoleWnd& operator<<(const IO::Path& path)
      {
         return *this << Cons::Push << Cons::Yellow << path.c_str() << Cons::Pop;
      }

      /// <summary>Writes a game version string to the console in yellow</summary>
      /// <param name="str">game version string</param>
      ConsoleWnd& operator<<(const VersionString& str)
      {
         return *this << Cons::Push << Cons::Yellow << str.c_str() << Cons::Pop;
      }

      /// <summary>Writes text to the console</summary>
      /// <param name="txt">Text</param>
      void  Write(const wstring& txt)
      {
         WriteText(txt);
      }

      /// <summary>Writes the formatted text to the console</summary>
      /// <param name="format">Formatting string</param>
      /// <param name="...">Arguments</param>
      void  Writef(const WCHAR* format, ...)
      {
         va_list args;
         WriteText( StringResource::FormatV(format, va_start(args, format)) );
      }

      /// <summary>Writes the formatted text to the console</summary>
      /// <param name="format">Formatting string</param>
      /// <param name="...">Arguments</param>
      void  Writef(const wstring& format, ...)
      {
         va_list args;
         WriteText( StringResource::FormatV(format.c_str(), va_start(args, format)) );
      }

      /// <summary>Writes text to the console</summary>
      /// <param name="txt">Text</param>
      void  WriteLn(const wstring& txt = L"")
      {
         WriteText(txt);
         WriteText(L"\n");
      }

      /// <summary>Writes the formatted text to the console</summary>
      /// <param name="format">Formatting string</param>
      /// <param name="...">Arguments</param>
      void  WriteLnf(const WCHAR* format = L"", ...)
      {
         va_list args;
         WriteText( StringResource::FormatV(format, va_start(args, format)) );
         WriteText(L"\n");
      }

      /// <summary>Writes the formatted text to the console</summary>
      /// <param name="format">Formatting string</param>
      /// <param name="...">Arguments</param>
      void  WriteLnf(const wstring& format, ...)
      {
         va_list args;
         WriteText( StringResource::FormatV(format.c_str(), va_start(args, format)) );
         WriteText(L"\n");
      }

   private:
      /// <summary>Writes text to the output.</summary>
      /// <param name="txt">The text.</param>
      void WriteText(const wstring& txt);
//      {
//         DWORD written=0;
//
//         // Ensure exists
//         if (Handle)
//            WriteConsole(Handle, txt.c_str(), txt.length(), &written, NULL);
//         
//#ifdef _DEBUG
//         OutputDebugString(txt.c_str());
//#endif
//      }

      // -------------------- REPRESENTATION ---------------------
   public:
      static ConsoleWnd  Instance;

   private:
      deque<WORD> AttributeStack;
      HANDLE      Handle;
   };

   // Provide singleton access
   #define Console      ConsoleWnd::Instance
}

using namespace Logic;
