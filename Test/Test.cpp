// Test.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define INITGUID
#include <guiddef.h>
#include <mfapi.h>
#include <mftransform.h>
#include <dmoreg.h>
#include <medparam.h>

#include <log4cplus/configurator.h>

static HRESULT enumMFTransforms();
static HRESULT enumDMOs();

int main()
{
	log4cplus::BasicConfigurator::doConfigure();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	MFStartup(MF_VERSION);

	enumMFTransforms();
	enumDMOs();

	MFShutdown();
	CoUninitialize();

    return 0;
}

#define GUID_ENTRY(x) { x, OLESTR(#x) }
struct GuidEntry {
	REFGUID guid;
	LPCWSTR name;
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
		CComBSTR strGuid(entry.guid);
		CComHeapPtr<CLSID> pClsId;
		UINT32 count;
		HR_ASSERT_OK(MFTEnum(entry.guid, 0, NULL, NULL, NULL, &pClsId, &count));

		std::wcout << L"\n" << (LPCWSTR)strGuid << L":" << entry.name << L": Count=" << count << std::endl;

		for (UINT32 n = 0; n < count; n++) {
			CComHeapPtr<WCHAR> name;
			if (SUCCEEDED(MFTGetInfo(pClsId[n], &name, NULL, NULL, NULL, NULL, NULL))) {
				CComBSTR strGuid(pClsId[n]);
				std::wcout << L"  " << n << L": " << (LPCWSTR)strGuid << L":" << (LPCWSTR)name << std::endl;
			}
		}
	}

	return S_OK;
}

static const GuidEntry g_dmoCategories[] = {
	GUID_ENTRY(DMOCATEGORY_AUDIO_DECODER),			// Audio decoder
	GUID_ENTRY(DMOCATEGORY_AUDIO_EFFECT),			// Audio effect
	GUID_ENTRY(DMOCATEGORY_AUDIO_ENCODER),			// Audio encoder
	GUID_ENTRY(DMOCATEGORY_VIDEO_DECODER),			// Video decoder
	GUID_ENTRY(DMOCATEGORY_VIDEO_EFFECT),			// Video effect
	GUID_ENTRY(DMOCATEGORY_VIDEO_ENCODER),			// Video encoder
	GUID_ENTRY(DMOCATEGORY_AUDIO_CAPTURE_EFFECT),	// Audio capture effect
};

HRESULT enumDMOs()
{
	for (int i = 0; i < ARRAYSIZE(g_dmoCategories); i++) {
		const GuidEntry& entry = g_dmoCategories[i];

		CComBSTR strGuid(entry.guid);
		std::wcout << L"\n" << (LPCWSTR)strGuid << L":" << entry.name << std::endl;

		CComPtr<IEnumDMO> enumDmo;
		HR_ASSERT_OK(DMOEnum(entry.guid, DMO_ENUMF_INCLUDE_KEYED,
						0, NULL,
						0, NULL,
						&enumDmo));

		while (true) {
			GUID guid;
			CComHeapPtr<WCHAR> pStr;
			HRESULT hr = HR_EXPECT_OK(enumDmo->Next(1, &guid, &pStr, NULL));
			if (hr != S_OK) break;
			CComPtr<IMediaObject> dmo;
			HR_ASSERT_OK(dmo.CoCreateInstance(guid));

			// Check if the DMO exposes IMFTransform.
			CComPtr<IMFTransform> mft;
			hr = dmo.QueryInterface(&mft);
			CComBSTR strGuid(guid);
			std::wcout << L"  " << (LPCWSTR)strGuid << L":" << (SUCCEEDED(hr) ? L"MFT " : L"    ") << (LPCWSTR)pStr << std::endl;

			// Enumerate parameters.
			CComPtr<IMediaParamInfo> pi;
			if (SUCCEEDED(dmo.QueryInterface(&pi))) {
				DWORD paramCount;
				pi->GetParamCount(&paramCount);
				for (DWORD p = 0; p < paramCount; p++) {
					MP_PARAMINFO info;
					pi->GetParamInfo(p, &info);
					LPCWSTR type = L"?";
					switch (info.mpType) {
					case MPT_INT:	type = L"MPT_INT  "; break;
					case MPT_FLOAT:	type = L"MPT_FLOAT"; break;
					case MPT_BOOL:	type = L"MPT_BOOL "; break;
					case MPT_ENUM:	type = L"MPT_ENUM "; break;
					}
					std::wcout << L"    " << type << L" " << info.szLabel << L"(" << info.szUnitText << L")="
								<< info.mpdMinValue << L"~" << info.mpdMaxValue << L":" << info.mpdNeutralValue;

					CComPtr<IMediaParams> params;
					HR_ASSERT_OK(dmo.QueryInterface(&params));
					MP_DATA value;
					HR_ASSERT_OK(params->GetParam(p, &value));
					std::wcout << L", default=" << value << std::endl;
				}
			}
		}
	}

	return S_OK;
}
