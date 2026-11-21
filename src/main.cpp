#include "FreiaUI.h"
#include "ClientConnect.h"

int main()
{
    
    FreiaUI ui;
    while (ui.render()) {}
    return 0;
}