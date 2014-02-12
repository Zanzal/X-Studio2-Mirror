
// MFC Test 1.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "afxcview.h"
#include "Application.h"
#include "AboutDlg.h"
#include "MainWnd.h"

#include "ScriptDocument.h"
#include "LanguageDocument.h"

//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// --------------------------------- GLOBAL --------------------------------

Application theApp;

// --------------------------------- TYPES ---------------------------------

// --------------------------------- APP WIZARD ---------------------------------
  
BEGIN_MESSAGE_MAP(Application, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &Application::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

// -------------------------------- CONSTRUCTION --------------------------------

Application::Application() : GameDataState(AppState::NoGameData)
{
	SetAppID(_T("BearWare.X-Studio.2"));
}

// ------------------------------- STATIC METHODS -------------------------------

// ------------------------------- PUBLIC METHODS -------------------------------

/// <summary>Exits the instance.</summary>
/// <returns></returns>
int Application::ExitInstance()
{
	//TODO: handle additional resources you may have added
	return CWinAppEx::ExitInstance();
}

/// <summary>Initializes the instance.</summary>
/// <returns></returns>
BOOL Application::InitInstance()
{
	
   //VLDEnable();
   VLDDisable();
   //_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

	// Init common controls
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

   // Initialise OLE/COM
	CWinAppEx::InitInstance();
   AfxOleInit();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDS_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	EnableTaskbarInteraction();

	// Initialise RichEdit
   AfxInitRichEdit2();
	AfxInitRichEdit5();

	
	// Set app registry key 
	SetRegistryKey(L"Bearware");
	LoadStdProfileSettings(4);  // Load MRU

   // Menu manager
	InitContextMenuManager();

   // Keyboard manager
	InitKeyboardManager();

   // Tooltip manager
	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// document templates
	AddDocTemplate(new ScriptDocTemplate());
	AddDocTemplate(new LanguageDocTemplate());

	// Frame window
	MainWnd* pMainFrame = new MainWnd;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;

	// Show window
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

/// <summary>Loads an icon.</summary>
/// <param name="nResID">The resource identifier.</param>
/// <param name="iSize">Size of the icon</param>
/// <returns></returns>
HICON  Application::LoadIcon(UINT nResID, UINT iSize) const
{
   return (HICON)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), IMAGE_ICON, iSize, iSize, 0);
}

/// <summary>Loads a bitmap.</summary>
/// <param name="nResID">The resource identifier.</param>
/// <param name="cx">The width.</param>
/// <param name="cy">The height.</param>
/// <param name="flags">Loading flags.</param>
/// <returns></returns>
CBitmap*  Application::LoadBitmap(UINT nResID, int cx, int cy, UINT flags) const
{
   CBitmap* bmp = new CBitmap();
   HBITMAP h = (HBITMAP)::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), IMAGE_BITMAP, cx, cy, flags);
   bmp->Attach(h);
   return bmp;
}

/// <summary>Dispay about box</summary>
void Application::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/// <summary>Opens the string library.</summary>
/// <returns></returns>
BOOL  Application::OpenStringLibrary()
{
   for (POSITION pos = GetFirstDocTemplatePosition(); pos != NULL; )
   {
      LanguageDocTemplate* doc = dynamic_cast<LanguageDocTemplate*>(GetNextDocTemplate(pos));
      if (doc != nullptr)
         return doc->OpenDocumentFile(L"String Library", FALSE, TRUE) != nullptr;
   }
   return FALSE;
}

/// <summary>Application customization load/save methods</summary>
void Application::PreLoadState()
{
	GetContextMenuManager()->AddMenu(GuiString(IDS_EDIT_MENU).c_str(), IDM_EDIT_POPUP);
	GetContextMenuManager()->AddMenu(GuiString(IDR_PROJECT).c_str(), IDM_PROJECT_POPUP);
}

void Application::LoadCustomState()
{
}

void Application::SaveCustomState()
{
}

/// <summary>Displays and logs an exception</summary>
/// <param name="src">The handler location</param>
/// <param name="e">The exception</param>
/// <param name="msg">The display message</param>
/// <returns></returns>
BOOL Application::ShowError(const GuiString& src, const ExceptionBase& e, const GuiString& msg) const
{
   Console.Log(src, e, msg);
   return AfxMessageBox(GuiString(L"%s : %s\n\nSink: %s\nSource: %s", msg.c_str(), e.Message.c_str(), src.c_str(), e.Source.c_str()).c_str(), MB_ICONERROR|MB_OK);
}

// ------------------------------ PROTECTED METHODS -----------------------------

// ------------------------------- PRIVATE METHODS ------------------------------


