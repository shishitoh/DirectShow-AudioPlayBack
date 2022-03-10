#include <Dshow.h>

#include "utlty.h"

#pragma once

/* not used now */
HRESULT GetFilterName(IBaseFilter *pFilter, TCHAR *Text) {

    HRESULT hr = E_POINTER;
    if (!pFilter) {
        return hr;
    }
    FILTER_INFO FInfo;
    hr = pFilter->QueryFilterInfo(&FInfo);
    if (SUCCEEDED(hr)) {
        wcscpy_s(Text, 128, FInfo.achName);
    }
    return hr;
}

HRESULT GetFriendlyName(IMoniker *pMoniker, TCHAR *Text) {

    HRESULT hr = E_POINTER;
    if (!pMoniker) {
        return hr;
    }
    IPropertyBag *pPropBag;
    hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
    if (SUCCEEDED(hr)) {
        VARIANT var;
        VariantInit(&var);
        hr = pPropBag->Read(L"FriendlyName", &var, nullptr);
        if (SUCCEEDED(hr)) {
            wcscpy_s(Text, 128, var.bstrVal);
            VariantClear(&var);
        }
    }
    SafeRelease(&pPropBag);

    return hr;
}

HRESULT GetEnumMoniker(GUID guid, IEnumMoniker **ppEnum) {

    HRESULT hr = S_OK;
    ICreateDevEnum *pDevEnum = nullptr;
    hr = CoCreateInstance(
        CLSID_SystemDeviceEnum,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDevEnum)
    );
    if (SUCCEEDED(hr)) {
        hr = pDevEnum->CreateClassEnumerator(guid, ppEnum, 0);
    }
    SafeRelease(&pDevEnum);

    return hr;
}

HRESULT FindUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin) {

    HRESULT hr = E_POINTER;
    if (!pFilter) return hr;

    IEnumPins *pEnum = nullptr;
    bool Found = false;

    hr = pFilter->EnumPins(&pEnum);
    if (SUCCEEDED(hr)) {
        IPin *pPin = nullptr;
        while(pEnum->Next(1, &pPin, nullptr) == S_OK) {
            IPin *pTmp = nullptr;
            hr = pPin->ConnectedTo(&pTmp);
            if (hr == VFW_E_NOT_CONNECTED) {
                PIN_DIRECTION ppPinPinDir;
                hr = pPin->QueryDirection(&ppPinPinDir);
                if (SUCCEEDED(hr) && PinDir == ppPinPinDir) {
                    Found = true;
                    *ppPin = pPin;
                    (*ppPin)->AddRef();
                }
            }
            SafeRelease(&pPin);
            SafeRelease(&pTmp);
            if (Found) break;
        }
    }
    if (!Found) {
        hr = VFW_E_NOT_FOUND;
    }
    SafeRelease(&pEnum);

    return hr;
}