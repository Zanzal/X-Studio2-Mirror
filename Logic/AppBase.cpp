#include "stdafx.h"
#include "AppBase.h"
#include "ConsoleLog.h"

namespace Logic
{
   // -------------------------------- CONSTRUCTION --------------------------------

   AppBase::AppBase()
   {
      m_pszAppName = _tcsdup(L"X-Studio II");      // Override name of subkey used by MFC registry functions
      SetAppID(L"BearWare.X-Studio.2");            // ProgID [Not used at the moment]
      SetRegistryKey(L"Bearware");                 // Define name of base key used by MFC registry functions
   }


   AppBase::~AppBase()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   BEGIN_MESSAGE_MAP(AppBase, CWinAppEx)
   END_MESSAGE_MAP()

   
   /// <summary>Termination handler</summary>
   void  AppBase::OnCriticalError()
   {
      try
      {
         // Ensure console is valid RTF by appending footer
         LogFile.Close();
      }
      catch (ExceptionBase&) {
      }
   }

   // ------------------------------- PUBLIC METHODS -------------------------------

   
   /// <summary>Exits the instance.</summary>
   /// <returns></returns>
   int AppBase::ExitInstance()
   {
      try
      {
         // Free resources
         FreeLibrary(ResourceLibrary);
         ResourceLibrary = NULL;

         // Close LogFile
         LogFile.Close();
      }
      catch (ExceptionBase& e) {
         ShowError(HERE, e);
      }
	
	   return __super::ExitInstance();
   }



   /// <summary>Initializes the instance.</summary>
   /// <returns></returns>
   BOOL AppBase::InitInstance()
   {
	   // Visual Leak Detector
      VLDEnable();

      try
      {
         // Set termination handler
         set_terminate(OnCriticalError);

         // Load resource library
         if(ResourceLibrary = LoadLibrary(L"XStudio2.Resources.dll"))
            AfxSetResourceHandle(ResourceLibrary);
         else
            throw Win32Exception(HERE, L"Unable to load resource library 'XStudio2.Resources.dll'");

         // LogFile
         try
         {
            LogFile.Open();
         }
         catch (ExceptionBase& e) {
            ShowError(HERE, e, L"Unable to open log file");
         }

	      // Init common controls
	      INITCOMMONCONTROLSEX InitCtrls;
	      InitCtrls.dwSize = sizeof(InitCtrls);
	      InitCtrls.dwICC = ICC_WIN95_CLASSES;
	      if (!InitCommonControlsEx(&InitCtrls))
            throw Win32Exception(HERE, L"Unable to initialize common controls library");

         // Initialise OLE/COM
         AfxOleInit();

         // Init App
	      return __super::InitInstance();
      }
      catch (ExceptionBase& e) {
         ShowError(HERE, e);
         return FALSE;
      }
   }

   /// <summary>Displays and logs an exception</summary>
   /// <param name="src">The handler location</param>
   /// <param name="e">The exception</param>
   /// <param name="msg">The display message</param>
   /// <returns></returns>
   BOOL AppBase::ShowError(const GuiString& src, const exception& e, const GuiString& msg) const
   {
      Console.Log(src, e);

      // Show error
      auto text = VString(L"%s : %s\n\nCaught: %s", msg.c_str(), GuiString::Convert(e.what(), CP_ACP).c_str(), src.c_str());
      return ShowMessage(text, MB_ICONERROR|MB_OK);
   }

   /// <summary>Displays and logs an exception</summary>
   /// <param name="src">The handler location</param>
   /// <param name="e">The exception</param>
   /// <param name="msg">The display message</param>
   /// <returns></returns>
   BOOL AppBase::ShowError(const GuiString& src, const ExceptionBase& e, const GuiString& msg) const
   {
      Console.Log(src, e, msg);

      // Application exception: Display verbatim
      if (auto app = dynamic_cast<const ApplicationException*>(&e))
         return ShowMessage(msg + L": " + app->Message, MB_OK|MB_ICONERROR);
   
      // Exception: Display source/sink data
      auto text = VString(L"%s : %s\n\nSink: %s\nSource: %s", msg.c_str(), e.Message.c_str(), src.c_str(), e.Source.c_str());
      return ShowMessage(text, MB_ICONERROR|MB_OK);
   }


   /// <summary>Displays and logs an exception</summary>
   /// <param name="src">The handler location</param>
   /// <param name="e">The exception</param>
   /// <returns></returns>
   BOOL AppBase::ShowError(const GuiString& src, const ExceptionBase& e) const
   {
      Console.Log(src, e);

      // Application exception: Display verbatim
      if (auto app = dynamic_cast<const ApplicationException*>(&e))
         return ShowMessage(app->Message, MB_OK|MB_ICONERROR);

      // Exception: Display source/sink data
      auto text = VString(L"%s\n\nSink: %s\nSource: %s", e.Message.c_str(), src.c_str(), e.Source.c_str());
      return ShowMessage(text, MB_ICONERROR|MB_OK);
   }

   /// <summary>Shows a message.</summary>
   /// <param name="msg">The MSG.</param>
   /// <param name="flags">button/icon flags.</param>
   /// <returns></returns>
   BOOL  AppBase::ShowMessage(const wstring& msg, UINT flags) const
   {
      return AfxMessageBox(msg.c_str(), flags);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------

   // ------------------------------- PRIVATE METHODS ------------------------------
}

