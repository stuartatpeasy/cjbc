/*
    display.cc: manages the system display

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/application/display.h"
#include "include/application/sessionmanager.h"
#include <ctime>


// Time-interval constants
static const time_t 
    HOURS_PER_DAY                   = 24,
    MINS_PER_HOUR                   = 60,
    SECS_PER_MINUTE                 = 60,
    SECS_PER_HOUR                   = MINS_PER_HOUR * SECS_PER_MINUTE,
    SECS_PER_DAY                    = HOURS_PER_DAY * SECS_PER_HOUR,
    DEFAULT_DISPLAY_UPDATE_INTERVAL = 1;


// ctor
//
Display::Display(SessionManager& sm) noexcept
    : sm_(sm),
      lcd_(Registry::instance().lcd()),
      lastDisplayUpdate_(0),
      displayUpdateInterval_(DEFAULT_DISPLAY_UPDATE_INTERVAL)
{
}


// getTempControlIndicator() - return a character representing the temperature-control activity specified by <state>,
// e.g. an up-arrow character to indicate that the session is currently heating its vessel.
//
char Display::getTempControlIndicator(const SessionTempControlState_t state) const noexcept
{
    switch(state)
    {
        case FAST_COOL:     return LCD_CH_ARROW_2DOWN;
        case COOL:          return LCD_CH_ARROW_DOWN;
        case HOLD:          return LCD_CH_ARROW_RIGHT;
        case HEAT:          return LCD_CH_ARROW_UP;
        case FAST_HEAT:     return LCD_CH_ARROW_2UP;
        default:            return '?';
    }
}


// getSessionTypeIndicator() - return a character representing the type (fermentation or conditioning) specified by
// <type>.
//
char Display::getSessionTypeIndicator(const SessionType_t type) const noexcept
{
    switch(type)
    {
        case FERMENT:       return 'F';
        case CONDITION:     return 'C';
        default:            return '?';
    }
}


// init() - initialise the display.  Note that this method doesn't initialise the underlying LCD; that has already been
// done elsewhere.
//
void Display::init() noexcept
{
    lcd_.backlight(true);
}


void Display::update() noexcept
{
    lcd_.printAt(18, 0, "\xdf""C");

    const time_t now = ::time(NULL);

    if((now - lastDisplayUpdate_) > displayUpdateInterval_)
    {
        char buffer[16];
        time_t tm;

        lastDisplayUpdate_ = now;

        ::time(&tm);
        ::strftime(buffer, sizeof(buffer), "%H:%M", ::localtime(&tm));
        lcd_.printAt(0, 0, buffer);

        const Temperature ambientTemp = sm_.ambientTemp();
        if(ambientTemp)
            lcd_.printAt(16, 0, "%2d", (int) (ambientTemp.C() + 0.5));
        else
            lcd_.printAt(16, 0, "--");

        for(auto& session : sm_.sessions())
        {
            lcd_.printAt(0, 2, "G%-3d", session->gyleId());
            lcd_.printAt(0, 3, "%.20s", session->gyleName().c_str());
            
            if(session->isActive())
            {
                lcd_.putAt(5, 2, getSessionTypeIndicator(session->type()));

                lcd_.putAt(7, 2, getTempControlIndicator(session->tempControlState()));
                if(session->vesselTempSensorInRange())
                    lcd_.printAt(8, 2, "%4.1lf\xdf", session->currentTemp().C());
                else
                    lcd_.printAt(8, 2, "--.-\xdf");

                const time_t secsRemaining  = session->remainingTime(),
                             days           = secsRemaining / SECS_PER_DAY,
                             hours          = secsRemaining / SECS_PER_HOUR,
                             minutes        = secsRemaining / SECS_PER_MINUTE;

                time_t field1, field2;
                const char *fmt = nullptr;

                if(days)
                {
                    fmt = "%2dd%02dh";
                    field1 = days;
                    field2 = hours % HOURS_PER_DAY;
                }
                else if(hours)
                {
                    fmt = "%2dh%02dm";
                    field1 = hours;
                    field2 = minutes % MINS_PER_HOUR;
                }
                else if(secsRemaining)
                {
                    fmt = "%2dm%02ds";
                    field1 = minutes;
                    field2 = secsRemaining % SECS_PER_MINUTE;
                }

                if(fmt != nullptr)
                    lcd_.printAt(14, 2, fmt, field1, field2);
                else
                    lcd_.printAt(14, 2, "      ");
            }
            else
            {
                lcd_.printAt(5, 2, session->isComplete() ? "Complete" : "Starts in");
            }
        }
    }
}


// notifyShutdown() - clear the LCD and display a "shutting down" message
//
void Display::notifyShutdown() noexcept
{
    lcd_.clear();
    lcd_.printAt(3, 1, "Shutting down.");
}


// stop() - clear the LCD and switch off the backlight, in preparation for system shutdown.
//
void Display::stop() noexcept
{
    lcd_.clear();
    lcd_.backlight(false);
}

