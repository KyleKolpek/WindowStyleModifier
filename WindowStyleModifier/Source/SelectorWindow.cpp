#include <Windows.h>

#define IDC_MAIN_LISTBOX  101
#define IDC_APPLY_BUTTON 102

// TODO: Convert to LISTVIEW
//#define IDC_MAIN_LISTVIEW 103

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);

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
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|LBS_SORT,
		0,
		0,
		rcClient.right,
		rcClient.bottom-25,
		hWnd,
		(HMENU)IDC_MAIN_LISTBOX,
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
	EnumWindows(EnumWindowsProc, (LPARAM)hWnd);

	// Create the apply button
	HWND hButton;
	hButton = CreateWindowEx(WS_EX_CLIENTEDGE,
		TEXT("BUTTON"),
		TEXT("Apply"),
		WS_CHILDWINDOW|BS_PUSHBUTTON|WS_VISIBLE,
		rcClient.right-100,
		rcClient.bottom-25,
		100,
		25,
		hWnd,
		(HMENU)IDC_APPLY_BUTTON,
		hInstance,
		NULL);

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
				case IDC_APPLY_BUTTON:
				{
					int index;
					HWND targetWindow;

					// Get the index of the selected item
					index = SendDlgItemMessage(hWnd, IDC_MAIN_LISTBOX, LB_GETCURSEL, NULL, NULL);

					// Get the handle to the window that we want to modify from the selected item
					targetWindow = (HWND)SendDlgItemMessage(hWnd, IDC_MAIN_LISTBOX, LB_GETITEMDATA, index, NULL);

					// Get the old style
					DWORD newStyle, oldStyle;
					oldStyle = GetWindowLong(targetWindow, GWL_STYLE);

					// Apply changes
					newStyle = oldStyle ^ 0x00C00000L;
					SetWindowLong(targetWindow, GWL_STYLE, newStyle);
					SetWindowPos(targetWindow, NULL, 0, 0, 1920, 1080,
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
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);

			hList = GetDlgItem(hWnd, IDC_MAIN_LISTBOX);
			SetWindowPos(hList, NULL, 0, 0, rcClient.right, rcClient.bottom-25, SWP_NOZORDER);
			hButton = GetDlgItem(hWnd, IDC_APPLY_BUTTON);
			SetWindowPos(hButton, NULL,rcClient.right-100, rcClient.bottom-25, 100, 25, SWP_NOZORDER);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
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
		int index = SendDlgItemMessage((HWND)lParam, IDC_MAIN_LISTBOX, LB_ADDSTRING, 0, (LPARAM)fileName);

		// Add the window handle as the item data. This may not be safe on x64 machines because
		// the data is required to be 32 bit and I'm not positive hWnd has the same requirements.
		SendDlgItemMessage((HWND)lParam, IDC_MAIN_LISTBOX, LB_SETITEMDATA, index, (LPARAM)hWnd);
	}
	return 1;
}