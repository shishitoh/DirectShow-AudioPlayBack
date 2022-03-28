#include <iostream>
#include <locale>

#include <windows.h>

#pragma once

constexpr char LOCALE[] = "ja-JP.UTF-8";

class DebugConsole {

    FILE *fp;

    void SetConsole() {
        if (AttachConsole(ATTACH_PARENT_PROCESS) == FALSE) {
            AllocConsole();
        }
    }

public:

    DebugConsole() {
        SetConsole();
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        fprintf(stdout, "\n");
        std::setlocale(LC_ALL, LOCALE);
    }

    ~DebugConsole() {
        fclose(fp);
        FreeConsole();
    }
};