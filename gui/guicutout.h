#ifndef GUI_CUTOUT_H_
#define GUI_CUTOUT_H_

int EnableCutout(LPCALC lpCalc);
int DisableCutout(LPCALC);
LRESULT CALLBACK SmallButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PositionLittleButtons(HWND);

#define g_szSmallClose _T("wabbitclose")
#define g_szSmallMinimize _T("wabbitminimize")

#endif /*GUI_CUTOUT_H_*/
