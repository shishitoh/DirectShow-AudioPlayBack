#pragma once

template<typename T>
void SafeRelease(T **ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

/* Trackbarの位置から音量を返す関数、要調整。 */
inline int postodB(const int pos) {

    if (pos < 1000) {
        return pos/100*475 - 10000;
    }else if (1000 <= pos && pos < 5000) {
        return pos/5*4 - 6000;
    } else {
        return pos/5*2 - 4000;
    }
}