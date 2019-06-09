#include "pch.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

static Dll dll{};

///////////////////////////////////////////////////////////////////////////////////////////////////

enum class UNREG
{
	DO_NOTHING,
	DELETE_KEY,
	DELETE_VAL
};

struct REG_ITEM
{
	UNREG UnregType;
	HKEY RegKeyHandle;
	LPCWSTR SubKeyFormat;
	LPCWSTR SubKeyData1;
	LPCWSTR SubKeyData2;
	LPCWSTR ValueName;
	LPCWSTR ValueData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static LPCWSTR EMPTY{ L"" };
static LPCWSTR REG_FMT_CLSID{ L"CLSID\\%s" };
static LPCWSTR REG_FMT_INPROC32{ L"CLSID\\%s\\InprocServer32" };
static LPCWSTR REG_FMT_APPROVED{ L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved" };
static LPCWSTR REG_FMT_CLASSROOT{ L"%s\\ShellEx\\ContextMenuHandlers\\%s" };
static LPCWSTR REG_THREADINGMODEL{ L"ThreadingModel" };
static LPCWSTR REG_APARTMENT{ L"Apartment" };

///////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<REG_ITEM> GetRegList(const std::vector<LPCWSTR>& hkcr, LPCWSTR clsid, LPCWSTR name, LPCWSTR path)
{
	std::vector<REG_ITEM> list
	{
		{ UNREG::DELETE_KEY, HKEY_CLASSES_ROOT, REG_FMT_CLSID, clsid, EMPTY, EMPTY, name },
		{ UNREG::DO_NOTHING, HKEY_CLASSES_ROOT, REG_FMT_INPROC32, clsid, EMPTY, EMPTY, path },
		{ UNREG::DO_NOTHING, HKEY_CLASSES_ROOT, REG_FMT_INPROC32, clsid, EMPTY, REG_THREADINGMODEL, REG_APARTMENT },
		{ UNREG::DELETE_VAL, HKEY_LOCAL_MACHINE, REG_FMT_APPROVED, EMPTY, EMPTY, clsid, name }
	};

	for (const auto& cr : hkcr)
	{
		list.push_back({ UNREG::DELETE_KEY, HKEY_CLASSES_ROOT, REG_FMT_CLASSROOT, cr, name, EMPTY, clsid });
	}

	return list;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static size_t GetSubKeySize(const std::vector<REG_ITEM>& list)
{
	size_t size{ 0 };

	for (const auto& item : list)
	{
		auto len{ wcslen(item.SubKeyFormat) + 1 };
		len += wcslen(item.SubKeyData1);
		len += wcslen(item.SubKeyData2);
		if (len > size)
		{
			size = len;
		}
	}

	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(_In_ HMODULE hModule, _In_ DWORD dwReason, _In_ LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		return dll.Initialize(hModule);
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow()
{
	return dll.CanUnloadNow();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
	if (ppv == nullptr)
	{
		return E_POINTER;
	}
	else if (!IsEqualCLSID(rclsid, hlp::CreateGUID(PRODUCT_CLSID_W)))
	{
		*ppv = nullptr;
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	auto cf{ new (std::nothrow) ClassFactory(dll) };
	if (cf == nullptr)
	{
		*ppv = nullptr;
		return E_OUTOFMEMORY;
	}

	auto hr{ cf->QueryInterface(riid, ppv) };
	cf->Release();

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
	auto hr{ S_OK };
	auto regList{ GetRegList(std::vector<LPCWSTR>{ PRODUCT_HKCR_W }, PRODUCT_CLSID_W, PRODUCT_NAME_W, dll.Path()) };
	auto subKeySize{ GetSubKeySize(regList) };
	auto subKeyBuffer{ std::make_unique<WCHAR[]>(subKeySize) };

	for (const auto& regItem : regList)
	{
		if (FAILED(StringCchPrintfW(subKeyBuffer.get(), subKeySize, regItem.SubKeyFormat, regItem.SubKeyData1, regItem.SubKeyData2)) ||
			!hlp::SetRegValue(regItem.RegKeyHandle, subKeyBuffer.get(), regItem.ValueName, regItem.ValueData, REG_SZ))
		{
			DllUnregisterServer();
			hr = SELFREG_E_CLASS;
			break;
		}
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

STDAPI DllUnregisterServer()
{
	auto hr{ S_OK };
	auto regList{ GetRegList(std::vector<LPCWSTR>{ PRODUCT_HKCR_W }, PRODUCT_CLSID_W, PRODUCT_NAME_W, dll.Path()) };
	auto subKeySize{ GetSubKeySize(regList) };
	auto subKeyBuffer{ std::make_unique<WCHAR[]>(subKeySize) };

	for (const auto& regItem : regList)
	{
		if (regItem.UnregType != UNREG::DO_NOTHING)
		{
			if (FAILED(StringCchPrintfW(subKeyBuffer.get(), subKeySize, regItem.SubKeyFormat, regItem.SubKeyData1, regItem.SubKeyData2)))
			{
				hr = SELFREG_E_CLASS;
			}
			else if (regItem.UnregType == UNREG::DELETE_KEY)
			{
				RegDeleteTreeW(regItem.RegKeyHandle, subKeyBuffer.get());
				if (hlp::RegKeyExists(regItem.RegKeyHandle, subKeyBuffer.get()))
				{
					hr = SELFREG_E_CLASS;
				}
			}
			else if (regItem.UnregType == UNREG::DELETE_VAL)
			{
				RegDeleteKeyValueW(regItem.RegKeyHandle, subKeyBuffer.get(), regItem.ValueName);
				if (hlp::RegValueExists(regItem.RegKeyHandle, subKeyBuffer.get(), regItem.ValueName))
				{
					hr = SELFREG_E_CLASS;
				}
			}
		}
	}

	return hr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(suppress: 26495) // handle_ and path_ don't need to be initialized.

Dll::Dll() : refCount_{ 0 }
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL Dll::Initialize(HMODULE handle)
{
	auto size{ GetModuleFileNameW(handle, path_, _countof(path_)) };
	handle_ = handle;

	return size > 0 && size < _countof(path_);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT Dll::CanUnloadNow()
{
	return refCount_ == 0 ? S_OK : S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Dll::AddRef()
{
	++refCount_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void Dll::Release()
{
	--refCount_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

HMODULE Dll::Handle() const
{
	return handle_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

LPCWSTR Dll::Path() const
{
	return path_;
}
