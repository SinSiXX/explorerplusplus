// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "CustomizeColorsDialog.h"
#include "ColorRuleDialog.h"
#include "Explorer++_internal.h"
#include "IconResourceLoader.h"
#include "MainResource.h"
#include "ResourceHelper.h"
#include "../Helper/Helper.h"
#include "../Helper/ListViewHelper.h"
#include "../Helper/Macros.h"
#include "../Helper/WindowHelper.h"

const TCHAR CustomizeColorsDialogPersistentSettings::SETTINGS_KEY[] = _T("CustomizeColors");

CustomizeColorsDialog::CustomizeColorsDialog(HINSTANCE hInstance, HWND hParent,
	IExplorerplusplus *expp, std::vector<NColorRuleHelper::ColorRule_t> *pColorRuleList) :
	BaseDialog(hInstance, IDD_CUSTOMIZECOLORS, hParent, true),
	m_expp(expp),
	m_pColorRuleList(pColorRuleList)
{
	m_persistentSettings = &CustomizeColorsDialogPersistentSettings::GetInstance();
}

INT_PTR CustomizeColorsDialog::OnInitDialog()
{
	HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);

	SetWindowTheme(hListView,L"Explorer",NULL);

	ListView_SetExtendedListViewStyleEx(hListView,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES,
		LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	TCHAR szTemp[128];

	LoadString(GetInstance(),IDS_CUSTOMIZE_COLORS_COLUMN_DESCRIPTION,
		szTemp,SIZEOF_ARRAY(szTemp));
	LVCOLUMN lvColumn;
	lvColumn.mask		= LVCF_TEXT;
	lvColumn.pszText	= szTemp;
	ListView_InsertColumn(hListView,0,&lvColumn);

	LoadString(GetInstance(),IDS_CUSTOMIZE_COLORS_COLUMN_FILENAME_PATTERN,
		szTemp,SIZEOF_ARRAY(szTemp));
	lvColumn.mask		= LVCF_TEXT;
	lvColumn.pszText	= szTemp;
	ListView_InsertColumn(hListView,1,&lvColumn);

	LoadString(GetInstance(),IDS_CUSTOMIZE_COLORS_COLUMN_ATTRIBUTES,
		szTemp,SIZEOF_ARRAY(szTemp));
	lvColumn.mask		= LVCF_TEXT;
	lvColumn.pszText	= szTemp;
	ListView_InsertColumn(hListView,2,&lvColumn);

	RECT rc;
	GetClientRect(hListView,&rc);
	SendMessage(hListView,LVM_SETCOLUMNWIDTH,0,GetRectWidth(&rc) / 3);
	SendMessage(hListView,LVM_SETCOLUMNWIDTH,1,GetRectWidth(&rc) / 3);
	SendMessage(hListView,LVM_SETCOLUMNWIDTH,2,GetRectWidth(&rc) / 3);

	int iItem = 0;

	for(const auto &ColorRule : *m_pColorRuleList)
	{
		InsertColorRuleIntoListView(hListView,ColorRule,iItem++);
	}

	SetFocus(hListView);

	m_persistentSettings->RestoreDialogPosition(m_hDlg,true);

	return 0;
}

wil::unique_hicon CustomizeColorsDialog::GetDialogIcon(int iconWidth, int iconHeight) const
{
	return m_expp->GetIconResourceLoader()->LoadIconFromPNGAndScale(Icon::CustomizeColors, iconWidth, iconHeight);
}

void CustomizeColorsDialog::GetResizableControlInformation(BaseDialog::DialogSizeConstraint &dsc,
	std::list<ResizableDialog::Control_t> &ControlList)
{
	dsc = BaseDialog::DIALOG_SIZE_CONSTRAINT_NONE;

	ResizableDialog::Control_t Control;

	Control.iID = IDC_LISTVIEW_COLORRULES;
	Control.Type = ResizableDialog::TYPE_RESIZE;
	Control.Constraint = ResizableDialog::CONSTRAINT_NONE;
	ControlList.push_back(Control);

	Control.iID = IDC_BUTTON_DELETE;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_X;
	ControlList.push_back(Control);

	Control.iID = IDC_BUTTON_MOVEDOWN;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_X;
	ControlList.push_back(Control);

	Control.iID = IDC_BUTTON_MOVEUP;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_X;
	ControlList.push_back(Control);

	Control.iID = IDC_BUTTON_EDIT;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_X;
	ControlList.push_back(Control);

	Control.iID = IDC_BUTTON_NEW;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_X;
	ControlList.push_back(Control);

	Control.iID = IDOK;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_NONE;
	ControlList.push_back(Control);

	Control.iID = IDCANCEL;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_NONE;
	ControlList.push_back(Control);

	Control.iID = IDC_GRIPPER;
	Control.Type = ResizableDialog::TYPE_MOVE;
	Control.Constraint = ResizableDialog::CONSTRAINT_NONE;
	ControlList.push_back(Control);
}

void CustomizeColorsDialog::InsertColorRuleIntoListView(HWND hListView,const NColorRuleHelper::ColorRule_t &ColorRule,
	int iIndex)
{
	TCHAR szTemp[512];

	StringCchCopy(szTemp,SIZEOF_ARRAY(szTemp),
		ColorRule.strDescription.c_str());

	LVITEM lvItem;
	lvItem.mask		= LVIF_TEXT;
	lvItem.pszText	= szTemp;
	lvItem.iItem	= iIndex;
	lvItem.iSubItem	= 0;
	int iActualIndex = ListView_InsertItem(hListView,&lvItem);

	if(iActualIndex != -1)
	{
		StringCchCopy(szTemp,SIZEOF_ARRAY(szTemp),ColorRule.strFilterPattern.c_str());
		ListView_SetItemText(hListView,iActualIndex,1,szTemp);

		BuildFileAttributeString(ColorRule.dwFilterAttributes,szTemp,SIZEOF_ARRAY(szTemp));
		ListView_SetItemText(hListView,iActualIndex,2,szTemp);
	}
}

INT_PTR CustomizeColorsDialog::OnCommand(WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch(LOWORD(wParam))
	{
	case IDC_BUTTON_NEW:
		OnNew();
		break;

	case IDC_BUTTON_EDIT:
		OnEdit();
		break;

	case IDC_BUTTON_MOVEUP:
		OnMove(TRUE);
		break;

	case IDC_BUTTON_MOVEDOWN:
		OnMove(FALSE);
		break;

	case IDC_BUTTON_DELETE:
		OnDelete();
		break;

	case IDOK:
		OnOk();
		break;

	case IDCANCEL:
		OnCancel();
		break;
	}

	return 0;
}

INT_PTR CustomizeColorsDialog::OnNotify(NMHDR *pnmhdr)
{
	switch(pnmhdr->code)
	{
	case NM_DBLCLK:
		{
			NMITEMACTIVATE *pnmItem = reinterpret_cast<NMITEMACTIVATE *>(pnmhdr);

			if(pnmItem->iItem != -1)
			{
				EditColorRule(pnmItem->iItem);
			}
		}
		break;
	}

	return 0;
}

INT_PTR CustomizeColorsDialog::OnClose()
{
	EndDialog(m_hDlg,0);
	return 0;
}

void CustomizeColorsDialog::SaveState()
{
	m_persistentSettings->SaveDialogPosition(m_hDlg);

	m_persistentSettings->m_bStateSaved = TRUE;
}

void CustomizeColorsDialog::OnNew()
{
	HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);

	NColorRuleHelper::ColorRule_t ColorRule;

	ColorRuleDialog colorRuleDialog(GetInstance(), m_hDlg, &ColorRule, FALSE);

	INT_PTR iRet = colorRuleDialog.ShowModalDialog();

	if(iRet == 1)
	{
		m_pColorRuleList->push_back(ColorRule);

		int nItems = ListView_GetItemCount(hListView);
		InsertColorRuleIntoListView(hListView,ColorRule,nItems);
	}

	SetFocus(m_hDlg);
}

void CustomizeColorsDialog::OnEdit()
{
	HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);
	int iSelected = ListView_GetNextItem(hListView,-1,LVNI_ALL|LVNI_SELECTED);

	if(iSelected != -1)
	{
		EditColorRule(iSelected);
	}

	SetFocus(m_hDlg);
}

void CustomizeColorsDialog::EditColorRule(int iSelected)
{
	ColorRuleDialog ColorRuleDialog(GetInstance(), m_hDlg, &(*m_pColorRuleList)[iSelected], TRUE);

	INT_PTR iRet = ColorRuleDialog.ShowModalDialog();

	if(iRet == 1)
	{
		HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);

		TCHAR szTemp[512];

		StringCchCopy(szTemp,SIZEOF_ARRAY(szTemp),
			(*m_pColorRuleList)[iSelected].strDescription.c_str());
		ListView_SetItemText(hListView,iSelected,0,szTemp);

		StringCchCopy(szTemp,SIZEOF_ARRAY(szTemp),
			(*m_pColorRuleList)[iSelected].strFilterPattern.c_str());
		ListView_SetItemText(hListView,iSelected,1,szTemp);

		BuildFileAttributeString((*m_pColorRuleList)[iSelected].dwFilterAttributes,
			szTemp,SIZEOF_ARRAY(szTemp));
		ListView_SetItemText(hListView,iSelected,2,szTemp);
	}
}

void CustomizeColorsDialog::OnMove(BOOL bUp)
{
	HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);
	int iSelected = ListView_GetNextItem(hListView,-1,LVNI_SELECTED);

	if(iSelected != -1)
	{
		int iSwap;

		if(bUp)
		{
			if(iSelected == 0)
				return;

			iSwap = iSelected - 1;
		}
		else
		{
			if(iSelected == static_cast<int>((m_pColorRuleList->size() - 1)))
				return;

			iSwap = iSelected + 1;
		}

		auto itrSelected = m_pColorRuleList->begin();
		std::advance(itrSelected,iSelected);

		auto itrSwap = m_pColorRuleList->begin();
		std::advance(itrSwap,iSwap);

		std::iter_swap(itrSelected,itrSwap);

		NListView::ListView_SwapItems(hListView,iSelected,iSwap,FALSE);
	}
}

void CustomizeColorsDialog::OnDelete()
{
	HWND hListView = GetDlgItem(m_hDlg,IDC_LISTVIEW_COLORRULES);
	int iSelected = ListView_GetNextItem(hListView,-1,LVNI_SELECTED);

	if(iSelected != -1)
	{
		TCHAR szInfoMsg[128];
		LoadString(GetInstance(),IDS_COLORRULE_DELETE,
			szInfoMsg,SIZEOF_ARRAY(szInfoMsg));

		int iRes = MessageBox(m_hDlg,szInfoMsg,
			NExplorerplusplus::APP_NAME,MB_YESNO|MB_ICONINFORMATION|MB_DEFBUTTON2);

		if(iRes == IDYES)
		{
			int nItems = static_cast<int>(m_pColorRuleList->size());

			auto itr = m_pColorRuleList->begin();
			std::advance(itr,iSelected);
			m_pColorRuleList->erase(itr);

			ListView_DeleteItem(hListView,iSelected);

			if(iSelected == (nItems - 1))
				iSelected--;

			NListView::ListView_SelectItem(hListView,iSelected,TRUE);
		}

		SetFocus(hListView);
	}
}

void CustomizeColorsDialog::OnOk()
{
	EndDialog(m_hDlg,1);
}

void CustomizeColorsDialog::OnCancel()
{
	EndDialog(m_hDlg,0);
}

CustomizeColorsDialogPersistentSettings::CustomizeColorsDialogPersistentSettings() :
DialogSettings(SETTINGS_KEY)
{

}

CustomizeColorsDialogPersistentSettings& CustomizeColorsDialogPersistentSettings::GetInstance()
{
	static CustomizeColorsDialogPersistentSettings sfadps;
	return sfadps;
}