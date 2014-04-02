#pragma once

#include <vector>
#include <string>

namespace wtwUtils {
	class SettingsPage
	{
		static const int FLAG_INLINE = 1; // control will be placed directly after previous one (in same line)

		struct Control
		{
			HWND			hWnd; // control handle
			std::wstring	type; // EDIT, CHECKBOX, LABEL
			std::wstring	text; 
			std::wstring	defaultVal; // if nothing read from config, this value will be set
			std::wstring	configId; // name of row in config to get the value, leave empty if not using config file
			int				flags;
			int				x, y, w, h; // control position, internal
			void*			callback; // function called after pressing a button
			
			Control() : hWnd(NULL), x(0), y(0), w(0), h(0), flags(0), callback(NULL) {}
		};

		HWND					hParent;
		HINSTANCE				hInst;
		int						x, y, w, h; // panel size
		int						last_x, last_y;
		std::vector<Control>	ctrls;
		
		SettingsPage() {}
	public:
		SettingsPage(HWND hParent, HINSTANCE hInst, int x, int y, int cx, int cy) :
			hParent(hParent), hInst(hInst), x(x), y(y), w(cx), h(cy), last_x(0), last_y(0)
		{
		}
		
		void add(
			const wchar_t* type, 
			const wchar_t* text, 
			int w, int h = DEFAULT_HEIGHT, 
			int	flags = 0,
			const wchar_t* defaultValue = NULL,
			const wchar_t* configId = NULL,
			void* callback = NULL)
		{
			int x, y;
			
			if(flags & FLAG_INLINE)
			{
				y = last_y;
				x = last_x;
			}
			else
			{
				y = last_y + h;
				x = 0;
			}
			
			std::wstring val;
			if(defaultValue)
				val = defaultValue;
				
			std::wstring confId;
			if(configId)
			{
				confId = configId;
				val = settCtrl.getWStr(configId);
			}
			
			str::wstring txt;
			if(text)
				txt = text;
			
			HWND hCtrl = NULL;
			if(wcscmp(type, "LABEL") == 0)
				hCtrl = CreateWindow(L"STATIC", text, WS_CHILD, x, y, w, h, hParent, 0, hInst, 0);
			else if(wcscmp(type, "EDIT") == 0)
				hCtrl = CreateWindow(L"EDIT", val.c_str(), WS_CHILD, x, y, w, h, hParent, 0, hInst, 0);
			else if(wcscmp(type, "CHECKBOX") == 0)
				hCtrl = CreateWindow(L"BUTTON", text, WS_CHILD|BS_AUTOCHECKBOX, x, y, w, h, hParent, 0, hInst, 0);
				
			if(!hCtrl)
				return;
				
			SendMessage(hCtrl, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), MAKELPARAM(TRUE, 0))
			
			if(wcscmp(type, L"CHECKBOX") == 0 && !val.empty() && val == L"1")
				SendMessage(hCtrl, BM_SETCHECK, TRUE, 0);
				
			last_x = x + w;
			last_y = y;
				
			Control ctrl = {
				hCtrl, 
				std::wstring(type), 
				txt,
				val, 
				confId, 
				flags,
				x, y, w, h,
				callback };
			
			ctrls.push_back(ctrl);
		}	
		
		// --------------------- EVENTS ---------------------
		void onShow() 
		{
			for(unsigned int i=0;i<ctrls.size();i++)
				ShowWindow(ctrls[i].hWnd,SW_SHOW);
		}
		
		void onHide() 
		{
			for(unsigned int i=0;i<ctrls.size();i++)
				ShowWindow(ctrls[i].hWnd,SW_HIDE);
		}
		
		inline void onMove(int x, int y, int cx, int cy) {}
		
		void onApply() 
		{
			wchar_t buff[512] = {0};
			SettingsController& settCtrl = SettingsController::getInstance();
			for(unsigned int i=0; i<ctrls.size(); i++)
			{
				if(ctrls[i].configId.size() == 0)
					continue;
				if(ctrls[i].type == L"EDIT")
				{
					GetWindowText(ctrls[i].hWnd, buff, 511);
					settCtrl.setStr(ctrls[i].configId.c_str(), buff);
				}
				else if(ctrls[i].type == L"CHECKBOX")
				{
					if(SendMessage(checkSend, BM_GETCHECK, 0, 0) == FALSE)
						settCtrl.setInt(ctrls[i].configId.c_str(), 0);
					else
						settCtrl.setInt(ctrls[i].configId.c_str(), 1);
				}
			}
		}
		void onCancel() {}
	};
};
