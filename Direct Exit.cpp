/**************************
* Author: Kristjan Borgfjord
* Date: May 17 2019
*
**************************/


#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <ctime>
#include <cstring>
#include <shellapi.h>
#include <string>


#define ICON_NUMBER 1
#define WM_MYMESSAGE (WM_APP + 1)
#define NO_REPEAT_TIME 2

//no repeat time is in seconds

// Definitions
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow);


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT dwSize;
	RAWINPUT *buf;
	static bool shiftDown = false;
	static bool ctrlDown = false;
	static bool altDown = false;
	static bool minusDown = false;
	static clock_t lastUse = clock();
	HMENU menu;
	MENUITEMINFOW item;
	POINT screenPos;
	int menuSelection = 0;
	DWORD dwPID;
	HANDLE handle;
	wchar_t  wnd_title[60];
	HWND foregroundWin;
	static NOTIFYICONDATA trayIcon;

	for (int i = 0; i < 60; i++)
	{
		wnd_title[i] = '\0';
	}
	

	switch (msg)
	{
	
	case WM_CREATE:

		//create the tray icon and display it.

		trayIcon.cbSize = sizeof(NOTIFYICONDATA);
		trayIcon.hWnd = hwnd;
		trayIcon.uID = ICON_NUMBER;
		trayIcon.uFlags = (NIF_MESSAGE | NIF_INFO | NIF_REALTIME | NIF_TIP | NIF_ICON);
		trayIcon.uCallbackMessage = WM_MYMESSAGE;
		trayIcon.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcsncpy_s(trayIcon.szInfo, L"Direct Exit\0", ARRAYSIZE(trayIcon.szInfo)); //wcsncpy is wide char strncpy
		wcsncpy_s(trayIcon.szInfo, L"Direct Exit has started\0", ARRAYSIZE(trayIcon.szInfo));
		wcsncpy_s(trayIcon.szInfoTitle, L"Direct Exit\0", ARRAYSIZE(trayIcon.szInfoTitle));

		if (!Shell_NotifyIconW(NIM_ADD, &trayIcon))
		{
			MessageBox(NULL, L"Unable create notifications. Direct Exit will try to run without a tray icon.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		}
		
		break;

	case WM_INPUT:
		
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
			sizeof(RAWINPUTHEADER)) == 0) 
		{

			buf = (RAWINPUT*)malloc(dwSize);

			if (buf == NULL)
			{ 
				MessageBox(NULL, L"Unable to allocate memory for a buffer. Exiting.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
				return 0;
			}

			ZeroMemory(buf, dwSize);

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf, &dwSize,
				sizeof(RAWINPUTHEADER)))
			{
				
				if (buf->header.dwType == RIM_TYPEKEYBOARD
					&& (buf->data.keyboard.Message == WM_KEYDOWN
					||	buf->data.keyboard.Message == WM_SYSKEYDOWN))
				{
					if (buf->data.keyboard.VKey == 0xBD) //minus
					{
						minusDown = true;
					}
					
					if (buf->data.keyboard.VKey == 0x10) //shift
					{
						shiftDown = true;
					}
					
					if(buf->data.keyboard.VKey == 0x11) //control
					{
						ctrlDown = true;
					}
					
					if (buf->data.keyboard.VKey == 0x12) //alt
					{
						altDown = true;
					}
				}				
				else if (buf->header.dwType == RIM_TYPEKEYBOARD
					&& buf->data.keyboard.Message == WM_KEYUP
					|| buf->data.keyboard.Message == WM_SYSKEYUP)
				{
					if (buf->data.keyboard.VKey == 0xBD) //minus
					{
						minusDown = false;
					}
					
					if (buf->data.keyboard.VKey == 0x10) //shift
					{
						shiftDown = false;
					}
					
					if (buf->data.keyboard.VKey == 0x11) //control
					{
						ctrlDown = false;
					}

					if (buf->data.keyboard.VKey == 0x12) //alt
					{
						altDown = false;
					}
				}
			
				if (minusDown && ctrlDown && shiftDown && altDown)
				{
					if ((clock() - lastUse) / CLOCKS_PER_SEC > NO_REPEAT_TIME)  //prevents repetition when holding down all keys
					{
						lastUse = clock();

						foregroundWin = GetForegroundWindow(); //gets window text

						if (foregroundWin != NULL)
						{
							
							GetWindowThreadProcessId(foregroundWin, &dwPID);
							handle = OpenProcess(PROCESS_TERMINATE,FALSE,dwPID);

							if (handle)
							{
								
								GetWindowTextW(foregroundWin, wnd_title, 23); //get window title text

								wcscat_s(wnd_title, L"...\0");

								if (TerminateProcess(handle, 0))
								{
									wcscat_s(wnd_title, L" has been closed by Direct Exit.\0");
									wcsncpy_s(trayIcon.szInfo, wnd_title, ARRAYSIZE(trayIcon.szInfo));
								}
								else
								{
									wcscat_s(wnd_title, L" was unable to be closed by Direct Exit.\0");
									wcsncpy_s(trayIcon.szInfo, wnd_title, ARRAYSIZE(trayIcon.szInfo));
								}

								Shell_NotifyIconW(NIM_MODIFY, &trayIcon); //show updated tray notification

								
							}
							else
							{
								MessageBox(NULL, L"A problem occured when trying to get the window handle. Please try again.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
							}
						}
						else 
						{
							MessageBox(NULL, L"Unable to get the forground window for this program. Trying again may fix this issue.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
						}
					}
				}
			}
			else
			{
				MessageBox(NULL, L"Unable to get data from raw input device. Exiting.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
				return 0;
			}
						
			free(buf);
		}
		else
		{
			MessageBox(NULL, L"Unable to get data from raw input device. Exiting.", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
		// free the buffer
		
		break;

	case WM_DESTROY:
		
		Shell_NotifyIconW(NIM_DELETE, &trayIcon);
		PostQuitMessage(0);
		
		break;

	case WM_MYMESSAGE:

		switch (lParam) //icon right click menu
		{
		case NIN_KEYSELECT:
		case WM_RBUTTONUP:
			
			menu = CreatePopupMenu();
			item.cbSize = sizeof(MENUITEMINFO);
			item.fMask = MIIM_STRING | MIIM_ID;
			item.fType = MFT_STRING;
			item.fState = MFS_ENABLED; 
			item.wID = 1;
			item.hSubMenu = NULL;
			item.hbmpChecked = NULL;
			item.hbmpUnchecked = NULL;
			item.dwTypeData = (LPWSTR)L"Exit";
			item.cch = 4;

			GetCursorPos(&screenPos);

			InsertMenuItemW(menu, 0, false, &item);
			SetForegroundWindow(hwnd); //allows close of right click menu
			menuSelection = TrackPopupMenu(menu,
				TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RETURNCMD| TPM_NOANIMATION, 
				screenPos.x, screenPos.y, 0, hwnd, 0);

			DestroyMenu(menu);

			if (menuSelection == 1) //exit is selected.
			{
				menuSelection = 0;				
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}

		break;
		}
		
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;
	RAWINPUTDEVICE rid;
	

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"directExitWindow";

	if (!RegisterClassExW(&wc))
	{
		MessageBox(NULL, L"Unable to register window. Exiting.", L"Error!",MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowExW(0,L"directExitWindow",NULL, 0, 0, 0, 0, 0,NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"Unable to create window.", L"Error!",MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}
	else
	{	
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = hwnd;

		if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			MessageBox(NULL, L"Unable to register raw input device.", L"Error!",MB_ICONEXCLAMATION | MB_OK);
			return 0;
		}
	}


	// the message loop
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}
