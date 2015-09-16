//
// Copyright (C) Singular Inversions Inc. 2011
//
// Authors:     Andrew Beatty
// Created:     April 12, 2011
//

#include "stdafx.h"

#include "FgGuiApiTabs.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"
#include "FgMetaFormat.hpp"
#include "FgAlgs.hpp"

using namespace std;

// **************************************************************************************
// **************************************************************************************

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
                        L"",
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
                    tc.pszText = &wstr[0];
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

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiTabs & api)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinTabs(api)); }
