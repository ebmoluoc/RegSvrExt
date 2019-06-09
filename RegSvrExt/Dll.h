#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////

class Dll
{
public:
	Dll();
	BOOL Initialize(HMODULE handle);
	HRESULT CanUnloadNow();
	void AddRef();
	void Release();

	HMODULE Handle() const;
	LPCWSTR Path() const;

private:
	ULONG refCount_;
	HMODULE handle_;
	WCHAR path_[MAX_PATH];
};
