// AutoClicker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "AutoClicker.h"

#define MAX_LOADSTRING 100

#define MSG_AUTOCLICKER_RIGHTCLICK	(WM_USER + 1000)
#define MSG_AUTOCLICKER_LEFTCLICK	(WM_USER + 1001)
#define MSG_AUTOCLICKER_SINGLECLICK (WM_USER + 1002)
#define MSG_AUTOCLICKER_DOBLECLICK	(WM_USER + 1003)

#define MSG_AUTOCLICKER_THREAD_ENDED	(WM_USER + 5000)

typedef struct _auto_clicker {
	DWORD Click;
	DWORD Type;
	char Keyword[1024];
	BOOL Ctrl;
	BOOL Alt;
	BOOL Shift;
	BOOL WinKey;
	BOOL Enter;
	DWORD Interval;
	DWORD Times;
	BOOL Infinate;
	BOOL Current;
	BOOL Set;
	DWORD X;
	DWORD Y;
	DWORD DelayStart;
	BOOL Log;
}AUTOCLICKER, *PAUTOCLICKER;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_hDlg = NULL;

AUTOCLICKER g_Clicker = { 0 };

HANDLE g_hClicker = NULL;
DWORD g_ClickerId = 0;
BOOL g_Running = FALSE;
HANDLE g_hLog = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Help(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ScriptBuilder(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AutoClickerFunc(HWND, UINT, WPARAM, LPARAM);
void InitAutoClickerDialog(HWND hDlg);
BOOL DoStartAutoClicker(HWND hDlg);
BOOL GetAutoClickerInfo(HWND hDlg, PAUTOCLICKER pCliker);
DWORD WINAPI AutoClickerThreadFunction(LPVOID lpParam);

void DoAutoClickerMouseClick();
void DoAutoClickerMouseDClick();
//void DoAutoClickerMouseMove();
void DoAutoClickerSendKeyword();
void DoAutoClickerSendKeywordEx();
void DoAutoClickerSendEnter();
void DoAutoClickerSendUP();
void WriteAutoClickerBanner();
BOOL WriteAutoClickerLogData(char * data, int code=GetLastError());
void DoAutoClickerAi();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AUTOCLICKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCLICKER));

    MSG msg;
	g_hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_AUTO_CLICKER), 0, AutoClickerFunc, 0);
	ShowWindow(g_hDlg, nCmdShow);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCLICKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AUTOCLICKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
		if (LOWORD(wParam) == IDC_BUTTON1)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_AUTO_CLICKER_HELP), hDlg, Help);
			break;
		}
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Help(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AutoClickerFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		InitAutoClickerDialog(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch(wParam) 
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			PostQuitMessage(0);
			return (INT_PTR)TRUE;
		case IDC_BUTTON1: // Start/End
			DoStartAutoClicker(hDlg);
			break;
		case IDC_BUTTON2: // About
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		case MSG_AUTOCLICKER_THREAD_ENDED:
			Sleep(2000);
			SetDlgItemText(hDlg, IDC_BUTTON1, L"Start");
			SetDlgItemText(hDlg, IDC_EDIT4, L"Done!");
			g_Running = FALSE;
		//	if (g_hClicker != NULL)
		//		CloseHandle(g_hClicker);
		//	if (g_hLog != NULL)
		//		CloseHandle(g_hLog);
			break;
		case IDC_BUTTON3: // Set script file (into EDIT8 and check8)
			//char path[MAX_PATH] = {};
			//GetModuleFileNameA(NULL, path, MAX_PATH);
			//PathCchRemoveFileSpec(path, MAX_PATH); // pathcch.h | Pathcch.lib
			//PathCccCombineA(path, path, "config.ini");
			
			break;
		case IDC_BUTTON4: // Track
			break;
		case IDC_BUTTON5:
				//Show example of script file
				break;
		case IDC_BUTTON6: // Script Builder
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_AUTO_CLICKER_SCRIPT), hDlg, ScriptBuilder);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ScriptBuilder(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}


		break;
	}
	return (INT_PTR)FALSE;
}

void InitAutoClickerDialog(HWND hDlg)
{
	HWND hWndComboBox;
	SetDlgItemInt(hDlg, IDC_EDIT2, 100, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT5, 10, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT3, 5000, TRUE);

	hWndComboBox = GetDlgItem(hDlg, IDC_COMBO1);
	SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Right"));
	SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Left"));
	SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	hWndComboBox = GetDlgItem(hDlg, IDC_COMBO2);
	SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Single click"));
	SendMessage(hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("Double click"));
	SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

	CheckDlgButton(hDlg, IDC_RADIO1, BST_CHECKED);
	g_Running = FALSE;
}

BOOL DoStartAutoClicker(HWND hDlg)
{
	DWORD dwDelayStart = 0;
	wchar_t wcStartStopButton[MAX_PATH] = { 0 };
	BOOL bStart = TRUE;
	GetDlgItemText(hDlg, IDC_BUTTON1, wcStartStopButton, MAX_PATH);
	if (wcscmp(wcStartStopButton, L"Start") == 0)
	{
		bStart = TRUE;
		SetDlgItemText(hDlg, IDC_BUTTON1, L"Stop");
	}
	else
	{
		bStart = FALSE;
		SetDlgItemText(hDlg, IDC_BUTTON1, L"Start");
	}

	if (bStart)
	{
		GetAutoClickerInfo(hDlg, &g_Clicker);
		
		if ( g_Clicker.Log)
		{
			wchar_t wsDir[MAX_PATH] = { 0 };
			wchar_t wsLogPath[MAX_PATH] = { 0 };
			GetCurrentDirectory(MAX_PATH, wsDir);
			swprintf_s(wsLogPath, L"%s\\autoclicker.log", wsDir);
			SetDlgItemText(hDlg, IDC_EDIT4, wsLogPath);
			g_hLog = CreateFile(wsLogPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
			if (g_hLog == NULL)
				g_Clicker.Log = FALSE;
			else
			{

				SetDlgItemText(hDlg, IDC_EDIT4, L"Log started!");
				WriteAutoClickerBanner();
			}
		}
		// Start the clicker thread
		g_hClicker = CreateThread(NULL,0,AutoClickerThreadFunction,hDlg, 0,&g_ClickerId);
		if (g_hClicker == NULL)
			SetDlgItemText(hDlg, IDC_EDIT4, L"Failed to start clicker!");
		else
		{
			SetDlgItemText(hDlg, IDC_EDIT4, L"Clicker Started!");
			g_Running = TRUE;

			// Need to disable access to the GUI...
		}
		
	}
	else
	{
		// Stop the clicker
		g_Running = FALSE;
		if(g_hClicker != NULL)
			CloseHandle(g_hClicker);
		if (g_hLog != NULL)
			CloseHandle(g_hLog);
	}

	return TRUE;
}

void TestKeywordMouse(void)
{
	INPUT ip;
	/*
	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "A" key
	ip.ki.wVk = 0x41; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "A" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
	*/

	POINT pc;
	GetCursorPos(&pc);

	ip.type = INPUT_MOUSE;

	// Press the LBUTTON DOWN key
	ip.mi.dx = pc.x;
	ip.mi.dy = pc.y;
	ip.mi.mouseData = 0;
	ip.mi.time = 0;
	ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	ip.mi.dwExtraInfo = 0;

	SendInput(1, &ip, sizeof(INPUT));

	ip.mi.dwFlags = MOUSEEVENTF_LEFTUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));


	//CreateProcessAsUser()
}

BOOL GetAutoClickerInfo(HWND hDlg, PAUTOCLICKER pCliker)
{
	wchar_t wcTmp[1024] = { 0 };
	GetDlgItemText(hDlg, IDC_COMBO1, wcTmp, 1024);
	if (wcscmp(wcTmp, L"Right") == 0)
		pCliker->Click = MSG_AUTOCLICKER_RIGHTCLICK; // Combo 1
	else
		pCliker->Click = MSG_AUTOCLICKER_LEFTCLICK;

	ZeroMemory(wcTmp, 1024);
	GetDlgItemText(hDlg, IDC_COMBO2, wcTmp, 1024);
	if (wcscmp(wcTmp, L"Single click") == 0)
		pCliker->Type = MSG_AUTOCLICKER_SINGLECLICK; // Combo 2
	else
		pCliker->Type = MSG_AUTOCLICKER_DOBLECLICK;

	char kword[1024] = { 0 };
	ZeroMemory(pCliker->Keyword, 1024);
	ZeroMemory(kword, 1024);
	GetDlgItemTextA(hDlg, IDC_EDIT1, kword, 1024); // Edit1
	for (size_t ii = 0; ii < strlen(kword);ii++)
		pCliker->Keyword[ii] = toupper(kword[ii]);
	
	if(BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_CHECK2))
		pCliker->Ctrl=TRUE; //Check 2
	else
		pCliker->Ctrl = FALSE;
	
	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK3))
		pCliker->Alt = TRUE; //Check 3
	else
		pCliker->Alt = FALSE;

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK4))
		pCliker->Shift = TRUE; //Check 4
	else
		pCliker->Shift = FALSE;

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK5))
		pCliker->WinKey = TRUE; //Check 5
	else
		pCliker->WinKey = FALSE;

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK9))
		pCliker->Enter = TRUE; //Check 5
	else
		pCliker->Enter = FALSE;
	

	GetDlgItemInt(hDlg, IDC_EDIT2, NULL, TRUE);
	pCliker->Interval = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, TRUE); // Edit 2
	pCliker->Times = GetDlgItemInt(hDlg, IDC_EDIT5, NULL, TRUE); // Edit 5
	
	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK6))
		pCliker->Infinate = TRUE; // Check 6
	else
		pCliker->Infinate = FALSE;

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO1))
		pCliker->Current = TRUE; // Radio 1
	else
		pCliker->Current = FALSE;

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RADIO2))
		pCliker->Set = TRUE; // Radio 2
	else
		pCliker->Set = FALSE;

	pCliker->X = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, TRUE); // Edit 6
	pCliker->Y = GetDlgItemInt(hDlg, IDC_EDIT7, NULL, TRUE); // Edit 7
	pCliker->DelayStart = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, TRUE); // Edit 3

	if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CHECK7))
		pCliker->Log = TRUE; //Check 7
	else
		pCliker->Log = FALSE;
	return TRUE;
}

DWORD WINAPI AutoClickerThreadFunction(LPVOID lpParam)
{
	DWORD i = 0;
	HWND h = (HWND)lpParam;
	Sleep(g_Clicker.DelayStart);
	if (g_Clicker.Infinate == TRUE)
	{
		while (g_Running)
		{
			Sleep(g_Clicker.Interval);
			DoAutoClickerMouseClick();
			DoAutoClickerMouseDClick();
			DoAutoClickerSendKeyword();
			DoAutoClickerSendEnter();
		}
	}
	else
	{
		for (i = 0;i < g_Clicker.Times;i++)
		{
			if (g_Running == FALSE)
				break;
			Sleep(g_Clicker.Interval);
		//	DoAutoClickerMouseClick();
		//	DoAutoClickerMouseDClick();
		//	DoAutoClickerSendKeyword();
		//	DoAutoClickerSendEnter();

			//DoAutoClickerAi();
			DoAutoClickerSendKeywordEx();
		}
	}

	g_Running = FALSE;
	SendMessage(g_hDlg, WM_COMMAND, (WPARAM)MSG_AUTOCLICKER_THREAD_ENDED, 0);
	return 0;
}

void DoAutoClickerMouseClick()
{
	if (g_Clicker.Type != MSG_AUTOCLICKER_SINGLECLICK)
		return;

	WriteAutoClickerLogData("DoAutoClickerMouseClick start", 0);
	INPUT ip;
	POINT pc;

	GetCursorPos(&pc);

	ip.type = INPUT_MOUSE;

	// Press the LBUTTON DOWN key
	ip.mi.dx = pc.x;
	ip.mi.dy = pc.y;
	ip.mi.mouseData = 0;
	ip.mi.time = 0;
	if(g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	ip.mi.dwExtraInfo = 0;

	SendInput(1, &ip, sizeof(INPUT));

	if (g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &ip, sizeof(INPUT));
	WriteAutoClickerLogData("DoAutoClickerMouseClick end", 0);
}

void DoAutoClickerMouseDClick()
{
	if (g_Clicker.Type != MSG_AUTOCLICKER_DOBLECLICK)
		return;
	
	WriteAutoClickerLogData("DoAutoClickerMouseDClick start", 0);
	INPUT ip;
	POINT pc;

	GetCursorPos(&pc);

	ip.type = INPUT_MOUSE;

	// Press the LBUTTON DOWN key
	ip.mi.dx = pc.x;
	ip.mi.dy = pc.y;
	ip.mi.mouseData = 0;
	ip.mi.time = 0;
	if (g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	ip.mi.dwExtraInfo = 0;

	SendInput(1, &ip, sizeof(INPUT));

	if (g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &ip, sizeof(INPUT));


	ip.type = INPUT_MOUSE;

	// Press the LBUTTON DOWN key
	ip.mi.dx = pc.x;
	ip.mi.dy = pc.y;
	ip.mi.mouseData = 0;
	ip.mi.time = 0;
	if (g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	ip.mi.dwExtraInfo = 0;

	SendInput(1, &ip, sizeof(INPUT));

	if (g_Clicker.Click == MSG_AUTOCLICKER_LEFTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	else if (g_Clicker.Click == MSG_AUTOCLICKER_RIGHTCLICK)
		ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	else
		ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	SendInput(1, &ip, sizeof(INPUT));
	WriteAutoClickerLogData("DoAutoClickerMouseDClick end", 0);
}

void DoAutoClickerSendKeyword()
{
	int i = 0, size = strlen(g_Clicker.Keyword);
	if (g_Clicker.Keyword == NULL)
		return;

	if (size < 1)
		return;

	WriteAutoClickerLogData("DoAutoClickerSendKeyword start", 0);
	INPUT ip;
	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "A" key
	for (i = 0; i < size;i++)
	{
		ip.ki.wVk = g_Clicker.Keyword[i]; // virtual-key code for the "a" key
		ip.ki.dwFlags = 0; // 0 for key press
		SendInput(1, &ip, sizeof(INPUT));

		// Release the "A" key
		ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
		SendInput(1, &ip, sizeof(INPUT));
	}

	WriteAutoClickerLogData("DoAutoClickerSendKeyword end", 0);
}

void DoAutoClickerSendKeywordEx()
{
	int i = 0, size = strlen(g_Clicker.Keyword), key=0;
	if (g_Clicker.Keyword == NULL)
		return;

	if (size < 1)
		return;

	WriteAutoClickerLogData("DoAutoClickerSendKeyword start", 0);
	INPUT ip;
	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wVk = 0;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "A" key
	//https://stackoverflow.com/questions/49224390/c-sendinput-doesnt-manage-alt-codes-properly
	for (i = 0; i < size;i++)
	{
/*
---     ---------------   ---------------   ---------------   -----------
| 01|   | 3B| 3C| 3D| 3E| | 3F| 40| 41| 42| | 43| 44| 57| 58| |+37|+46|+45|
---     ---------------   ---------------   ---------------   -----------

-----------------------------------------------------------   -----------   ---------------
| 29| 02| 03| 04| 05| 06| 07| 08| 09| 0A| 0B| 0C| 0D|     0E| |*52|*47|*49| |+45|+35|+37| 4A|
|-----------------------------------------------------------| |-----------| |---------------|
|   0F| 10| 11| 12| 13| 14| 15| 16| 17| 18| 19| 1A| 1B|   2B| |*53|*4F|*51| | 47| 48| 49|   |
|-----------------------------------------------------------|  -----------  |-----------| 4E|
|    3A| 1E| 1F| 20| 21| 22| 23| 24| 25| 26| 27| 28|      1C|               | 4B| 4C| 4D|   |
|-----------------------------------------------------------|      ---      |---------------|
|      2A| 2C| 2D| 2E| 2F| 30| 31| 32| 33| 34| 35|        36|     |*4C|     | 4F| 50| 51|   |
|-----------------------------------------------------------|  -----------  |-----------|-1C|
|   1D|-5B|   38|                       39|-38|-5C|-5D|  -1D| |*4B|*50|*4D| |     52| 53|   |
-----------------------------------------------------------   -----------   ---------------
*/
		// 0x10 q
		// 0x11 w
		// 0x2E c
		switch (g_Clicker.Keyword[i])
		{
		case 'a': break;
		case 'A': break;
		case 'b': break;
		case 'B': break;
		case 'c': 
		case 'C': key = 0x2E;break;
		case 'd': break;
		case 'D': break;
		case 'e': break;
		case 'E': break;
		case 'f': break;
		case 'F': break;
		case 'g': break;
		case 'G': break;
		case 'h': break;
		case 'H': break;
		case 'i': break;
		case 'I': break;
		case 'j': break;
		case 'J': break;
		case 'k': break;
		case 'K': break;
		case 'l': break;
		case 'L': break;
		case 'm': break;
		case 'M': break;
		case 'n': break;
		case 'N': break;
		case 'o': break;
		case 'O': break;
		case 'p': break;
		case 'P': break;
		case 'q': 
		case 'Q': key = 0x10; break;
		case 'r': break;
		case 'R': break;
		case 's': break;
		case 'S': break;
		case 't': break;
		case 'T': break;
		case 'u': break;
		case 'U': break;
		case 'v': break;
		case 'V': break;
		case 'w': break;
		case 'W': key = 0x11; break;
		case 'x': break;
		case 'X': break;
		case 'y': break;
		case 'Y': break;
		case 'z': break;
		case 'Z': break;
		}
		ip.ki.wScan = key;//g_Clicker.Keyword[i]; // virtual-key code for the "a" key
		ip.ki.dwFlags = KEYEVENTF_SCANCODE;
		SendInput(1, &ip, sizeof(INPUT));

		// Release the  key
		ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
		SendInput(1, &ip, sizeof(INPUT));
	}

	WriteAutoClickerLogData("DoAutoClickerSendKeyword end", 0);
}

void DoAutoClickerSendEnter()
{
	if (g_Clicker.Enter == FALSE)
		return;

	WriteAutoClickerLogData("DoAutoClickerSendEnter start", 0);
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	//VK_SHIFT , VK_CONTROL, VK_BACK, VK_ESCAPE, VK_MENU (ALT), VK_LWIN, VK_RWIN
	ip.ki.wVk = VK_RETURN;//13; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "A" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
	
	WriteAutoClickerLogData("DoAutoClickerSendEnter end", 0);

}

void DoAutoClickerSendUP()
{
	//if (g_Clicker.Enter == FALSE)
	//	return;

	WriteAutoClickerLogData("DoAutoClickerSendEnter start", 0);
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	
	ip.ki.wVk = VK_UP;
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "A" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));

	WriteAutoClickerLogData("DoAutoClickerSendEnter end", 0);

}
//void DoAutoClickerMouseMove()

void WriteAutoClickerBanner()
{
	if (g_Clicker.Log == FALSE)
		return;
	if (g_hLog == NULL)
		return;

	DWORD dwWrite = 0;
	char cBanner[] = "AutoClicker log started\n";

	WriteFile(g_hLog, cBanner, strlen(cBanner), &dwWrite, NULL);
}

BOOL WriteAutoClickerLogData(char * data, int code)
{
	char buff[64] = { 0 };
	DWORD dwWrite = 0;

	if (g_hLog == NULL)
		return FALSE;
	if (g_Clicker.Log == FALSE)
		return FALSE;
	
	WriteFile(g_hLog, data, strlen(data), &dwWrite, NULL);
	sprintf_s(buff, 64, "[return code %d].\n", code);
	return WriteFile(g_hLog, buff, strlen(buff), &dwWrite, NULL);
}

void DoAutoClickerAi()
{
	POINT pc;
	GetCursorPos(&pc);

	WriteAutoClickerLogData("DoAutoClickerAi start", 0);
	// Save last mouse pos and Have a list of pos to be able to navigate
	//g_PosList->push(pc);

	// Get the Window Handle
	HWND hwnd = WindowFromPoint(pc);

	// Get information on that window
	WINDOWINFO wi = { 0 };
	wi.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(hwnd, &wi);

	// Click in this poss
	DoAutoClickerMouseClick();
	
	pc.x=pc.y += 50;
	// Random chnage mouse pos according to the window size
	SetCursorPos(pc.x, pc.y);

	// Click in new pos
	//for(int x=0;x<5;x++)
	//	DoAutoClickerSendUP();

	// Send keyword if needed
	DoAutoClickerSendKeyword();
	DoAutoClickerSendEnter();
	
	// Random wait
	//int WaitTime = rand() % 10;
	//WaitTime++;
	//Sleep(WaitTime * 1000);

	WriteAutoClickerLogData("DoAutoClickerAi End", 0);
}