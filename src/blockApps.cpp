/*
*      Copyright 2016 Riccardo Melioli.
*/

#include "blockApps.h"


HANDLE BlockApps::start() {
    hBlockAppsTh = CreateThread(0, 0, &BlockAppsThread, &dangerousApps, 0, 0);

    //WaitForSingleObject(hBlockAppsTh,INFINITE);

    //CloseHandle(hBlockAppsTh);
    return hBlockAppsTh;
}

void BlockApps::stop() {
    SuspendThread(hBlockAppsTh);
}

HANDLE BlockApps::getHandleThread() {
    return hBlockAppsTh;
}


DWORD WINAPI BlockAppsThread(void *arg) {
    //std::list<const char*>* dangApps = (std::list<const char*> *) arg;
    SharedList<const char *> Sl = *((SharedList<const char *> *) arg);
    std::list<const char *> dangApps = Sl.getCopy();
    HWND app_heandler;

    while (true) {
        for (std::list<const char *>::iterator it = (dangApps).begin();
             it != (dangApps).end(); ++it) {

            app_heandler = FindWindow(NULL, *it);
            if (app_heandler != NULL)
                PostMessage(app_heandler, WM_CLOSE, (LPARAM) 0, (WPARAM) 0);
        }

        Sleep(100);
        dangApps = Sl.getCopy();
    }

}