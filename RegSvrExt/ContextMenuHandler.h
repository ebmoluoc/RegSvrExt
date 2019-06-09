#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////

class ContextMenuHandler : public IShellExtInit, public IContextMenu
{
public:
	ContextMenuHandler(Dll& dll);
	~ContextMenuHandler();

	IFACEMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, LPDATAOBJECT pdtobj, HKEY hkeyProgID);

	IFACEMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
	IFACEMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pReserved, LPSTR pszName, UINT cchMax);

private:
	Dll& dll_;
	ULONG refCount_;
	HBITMAP hBitmapMenu_;
	hlp::DropFilesList pathList_;
};
