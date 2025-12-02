#include "FreiaUI.h"
#include "ClientConnect.h"

int main()
{
    
    FreiaUI ui;
    ClientConnect client;

    ui.setClient(&client);

    while (ui.render()) {}
    return 0;
}
