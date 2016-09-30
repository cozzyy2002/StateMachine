// Test.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define INITGUID
#include <guiddef.h>
#include <mfapi.h>

static HRESULT enumMFTransforms();

int main()
{
	enumMFTransforms();

    return 0;
}

#define GUID_ENTRY(x) { x, #x }
struct GuidEntry {
	REFGUID guid;
	LPTSTR name;
};

static const GuidEntry g_mftCategories[] = {
	GUID_ENTRY(MFT_CATEGORY_AUDIO_DECODER),
	GUID_ENTRY(MFT_CATEGORY_AUDIO_EFFECT),
	GUID_ENTRY(MFT_CATEGORY_AUDIO_ENCODER),
	GUID_ENTRY(MFT_CATEGORY_DEMULTIPLEXER),
	GUID_ENTRY(MFT_CATEGORY_MULTIPLEXER),
	GUID_ENTRY(MFT_CATEGORY_OTHER),
	GUID_ENTRY(MFT_CATEGORY_VIDEO_DECODER),
	GUID_ENTRY(MFT_CATEGORY_VIDEO_EFFECT),
	GUID_ENTRY(MFT_CATEGORY_VIDEO_ENCODER),
	GUID_ENTRY(MFT_CATEGORY_VIDEO_PROCESSOR),
};

HRESULT enumMFTransforms()
{
	for (int i = 0; i < ARRAYSIZE(g_mftCategories); i++) {
		const GuidEntry& entry = g_mftCategories[i];
		CLSID* pClsId;
		UINT32 count;
		HR_ASSERT_OK(MFTEnum(entry.guid, 0, NULL, NULL, NULL, &pClsId, &count));

		std::cout << entry.name << ": Count=" << count << std::endl;

		for (UINT32 n = 0; n < count; n++) {
			LPWSTR name;
			if (SUCCEEDED(MFTGetInfo(pClsId[n], &name, NULL, NULL, NULL, NULL, NULL))) {
				std::wcout << L"  " << n << L": " << name << std::endl;
				CoTaskMemFree(name);
			}
		}

		CoTaskMemFree(pClsId);
	}

	return S_OK;
}
