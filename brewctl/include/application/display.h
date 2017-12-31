#ifndef APPLICATION_DISPLAY_H_INC
#define APPLICATION_DISPLAY_H_INC
/*
    display.h: manages the system display

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/application/session.h"
#include "include/framework/registry.h"
#include "include/peripherals/button.h"
#include "include/peripherals/lcd.h"
#include <cstddef>
#include <map>


class SessionManager;       // Forward decl to enable composition

// Forward decl and ptr-to-member typedef to enable display-mode handlers to be called easily

// Display modes
typedef enum DisplayMode
{
    DM_DEFAULT,
    DM_TOPMENU
} DisplayMode_t;


class Display
{
typedef std::map<DisplayMode_t, void (Display::*)(void)> DispHandlerMap_t;

public:
                            Display(SessionManager& sm) noexcept;

    void                    init() noexcept;
    void                    update() noexcept;
    void                    notifyShutdown() noexcept;
    void                    stop() noexcept;

private:
    void                    displayDefault() noexcept;
    void                    displayTopMenu() noexcept;

    static void             buttonCallback(const ButtonId_t buttonId, const ButtonState_t state, void *arg) noexcept;
    void                    buttonEvent(const ButtonId_t buttonId, const ButtonState_t state) noexcept;

    char                    getTempControlIndicator(const SessionTempControlState_t state) const noexcept;
    char                    getSessionTypeIndicator(const SessionType_t type) const noexcept;

    SessionManager&         sm_;
    LCD&                    lcd_;
    time_t                  lastDisplayUpdate_;
    time_t                  displayUpdateInterval_;
    time_t                  sessionDwellTime_;
    DisplayMode_t           currentMode_;
    static DispHandlerMap_t handlers_;
};

#endif // APPLICATION_DISPLAY_H_INC

