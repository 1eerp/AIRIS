// Sakura.cpp : Defines the entry point for the application.
//


#include <Windows.h>
#include <Sakura.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

#if _DEBUG
    if(AllocConsole())
        Log::Init();
#endif

    try
    {
        // Intiate window properties and create main window
        WindowProps properties;
        properties.StartProps.ShowCommand = nCmdShow;
        SWindow mainWindow(hInstance, properties);

        // Create and run application

        SApp app(&mainWindow);
        app.Run();
    }
    catch (std::exception e)
    {
        CORE_ERROR(e.what());
    }
    return 0;
}
