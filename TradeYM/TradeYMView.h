/************************************************************************
 * Copyright(c) 2010, One Unified. All rights reserved.                 *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

// TradeYMView.h : interface of the CTradeYMView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Process.h"

class CTradeYMView : public CDialogImpl<CTradeYMView>
{
public:
	enum { IDD = IDD_TRADEYM_FORM };

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(CTradeYMView)
    COMMAND_HANDLER(IDC_BTN_IBCONNECT, BN_CLICKED, OnBnClickedBtnIbconnect)
    COMMAND_HANDLER(IDC_BTN_IBDISCONNECT, BN_CLICKED, OnBnClickedBtnIbdisconnect)
    COMMAND_HANDLER(IDC_BTN_BUY, BN_CLICKED, OnBnClickedBtnBuy)
    COMMAND_HANDLER(IDC_BTN_SELL, BN_CLICKED, OnBnClickedBtnSell)
  END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  CTradeYMView( void );
  ~CTradeYMView( void );

protected:
private:

  CProcess m_process;

public:
  LRESULT OnBnClickedBtnIbconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnBnClickedBtnIbdisconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnBnClickedBtnBuy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnBnClickedBtnSell(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};