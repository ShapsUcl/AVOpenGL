//
// Copyright (C) Singular Inversions Inc. 2011
//
// Authors:     Andrew Beatty
// Created:     March 17, 2011
//
// * The main window paints a background color to make it easy to avoid holes.
// * The flicker resulting from this is minimized by 'WS_CLIPCHILDREN'.
// * The tab control appears to do something similar to its entire client area.
// The standard way of handling WM_SIZE is to paint sub-wins as they are resized.
// This works because windows defers creating WM_PAINT messages when a WM_RESIZE
//      message is sent to a wndProc until that wndProc has returned.
// If a wndProc needs to call a bunch of MoveWindows while handling a different
//      message (for example a split divider has moved), WM_PAINT sending is not
//      deferred, which causes fighting as overlapping windows try to draw.
// Calling a MoveWindow on itself doesn't work because the parent window itself
//      hasn't moved and Windows will thus ignore the call.
// So the solution is to turn off repainting in all resize handlers, then trigger
//      a repaint after all resizing is done.

#include "stdafx.h"

#include "FgMeshViewView.h"
#include "FgMatrixC.hpp"
#include "FgGuiApiBase.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMetaFormat.hpp"
#include "FgHex.hpp"
#include "FGMeshView.h"
#include "MainFrm.h"

using namespace std;

// **************************************************************************************
// **************************************************************************************

FgGuiWinStatics s_fgGuiWin;

static wchar_t fgGuiWinMain[] = L"FgGuiWinMain";

struct  FgGuiWinMain
{
    FgString                    m_title;
    FgString                    m_store;    // Base filename for state storage
    FgSharedPtr<FgGuiOsBase>    m_win;
    FgVect2UI                   m_size;     // Current size of client area
    vector<HANDLE>              eventHandles;   // Client event handles to trigger message loop
    vector<void(*)()>           eventHandlers;  // Respective event handlers
    vector<FgGuiKeyHandle>      keyHandlers;

    void
    start()
    {
        // Load common controls DLL:
        INITCOMMONCONTROLSEX    icc;
        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_BAR_CLASSES;
        InitCommonControlsEx(&icc);

        // The following will give us a handle to the current instance aka 'module',
        // which corresponds to the binary file in which this code is compiled
        // (ie. EXE or a DLL):
        WNDCLASSEX  wcl;
        wcl.cbSize = sizeof(wcl);
        if (GetClassInfoEx(s_fgGuiWin.hinst,(LPCSTR)fgGuiWinMain,&wcl) == 0)
        {
            wcl.style = CS_HREDRAW | CS_VREDRAW;
            wcl.lpfnWndProc = &fgStatWndProc<FgGuiWinMain>;
            wcl.cbClsExtra = 0;
            wcl.cbWndExtra = sizeof(void *);
            wcl.hInstance = s_fgGuiWin.hinst;
            wcl.hIcon = LoadIcon(NULL,IDI_APPLICATION);
            wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
            // Background painting the whole application makes it easy to avoid holes,
            // and flicker is minimized by the use of 'WS_CLIPCHILDREN' below (very important).
            wcl.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
            wcl.lpszMenuName = NULL;
            wcl.lpszClassName = (LPCSTR) fgGuiWinMain;
            wcl.hIconSm = NULL;
            FGASSERTWIN(RegisterClassEx(&wcl));
        }

        // Retrieve previously saved window state if present and valid:

        // posDims: col vec 0 is position (upper left corner in  windows screen coordinates),
        // col vec 1 is size. Windows screen coordinates:
        // x - right, y - down, origin - upper left corner of MAIN screen.
        FgMatrix22I     posDims(CW_USEDEFAULT,1200,CW_USEDEFAULT,800),
                        pdTmp;
        if (fgLoadXml(m_store+".xml",pdTmp,false)) {
            FgVect2I    pdAbs = fgAbs(pdTmp.subMatrix<2,1>(0,0));
            FgVect2I    pdMin = FgVect2I(m_win->getMinSize());
            if ((pdAbs[0] < 32000) &&   // Windows internal representation limits
                (pdAbs[1] < 32000) &&
                (pdTmp[1] >= pdMin[0]) && (pdTmp[1] < 32000) &&
                (pdTmp[3] >= pdMin[1]) && (pdTmp[3] < 32000))
                posDims = pdTmp; }

        // CreateWindowEx sends WM_CREATE and certain other messages before returning.
        // This is done so that the caller can send messages to the child window immediately
        // after calling this function.
        s_fgGuiWin.hwndMain =
            CreateWindowEx(0,
                (LPCSTR)fgGuiWinMain, 
                (LPCSTR)m_title.as_wstring().c_str(),
                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                posDims[0],posDims[2],
                posDims[1],posDims[3],
                NULL,NULL,
                // Contrary to MSDN docs, this is used on all WinOSes to disambiguate
                // the class name over different modules [Raymond Chen]:
                s_fgGuiWin.hinst,
                this);      // Value to be sent as argument with WM_NCCREATE message
        FGASSERTWIN(s_fgGuiWin.hwndMain);

        // Upddate the windows to create any dynamic ones (all nodes still marked as changed):
        SendMessage(s_fgGuiWin.hwndMain,WM_USER,0,0);

        // Now that dynamic sub-windows are created we can call this:
        ShowWindow(s_fgGuiWin.hwndMain,SW_SHOWNORMAL);

        // This call ultimately results in windows sending the first WM_PAINT message:
        //UpdateWindow(s_fgGuiWin.hwndMain);

        MSG         msg;
        HANDLE      dummyEvent = INVALID_HANDLE_VALUE;
        HANDLE      *eventsPtr = (eventHandles.empty() ? &dummyEvent : &eventHandles[0]);
        for (;;) {
            BOOL        ret = MsgWaitForMultipleObjects(DWORD(eventHandles.size()),eventsPtr,FALSE,INFINITE,QS_ALLEVENTS);
            if (ret == WAIT_FAILED) {
                DWORD   err = GetLastError();
                fgout << fgnl << "MsgWaitForMultipleObjects failed with last error: " << err;
            }
            int         idx = int(ret) - int(WAIT_OBJECT_0);
            if ((idx >= 0) && (idx < int(eventHandles.size()))) {
                eventHandlers[idx]();
                g_gg.updateScreen();
            }
            // Get all messages here since MsgWaitForMultipleObjects waits for NEW messages:
            while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
                // WM_QUIT is only sent to main message loop after WM_DESTROY has been
                // sent and processed by all sub-windows:
                if (msg.message == WM_QUIT)
                    return;
                //static size_t cnt = 0;
                //fgout << cnt++ << "   " << fgAsHex(msg.message) << fgnl;
                // Translates multi-key combos into appropriate unicode. Intercept application-wide
                // special combos before calling this:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    LRESULT
    wndProc(
        HWND    hwnd,
        UINT    msg,
        WPARAM  wParam,
        LPARAM  lParam)
    {
        if (msg == WM_CREATE) {
            m_win->create(hwnd,0,m_store+"_s");
            return 0;
        }
        // Enforce minimum windows sizes. This only works on the main window so
        // we must calculate the minimum size from that of all sub-windows:
        else if (msg == WM_GETMINMAXINFO) {
            FgVect2UI       min = m_win->getMinSize() + fgNcSize(hwnd);
            MINMAXINFO *    mmi = (MINMAXINFO*)lParam;
            POINT           pnt;
            pnt.x = min[0];
            pnt.y = min[1];
            mmi->ptMinTrackSize = pnt;
            return 0;
        }
        else if (msg == WM_SIZE) {  // Doesn't send 0 size like sub-windows get at creation.
            m_size[0] = LOWORD(lParam);
            m_size[1] = HIWORD(lParam);
            //fgout << "WM_SIZE: " << m_size << endl;
            // Size zero is sent when window is minimized, which for unknown reasons causes windows
            // to not repaint after restore, even after resizing to non-zero and calling Invalidate
            // on entire window. So we must avoid zero sizing:
            if (m_size[0]*m_size[1] > 0)
                m_win->moveWindow(FgVect2I(0),FgVect2I(m_size));
            return 0;
        }
        // R. Chen: WM_WINDOWPOSCHANGED catches all possible size/move changes (added later).
        // Sent for each move and also for maximize/minimize/restore:
        //else if (msg == WM_MOVE)
        //    fgout << "WM_MOVE" << endl;
        // Only sent after initial showwindow call:
        //else if (msg == WM_SHOWWINDOW)
        //    fgout << "WM_SHOWWINDOW" << endl;
        //else if (msg == WM_QUERYOPEN)     // Queries window state before restore
        //    fgout << "WM_QUERYOPEN" << endl;
        else if (msg == WM_CHAR) {
            wchar_t     wkey = wchar_t(wParam);
            for (size_t ii=0; ii<keyHandlers.size(); ++ii) {
                FgGuiKeyHandle  kh = keyHandlers[ii];
                if (wkey == wchar_t(kh.key))
                    kh.handler();
            }
            return 0;
        }
        // This msg is sent by FgGuiGraph::updateScreen() which is called whenever an 
        // input has been changed:
        else if (msg == WM_USER) {
            m_win->updateIfChanged();
            return 0;
        }
        else if (msg == WM_DESTROY)         // User is closing application
        {
            WINDOWPLACEMENT     wp;
            wp.length = sizeof(wp);
            // Don't use GetWindowRect here as it's affected by minimization and maximization:
            FGASSERTWIN(GetWindowPlacement(hwnd,&wp));
            const RECT &        rect = wp.rcNormalPosition;
            FgMatrix22I dims(rect.left,rect.right-rect.left,rect.top,rect.bottom-rect.top);
            fgSaveXml(m_store+".xml",dims,false);
            m_win->saveState();
            PostQuitMessage(0);     // Sends WM_QUIT which ends msg loop
            return 0;
        }
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }
};

// **************************************************************************************
// **************************************************************************************


// **************************************************************************************
// **************************************************************************************

void
FgGuiGraph::updateScreen()
{
    changedSnapshot();
    SendMessage(s_fgGuiWin.hwndMain,WM_USER,0,0);
}

void
FgGuiGraph::quit()
{
    // When a WM_CLOSE message is generated by the user clicking the close 'X' button,
    // the default (DefWindowProc) is to do this, which of course generates a WM_DESTROY:
    DestroyWindow(s_fgGuiWin.hwndMain);
}
FgGuiGraph  g_gg;


// **************************************************************************************
// **************************************************************************************

FgVect2I
fgScreenPos(HWND hwnd,LPARAM lParam)
{
    POINT       coord;
    coord.x = GET_X_LPARAM(lParam);
    coord.y = GET_Y_LPARAM(lParam);
    FGASSERTWIN(ClientToScreen(hwnd,&coord));
    return FgVect2I(coord.x,coord.y);
}

FgVect2UI
fgNcSize(HWND hwnd)
{
    // Calculate how much space Windows is taking for the NC area.
    // I was unable to get similar values using GetSystemMetrics (eg. for
    // main windows NC size was (8,27) while GSM reported SM_C*BORDER=1
    // and SM_CYCAPTION=19
    RECT        rectW,rectC;
    FGASSERTWIN(GetWindowRect(hwnd,&rectW));
    FGASSERTWIN(GetClientRect(hwnd,&rectC));
    return
        FgVect2UI(
            (rectW.right-rectW.left)-(rectC.right-rectC.left),
            (rectW.bottom-rectW.top)-(rectC.bottom-rectC.top));
}

#include "FgGuiApiTabs.hpp"

struct  FgGuiWinTabs : public FgGuiOsBase
{
    FgGuiApiTabs                    m_api;
    HWND                            m_tabHwnd;
    HWND                            hwndThis;
    FgGuiOsPtrs                     m_panes;
    uint                            m_currPane;
    FgVect2I                        m_client;
    RECT                            m_dispArea;
    FgString                        m_store;

    FgGuiWinTabs(const FgGuiApiTabs & api)
        : m_api(api)
    {
        FGASSERT(m_api.tabs.size()>0);
        for (size_t ii=0; ii<m_api.tabs.size(); ++ii)
            m_panes.push_back(api.tabs[ii].win->getInstance());
        m_currPane = 0;
    }

    virtual void
    create(HWND parentHwnd,int ident,const FgString & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "FgGuiWinTabs::create " << m_api.tabs[0].label << fgpush;
        m_store = store;
        uint        cp;
        if (fgLoadXml(m_store+".xml",cp,false))
            if (cp < m_panes.size())
                m_currPane = cp;
        FgCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        fgCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual FgVect2UI
    getMinSize() const
    {
        FgVect2UI   max(0);
        for (size_t ii=0; ii<m_panes.size(); ++ii) {
            const FgGuiTab &    tab = m_api.tabs[ii];
            FgVect2UI           pad(tab.padLeft+tab.padRight,tab.padTop+tab.padBottom);
            max = fgMax(max,m_panes[ii]->getMinSize()+pad);
        }
        return max + FgVect2UI(0,37);
    }

    virtual FgVect2B
    wantStretch() const
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            if (m_panes[ii]->wantStretch()[0])
                return FgVect2B(true,true);
        return FgVect2B(false,true);
    }

    virtual void
    updateIfChanged()
    {m_panes[m_currPane]->updateIfChanged(); }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "FgGuiWinTabs::moveWindow " << lo << " , " << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void
    showWindow(bool s)
    {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }

    virtual void
    saveState()
    {
        fgSaveXml(m_store+".xml",m_currPane,false);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->saveState();
    }

    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            {
//fgout << fgnl << "FgGuiWinTabs::WM_CREATE " << m_api.tabs[0].label;
                hwndThis = hwnd;
                m_tabHwnd = 
                    CreateWindowEx(0,
                        WC_TABCONTROL,
                        "",
                        WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH,
                        0,0,0,0,
                        hwnd,
                        NULL,
                        s_fgGuiWin.hinst,
                        NULL);
                TCITEM  tc = {0};
                tc.mask = TCIF_TEXT;
                for (size_t ii=0; ii<m_panes.size(); ++ii) {
                    wstring     wstr = m_api.tabs[ii].label.as_wstring();
                    wstr += wchar_t(0);
                    tc.pszText = (LPSTR) &wstr[0];
                    TabCtrl_InsertItem(m_tabHwnd,ii,&tc);
                    m_panes[ii]->create(hwnd,int(ii),m_store+"_"+fgToString(ii),NULL,false);
                }
                SendMessage(m_tabHwnd,TCM_SETCURSEL,m_currPane,0);
                m_panes[m_currPane]->showWindow(true);
                return 0;
            }
        case WM_SIZE:
            {
                m_client = FgVect2I(LOWORD(lParam),HIWORD(lParam));
//fgout << fgnl << "FgGuiWinTabs::WM_SIZE " << m_api.tabs[0].label << " : " << m_client;
                if (m_client[0] * m_client[1] > 0)
                    resize(hwnd);
                return 0;
            }
        case WM_NOTIFY:
            {
                LPNMHDR lpnmhdr = (LPNMHDR)lParam;
                if (lpnmhdr->code == TCN_SELCHANGE) {
                    int     idx = int(SendMessage(m_tabHwnd,TCM_GETCURSEL,0,0));
                    // This can apparently be -1 for 'no tab selected':
                    if ((idx >= 0) && (size_t(idx) < m_panes.size())) {
                        if (uint(idx) != m_currPane) {
                            m_panes[m_currPane]->showWindow(false);
                            m_currPane = uint(idx);
                            m_panes[m_currPane]->showWindow(true);
                            resizeCurrPane();
                            m_panes[m_currPane]->updateIfChanged();
                            // This is needed only for the first time a tab is selected, and only
                            // for *some* of the sub-sub windows. No idea why:
                            InvalidateRect(hwnd,&m_dispArea,FALSE);
                        }
                    }
                }
                return 0;
            }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }

    void
    resizeCurrPane()
    {
        const FgGuiTab &    tab = m_api.tabs[m_currPane];
        FgVect2I    lo(m_dispArea.left + tab.padLeft, m_dispArea.top + tab.padTop),
                    hi(m_dispArea.right - tab.padRight,m_dispArea.bottom - tab.padBottom),
                    sz = hi - lo;
        m_panes[m_currPane]->moveWindow(lo,sz);
    }

    void
    resize(HWND)
    {
        MoveWindow(m_tabHwnd,0,0,m_client[0],m_client[1],TRUE);
        m_dispArea.left = 0;
        m_dispArea.top = 0;
        m_dispArea.right = m_client[0];
        m_dispArea.bottom = m_client[1];
        SendMessage(m_tabHwnd,
            TCM_ADJUSTRECT,
            NULL,               // Give me the display area for this window area:
            LPARAM(&m_dispArea));
        resizeCurrPane();
    }
};

void fgGuiChildStart(
    const FgString &            title,
    FgSharedPtr<FgGuiApiBase>   def,
    const FgString &            storeDir,
    const FgGuiOptions &        opts)
{
	TRACE("In fgGuiChildStart: title is %s\n", title); 
	CFGMeshViewApp *pApp=(CFGMeshViewApp *)AfxGetApp();
	CMainFrame *pFrame = (CMainFrame *) pApp->m_pMainWnd;
	CFGMeshViewView * pView = (CFGMeshViewView *) pFrame->GetSafeHwnd();
    // Must set all nodes to updated so that their values get the initial update (won't be
    // necessary once all window implementations create all sub-windows in their update()
    // rather than constructor):
    g_gg.setAllChanged();
    //s_fgGuiWin.hinst = GetModuleHandle(NULL);
    //FgGuiWinMain    gui = 	pView->FgGui;
 	//s_fgGuiWin.hwndMain = pView->GetSafeHwnd(); 
	const FgGuiApiTabs api();
//	FgGuiWinTabs gui = FgGuiWinTabs(&api); 
//	gui.m_client = def->getInstance();
 
	//gui.m_store = storeDir + "guiWindowState";
 //   gui.m_win = def->getInstance();
 //   for (size_t ii=0; ii<opts.events.size(); ++ii) {
 //       gui.eventHandles.push_back(opts.events[ii].handle);
 //       gui.eventHandlers.push_back(opts.events[ii].handler);
 //   }
 //   gui.keyHandlers = opts.keyHandlers;
 //   gui.start(pFrame->GetSafeHwnd());
    g_gg.saveInputs();
    //s_fgGuiWin.hwndMain = 0;    // This value may be sent to dialog boxes as owner hwnd.
}

void
fgGuiImplStart(
    const FgString &            title,
    FgSharedPtr<FgGuiApiBase>   def,
    const FgString &            storeDir,
    const FgGuiOptions &        opts)
{
    // Must set all nodes to updated so that their values get the initial update (won't be
    // necessary once all window implementations create all sub-windows in their update()
    // rather than constructor):
    g_gg.setAllChanged();
    s_fgGuiWin.hinst = GetModuleHandle(NULL);
    FgGuiWinMain    gui;
    gui.m_title = title;
    gui.m_store = storeDir + "guiWindowState";
    gui.m_win = def->getInstance();
    for (size_t ii=0; ii<opts.events.size(); ++ii) {
        gui.eventHandles.push_back(opts.events[ii].handle);
        gui.eventHandlers.push_back(opts.events[ii].handler);
    }
    gui.keyHandlers = opts.keyHandlers;
    gui.start();
    g_gg.saveInputs();
    s_fgGuiWin.hwndMain = 0;    // This value may be sent to dialog boxes as owner hwnd.
}
