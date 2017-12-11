#ifndef APPLICATION_DISPLAY_H_INC
#define APPLICATION_DISPLAY_H_INC
/*
    display.h: manages the system display

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/application/session.h"
#include "include/framework/registry.h"
#include "include/peripherals/lcd.h"
#include <cstddef>


class SessionManager;       // Forward decl to enable composition

class Display
{
public:
                        Display(SessionManager& sm) noexcept;

    void                init() noexcept;
    void                update() noexcept;
    void                notifyShutdown() noexcept;
    void                stop() noexcept;

private:
    char                getTempControlIndicator(const SessionTempControlState_t state) const noexcept;
    char                getSessionTypeIndicator(const SessionType_t type) const noexcept;

    SessionManager&     sm_;
    LCD&                lcd_;
    time_t              lastDisplayUpdate_;
    time_t              displayUpdateInterval_;
};

#endif // APPLICATION_DISPLAY_H_INC

