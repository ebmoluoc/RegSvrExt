#include "pch.h"
#include "ContextMenuHandler.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

ContextMenuHandler::ContextMenuHandler(Dll& dll) : dll_{ dll }, refCount_{ 1 }
{
	dll_.AddRef();

	hBitmapMenu_ = hlp::BitmapFromIconResource(dll_.Handle(), IDI_ICON_MENU, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

ContextMenuHandler::~ContextMenuHandler()
{
	if (hBitmapMenu_ != nullptr)
	{
		DeleteObject(hBitmapMenu_);
	}

	dll_.Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP ContextMenuHandler::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
	if (ppvObject == nullptr)
	{
		return E_POINTER;
	}
	else if (IsEqualIID(riid, IID_IContextMenu))
	{
		*ppvObject = static_cast<LPCONTEXTMENU>(this);
	}
	else if (IsEqualIID(riid, IID_IShellExtInit))
	{
		*ppvObject = static_cast<LPSHELLEXTINIT>(this);
	}
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	this->AddRef();

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP_(ULONG) ContextMenuHandler::AddRef()
{
	return ++refCount_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP_(ULONG) ContextMenuHandler::Release()
{
	auto rc{ --refCount_ };
	if (rc == 0)
	{
		delete this;
	}
	return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP ContextMenuHandler::Initialize(PCIDLIST_ABSOLUTE pidlFolder, LPDATAOBJECT pdtobj, HKEY hkeyProgID)
{
	UNREFERENCED_PARAMETER(pidlFolder);
	UNREFERENCED_PARAMETER(hkeyProgID);

	if (pdtobj == nullptr)
	{
		return E_INVALIDARG;
	}

	if (pathList_.Load(pdtobj))
	{
		if (!pathList_.IsMultiItems())
		{
			return S_OK;
		}
	}

	return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP ContextMenuHandler::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	UNREFERENCED_PARAMETER(idCmdLast);

	if ((uFlags & CMF_DEFAULTONLY) != 0)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	}

	auto hSubmenu{ CreatePopupMenu() };
	if (hSubmenu == nullptr)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	}

	auto idCmd{ idCmdFirst };

	if (!hlp::AddMenuItem(hSubmenu, SUBMENU_REGISTER, SUBMENU_REGISTER_W, idCmd++, nullptr, hBitmapMenu_) ||
		!hlp::AddMenuItem(hSubmenu, SUBMENU_UNREGISTER, SUBMENU_UNREGISTER_W, idCmd++, nullptr, hBitmapMenu_) ||
		!hlp::AddMenuItem(hmenu, indexMenu, PRODUCT_NAME_W, idCmd++, hSubmenu, hBitmapMenu_))
	{
		DestroyMenu(hSubmenu);
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, idCmd - idCmdFirst);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP ContextMenuHandler::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	if (HIWORD(pici->lpVerb) != 0)
	{
		return E_FAIL;
	}

	auto filePath{ pathList_.GetItems()[0] };
	auto arguments{ hlp::EscapeArgument(filePath) };

	switch (LOWORD(pici->lpVerb))
	{
	case SUBMENU_REGISTER:
		ShellExecuteW(nullptr, L"runas", L"regsvr32.exe", arguments.c_str(), nullptr, SW_SHOW);
		break;
	case SUBMENU_UNREGISTER:
		arguments.insert(0, L"/u ");
		ShellExecuteW(nullptr, L"runas", L"regsvr32.exe", arguments.c_str(), nullptr, SW_SHOW);
		break;
	default:
		return E_FAIL;
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

IFACEMETHODIMP ContextMenuHandler::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax)
{
	UNREFERENCED_PARAMETER(idCmd);
	UNREFERENCED_PARAMETER(uType);
	UNREFERENCED_PARAMETER(pReserved);
	UNREFERENCED_PARAMETER(pszName);
	UNREFERENCED_PARAMETER(cchMax);

	return E_NOTIMPL;
}
