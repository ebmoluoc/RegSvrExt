#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////

class ClassFactory : public IClassFactory
{
public:
	ClassFactory(Dll& dll);
	~ClassFactory();

	IFACEMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	IFACEMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObject);
	IFACEMETHODIMP LockServer(BOOL fLock);

private:
	Dll& dll_;
	ULONG refCount_;
};
