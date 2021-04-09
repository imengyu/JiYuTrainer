#include "stdafx.h"
#include "HelpWindow.h"
#include "resource.h"
#include "../JiYuTrainer/SysHlp.h"

extern int screenWidth, screenHeight;

HelpWindow::HelpWindow(HWND parentHWnd) : 
	CommonWindow(parentHWnd, 620, 600, L"JiYuTrainerHelpWindow", L"JiYu Trainer Help Window", IDR_HTML_ABOUT) {
	setup_callback();
	init();
	Show();
}
HelpWindow::~HelpWindow()
{
	
}

sciter::value HelpWindow::docunmentComplete()
{
	root = get_root();
	return sciter::value(true);
}

class HelpWindowPageHideSelector : public sciter::dom::callback {
	bool on_element(HELEMENT he)
	{
		sciter::dom::element ele(he);
		ele.set_style_attribute("display", L"none");
		return false;
	}
};
class HelpWindowLinkHideSelector : public sciter::dom::callback {
	bool on_element(HELEMENT he)
	{
		sciter::dom::element ele(he);
		ele.set_style_attribute("color", L"#000");
		return false;
	}
};

bool HelpWindow::on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)
{
	sciter::dom::element ele(he);
	if (type == HYPERLINK_CLICK)
	{
		if (ele.get_attribute("data-type") == L"catalog") {
			HelpWindowLinkHideSelector sel;
			HelpWindowPageHideSelector sel2;
			root.select_elements(&sel2, ".page");
			root.select_elements(&sel, "a[data-type='catalog']");
			sciter::dom::element currPage = root.get_element_by_id(ele.get_attribute("data-id").c_str());
			if (currPage)
				currPage.set_style_attribute("display", L"block");
			ele.set_style_attribute("color", L"#ff8b0d");
		}
		else if (ele.get_attribute("href").find_first_of(L"http") == 0) {
			SysHlp::OpenUrl(ele.get_attribute("href").c_str());
		}
	}
	return false;
}
LRESULT HelpWindow::onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled)
{
	switch (message)
	{
	case WM_MY_FORCE_HIDE: Hide(); *handled = TRUE; return 0;
	}
	return 0;
}


