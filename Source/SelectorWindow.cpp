#include <Windows.h>

#define WINDOW_LISTBOX   101
#define APPLY_BUTTON     102
#define DESKTOP_COMBOBOX 103

// TODO: Convert to LISTVIEW
//#define IDC_MAIN_LISTVIEW

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PopulateWindowsEnumProc(HWND, LPARAM);
BOOL CALLBACK PopulateDesktopsEnumProc(HMONITOR, HDC, LPRECT, LPARAM);

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR szCmdLine,
	int nCmdShow)
{
	static TCHAR szAppName[] = TEXT("Selector Window");
	HWND hWnd;
	MSG msg;
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szAppName;
	wc.hIconSm       = NULL;

	wc.lpszClassName = szAppName;

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL,
			TEXT("This program requires Windows NT."),
			szAppName,
			MB_ICONERROR);
		return 0;
	}
	
	// Create main window
	hWnd = CreateWindowEx(0,
		szAppName,
		TEXT("Border Remover"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		400,
		500,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL);

	if(!hWnd)
	{
		MessageBox(hWnd, TEXT("Could not create window."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	// Get the dimensions available
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

	// Create listbox
	HWND hList;
	hList = CreateWindowEx(WS_EX_CLIENTEDGE,
		TEXT("LISTBOX"),
		TEXT(""),
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_SORT|LBS_NOINTEGRALHEIGHT,
		0,
		0,
		rcClient.right,
		rcClient.bottom-25,
		hWnd,
		(HMENU)WINDOW_LISTBOX,
		hInstance,
		NULL);

	// Check for failure
	if(!hList)
	{
		MessageBox(hWnd, TEXT("Could not create list box."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Allow Horizontal Scrolling
	SendMessage(hList, LB_SETHORIZONTALEXTENT, 1920, NULL);
	// Populate the list
	EnumWindows(PopulateWindowsEnumProc, (LPARAM)hWnd);

	// Create the apply button
	HWND hButton;
	hButton = CreateWindowEx(WS_EX_CLIENTEDGE,
		TEXT("BUTTON"),
		TEXT("Apply"),
		WS_CHILDWINDOW|WS_VISIBLE|BS_PUSHBUTTON,
		rcClient.right-100,
		rcClient.bottom-25,
		100,
		25,
		hWnd,
		(HMENU)APPLY_BUTTON,
		hInstance,
		NULL);
	
	// Check for failure
	if(!hButton)
	{
		MessageBox(hWnd, TEXT("Could not create button."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Create the desktop selection menu
	HWND hDesktopBox;
	hDesktopBox = CreateWindowEx(WS_EX_CLIENTEDGE,
		TEXT("COMBOBOX"),
		TEXT(""),
		WS_CHILDWINDOW|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_HASSTRINGS,
		0,
		rcClient.bottom-25,
		rcClient.right-100,
		125,
		hWnd,
		(HMENU)DESKTOP_COMBOBOX,
		hInstance,
		NULL);
	
	// Check for failure
	if(!hDesktopBox)
	{
		MessageBox(hWnd, TEXT("Could not create combo box."), TEXT("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Populate combo box
	EnumDisplayMonitors(NULL, NULL, PopulateDesktopsEnumProc, (LPARAM)hWnd); 

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd); 

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch(msg)
	{
		case WM_CREATE:
		{	
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case APPLY_BUTTON:
				{
					int index;
					HWND targetWindow;
					HMONITOR targetDesktop;
					MONITORINFO desktopInfo;
					LPRECT desktopRect;

					// Get the index of the selected window in the list
					index = SendDlgItemMessage(hWnd, WINDOW_LISTBOX, LB_GETCURSEL, NULL, NULL);
					
					// Make sure we have selected a window
					if(index == LB_ERR)
					{
						MessageBox(hWnd, TEXT("Must select a window."), TEXT("Error"), MB_OK | MB_ICONERROR);
						return FALSE;
					}

					// Get the handle to the window that we want to modify from the selected item
					targetWindow = (HWND)SendDlgItemMessage(hWnd, WINDOW_LISTBOX, LB_GETITEMDATA, index, NULL);

					// Get the index of the selected desktop in the combo box
					index = SendDlgItemMessage(hWnd, DESKTOP_COMBOBOX, CB_GETCURSEL, NULL, NULL);

					// Make sure we have selected a desktop
					if(index == CB_ERR)
					{
						MessageBox(hWnd, TEXT("Must select a desktop."), TEXT("Error"), MB_OK | MB_ICONERROR);
						return FALSE;
					}

					// Get the handle to the desktop we want to place the window on
					targetDesktop = (HMONITOR)SendDlgItemMessage(hWnd, DESKTOP_COMBOBOX, CB_GETITEMDATA, index, NULL);
					//desktopRect = (LPRECT)SendDlgItemMessage(hWnd, DESKTOP_COMBOBOX, CB_GETITEMDATA, index, NULL);

					// Get a pointer to the rectangle describing the desktop
					desktopInfo.cbSize = sizeof(MONITORINFO);
					GetMonitorInfo(targetDesktop, &desktopInfo);
					desktopRect = &(desktopInfo.rcMonitor);

					// Get the old style
					DWORD newStyle, oldStyle;
					oldStyle = GetWindowLong(targetWindow, GWL_STYLE);

					// Apply changes
					newStyle = oldStyle ^ 0x00C00000L;
					SetWindowLong(targetWindow, GWL_STYLE, newStyle);
					SetWindowPos(targetWindow,
						NULL,
						desktopRect->left,
						desktopRect->top,
						desktopRect->right - desktopRect->left,
						desktopRect->bottom - desktopRect->top,
						SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE);
					InvalidateRect(targetWindow, NULL, TRUE);

					break;
				}
			}
			break;
		}
		case WM_SIZE:
		{
			HWND hList;
			HWND hButton;
			HWND hCombo;
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);

			// Set appropriate dimensions for when the window is resized
			hList = GetDlgItem(hWnd, WINDOW_LISTBOX);
			SetWindowPos(hList, NULL, 0, 0, rcClient.right, rcClient.bottom-25, SWP_NOZORDER);
			hButton = GetDlgItem(hWnd, APPLY_BUTTON);
			SetWindowPos(hButton, NULL, rcClient.right-100, rcClient.bottom-25, 100, 25, SWP_NOZORDER);
			hCombo = GetDlgItem(hWnd, DESKTOP_COMBOBOX);
			SetWindowPos(hCombo, NULL, 0, rcClient.bottom-25, rcClient.right-100, 125, SWP_NOZORDER);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


// Populates our WINDOW_LISTBOX with every window that does not have an empty title.
// Ideally, we could use the icons and the .exe file to identify, but those are non-trivial
// in comparison to the title.
BOOL CALLBACK PopulateWindowsEnumProc(HWND hWnd, LPARAM lParam)
{
	// Adjust this length
	WCHAR fileName[512];
	//GetWindowModuleFileName(hWnd,fileName,512);

	// Get the title
	SendMessage(hWnd, WM_GETTEXT, MAX_PATH, (LPARAM)fileName);

	// Remove Windows without titles
	if(fileName[0]!='\0')
	{
		// Add the item to the listbox
		int index = SendDlgItemMessage((HWND)lParam, WINDOW_LISTBOX, LB_ADDSTRING, 0, (LPARAM)fileName);

		// Add the window handle as the item data. This may not be safe on x64 machines because
		// the data is required to be 32 bit and I'm not positive hWnd has the same requirements.
		SendDlgItemMessage((HWND)lParam, WINDOW_LISTBOX, LB_SETITEMDATA, index, (LPARAM)hWnd);
	}
	return TRUE;
}

BOOL CALLBACK PopulateDesktopsEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM lParam)
{
	// Surely no one has more than 10^4 monitors... right?
	WCHAR displayID[4];
	int index;

	// Get the desktop number by looking at the number of items currently in the combo box
	index = SendDlgItemMessage((HWND)lParam, DESKTOP_COMBOBOX, CB_GETCOUNT, NULL, NULL);

	// Convert the index into a string
	_itow_s(index, displayID, 10);

	// Add an item to the combo box
	index = SendDlgItemMessage((HWND)lParam, DESKTOP_COMBOBOX, CB_ADDSTRING, NULL, (LPARAM)displayID);

	// Attach the data to the combo box as the item data
	SendDlgItemMessage((HWND)lParam, DESKTOP_COMBOBOX, CB_SETITEMDATA, index, (LPARAM)hMonitor);
	
	return TRUE;
}