#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <memory>

#include <windows.h>
#include <Dshow.h>
#include <CommCtrl.h>

#include "basewin.h"
#include "DirectShow.h"
#include "utlty.h"
#include "console.h"

constexpr int DEFAULT_POS = 10000;

class MainWindow : public BaseWindow<MainWindow> {

    HWND hwndButton;
    HWND hwndTrackBar;
    HWND hwndComboBox;
    IGraphBuilder *pGraph;
    IMediaControl *pControl;
    IBasicAudio *pBasic;
    IBaseFilter *pCap;

    HRESULT SetAudioResources();
    HRESULT SetUI();
    HRESULT SetCap(LPCTSTR);
    HRESULT SetRender() const;
    HRESULT RemoveCap();
    HRESULT SetStringToComboBox() const;
    HRESULT ChangeInput(LPCTSTR);
    void DiscardResouces();
    HRESULT put_Volume(int pos) const {
        if (!pBasic) return E_POINTER;
        return pBasic->put_Volume(pos);
    }
    HRESULT Run() const {
        if (!pControl) return E_POINTER;
        return pControl->Run();
    }

public:

    MainWindow()
        :pGraph(nullptr), pControl(nullptr), pBasic(nullptr), pCap(nullptr) {}

    PCWSTR ClassName() const {
        return L"AudioPlayBack";
    }

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

HRESULT MainWindow::SetAudioResources() {

    HRESULT hr = CoCreateInstance(
        CLSID_FilterGraph,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**)&pGraph
    );
    if (FAILED(hr)) return hr;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    if (FAILED(hr)) return hr;
    hr = pGraph->QueryInterface(IID_IBasicAudio, (void**)&pBasic);
    if (FAILED(hr)) return hr;

    return hr;
}

HRESULT MainWindow::SetUI() {

    HRESULT hr;

    /* TrackBar */
    hwndTrackBar = CreateWindow(
        TRACKBAR_CLASS,
        L"TrackBar",
        TBS_AUTOTICKS | TBS_ENABLESELRANGE | TBS_TOP | WS_CHILD | WS_VISIBLE,
        0, 0,
        676, 48,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL
    );
    if (!hwndTrackBar) return E_FAIL;
    SendMessage(hwndTrackBar, (UINT)TBM_SETRANGE, (WPARAM)TRUE, MAKELPARAM(0, 10000));
    SendMessage(hwndTrackBar, (UINT)TBM_SETTICFREQ, (WPARAM)1000, (LPARAM)0); // 10分割
    SendMessage(hwndTrackBar, (UINT)TBM_SETPOS, (WPARAM)TRUE, (LPARAM)DEFAULT_POS);
    SendMessage(hwndTrackBar, (UINT)TBM_SETSEL, (WPARAM)TRUE, MAKELPARAM(0, DEFAULT_POS));

    /* ComboBox */
    hwndComboBox = CreateWindow(
        WC_COMBOBOX,
        L"ComboBox",
        CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        0, 48,
        676, 192,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL
    );
    if (!hwndComboBox) return E_FAIL;
    hr = SetStringToComboBox();
    if (FAILED(hr)) return hr;
    ComboBox_SetCurSel(hwndComboBox, 0);

    return S_OK;
}

HRESULT MainWindow::SetCap(LPCTSTR Text) {

    if (!pGraph) return E_POINTER;

    HRESULT hr;
    IEnumMoniker *pEnum;

    hr = GetEnumMoniker(CLSID_AudioInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr) && hr != S_FALSE) {

        IMoniker *pMoniker;

        while(pEnum->Next(1, &pMoniker, nullptr) == S_OK) {

            TCHAR FriendlyName[128];
            hr = GetFriendlyName(pMoniker, FriendlyName);
            if (SUCCEEDED(hr) && wcscmp(Text, FriendlyName) == 0) {
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);
                if (SUCCEEDED(hr)) {
                    hr = pGraph->AddFilter(pCap, FriendlyName);
                    if (SUCCEEDED(hr)) {
                        SafeRelease(&pMoniker);
                        break;
                    }
                }
            }
            hr = E_NOINTERFACE;
            SafeRelease(&pMoniker);
        }
    }
    SafeRelease(&pEnum);

    return hr;
}

HRESULT MainWindow::SetRender() const {

    if (!pCap || !pGraph) return E_POINTER;

    HRESULT hr;

    IPin *pPin;
    hr = FindUnconnectedPin(pCap, PINDIR_OUTPUT, &pPin);
    if (SUCCEEDED(hr)) {
        hr = pGraph->Render(pPin);
    }
    SafeRelease(&pPin);

    return hr;
}

HRESULT MainWindow::RemoveCap() {

    if (!pControl || !pGraph) return E_POINTER;

    HRESULT hr;

    /* グラフが動いていたら止める */
    hr = pControl->Stop();

    if (SUCCEEDED(hr) && pCap) {
        hr = pGraph->RemoveFilter(pCap);
        if (SUCCEEDED(hr)) {
            SafeRelease(&pCap);
        }
    }
    return hr;
}

void MainWindow::DiscardResouces() {
    pControl->Stop();
    SafeRelease(&pBasic);
    SafeRelease(&pControl);
    SafeRelease(&pGraph);

}

HRESULT MainWindow::SetStringToComboBox() const {

    ComboBox_ResetContent(hwndComboBox);

    IEnumMoniker *pEnum;
    HRESULT hr = GetEnumMoniker(CLSID_AudioInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr) && hr != S_FALSE) {

        IMoniker *pMoniker;
        bool IsEmpty = true;

        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
            TCHAR FriendlyName[128];
            hr = GetFriendlyName(pMoniker, FriendlyName);
            if (SUCCEEDED(hr)) {
                ComboBox_AddString(hwndComboBox, FriendlyName);
                IsEmpty = false;
            }
            SafeRelease(&pMoniker);
        }
        if (IsEmpty) {
            hr = E_NOINTERFACE;
        }
    }
    SafeRelease(&pEnum);

    return hr;
}

HRESULT MainWindow::ChangeInput(LPCTSTR Text) {

    HRESULT hr;
    hr = RemoveCap();
    if (SUCCEEDED(hr)) {
        hr = SetCap(Text);
        if (SUCCEEDED(hr)) {
            hr = SetRender();
        }
    }

    return hr;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {

    DebugConsole DC;

    MainWindow win;

    if (
        !win.Create(
            L"AudioPlayBack",
            WS_OVERLAPPEDWINDOW,
            0,
            CW_USEDEFAULT, CW_USEDEFAULT,
            692, 144
        )
    ) {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

        case WM_CREATE:
            CoInitialize(NULL);
            if (FAILED(SetAudioResources()) || FAILED(SetUI())) {
                SendMessage(m_hwnd, (UINT)WM_DESTROY, 0, 0);
            }
            PostMessage(m_hwnd, (UINT)WM_COMMAND, MAKEWPARAM(0, CBN_SELENDOK), 0);
            return 0;

        case WM_DESTROY:
            DiscardResouces();
            CoUninitialize();
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW));
            EndPaint(m_hwnd, &ps);
            return 0;
        }

        case WM_HSCROLL: {
            int pos = SendMessage(hwndTrackBar, (UINT)TBM_GETPOS, (WPARAM)0, (LPARAM)0);
            put_Volume(postodB(pos));
            SendMessage(hwndTrackBar, (UINT)TBM_SETSELEND, (WPARAM)TRUE, (LPARAM)pos);
        }
        return 0;

        case WM_COMMAND: {

            switch (HIWORD(wParam)) {
                case CBN_SELENDOK: {
                    TCHAR Text[128];
                    ComboBox_GetText(hwndComboBox, Text, 128);
                    HRESULT hr = ChangeInput(Text);
                    if (SUCCEEDED(hr)) {
                        Run();
                    }
                    return 0;
                }
                default:
                    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);

            }
        }
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}