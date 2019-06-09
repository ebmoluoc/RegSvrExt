#pragma once

//////////////////////  Windows 7 and above  //////////////////////////////////////////////////////

#define NTDDI_VERSION	0x06010000
#define WINVER			0x0601
#define _WIN32_WINDOWS	0x0601
#define _WIN32_WINNT	0x0601
#define _WIN32_IE		0x0A00

//////////////////////  Precompiled Headers  //////////////////////////////////////////////////////

#include <memory>
#include <strsafe.h>
#include <olectl.h>
#include <CppHelpers.h>

#include "Dll.h"
#include "ClassFactory.h"
#include "resource.h"
