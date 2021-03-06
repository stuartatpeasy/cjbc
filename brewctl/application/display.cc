/*
    display.cc: manages the system display

    Stuart Wallace <stuartw@atom.net>, December 2017.

    Part of brewctl
*/

#include "include/application/display.h"
#include "include/application/sessionmanager.h"
#include "include/framework/log.h"
#include "include/util/validator.h"
#include <ctime>
#include <functional>
#include <map>

using std::invoke;
using std::map;
namespace Validator = Util::Validator;


// Front-panel button IDs
const ButtonId_t BUTTON_BOTTOM  = 2,
                 BUTTON_TOP     = 3,
                 ROT_CW         = 4,
                 ROT_CCW        = 5,
                 ROT_BUTTON     = 6;

// Time-interval constants
static const time_t 
    HOURS_PER_DAY                   = 24,
    MINS_PER_HOUR                   = 60,
    SECS_PER_MINUTE                 = 60,
    SECS_PER_HOUR                   = MINS_PER_HOUR * SECS_PER_MINUTE,
    SECS_PER_DAY                    = HOURS_PER_DAY * SECS_PER_HOUR,
    DEFAULT_DISPLAY_UPDATE_INTERVAL = 1,
    DEFAULT_SESSION_DWELL_TIME      = 5;

// Handlers for each display mode
Display::DispHandlerMap_t Display::handlers_ =
{
    {DM_DEFAULT, &Display::displayDefault},
    {DM_TOPMENU, &Display::displayTopMenu}
};


// ctor
//
Display::Display(SessionManager& sm) noexcept
    : sm_(sm),
      lcd_(Registry::instance().lcd()),
      lastDisplayUpdate_(0),
      displayUpdateInterval_(DEFAULT_DISPLAY_UPDATE_INTERVAL),
      currentMode_(DM_DEFAULT)
{
    sessionDwellTime_ = Registry::instance().config()
                            .get("display.session_dwell_time", DEFAULT_SESSION_DWELL_TIME, Validator::gt0);
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
        case SERVE:         return 'S';
        default:            return '?';
    }
}


// init() - initialise the display.  Note that this method doesn't initialise the underlying LCD; that has already been
// done elsewhere.
//
void Display::init() noexcept
{
    auto& bm = Registry::instance().buttonManager();

    // Register front-panel buttons with the ButtonManager.
    bm.registerButton(BUTTON_BOTTOM)
      .registerButton(BUTTON_TOP)
      .registerButton(ROT_CW)
      .registerButton(ROT_CCW)
      .registerButton(ROT_BUTTON);

    // Install button-press handlers
    bm.button(BUTTON_TOP).registerCallback(BUTTON_ANY_STATE, &Display::buttonCallback, this);
    bm.button(BUTTON_BOTTOM).registerCallback(BUTTON_ANY_STATE, &Display::buttonCallback, this);
    bm.button(ROT_CW).registerCallback(BUTTON_PRESSED, &Display::buttonCallback, this);
    bm.button(ROT_BUTTON).registerCallback(BUTTON_ANY_STATE, &Display::buttonCallback, this);

    lcd_.backlight(true);
}


void Display::update() noexcept
{
    auto h = handlers_.find(currentMode_);

    if(h != handlers_.end())
        invoke(h->second, *this);
    else
        displayDefault();
}


// displayDefault() - display the default screen, which comprises a fermentation/conditioning progress indicator and
// some other information.
//
void Display::displayDefault() noexcept
{
    const time_t now = ::time(NULL);
    static size_t currentIdx = -1;

    if((now - lastDisplayUpdate_) >= displayUpdateInterval_)
    {
        char buffer[16];
        time_t tm;

        lastDisplayUpdate_ = now;

        ::time(&tm);
        ::strftime(buffer, sizeof(buffer), "%H:%M", ::localtime(&tm));
        lcd_.printAt(0, 0, buffer);

        const Temperature ambientTemp = sm_.ambientTemp();
        if(ambientTemp)
            lcd_.printAt(15, 0, "% 3d\xdf""C", (int) (ambientTemp.C() + 0.5));
        else
            lcd_.printAt(16, 0, "--\xdf""C");

        const size_t nsessions = sm_.sessions().size();
        if(nsessions)
        {
            const size_t sessionIdx = (now / sessionDwellTime_) % nsessions;
            auto itSession = sm_.sessions().begin();
            for(size_t i = 0; (i < sessionIdx) && (itSession != sm_.sessions().end()); ++i)
                ++itSession;

            const auto& session = itSession->second;

            // If we're about to render a new session, clear the bottom half of the LCD in preparation for writing
            // session data to it.
            if(sessionIdx != currentIdx)
            {
                currentIdx = sessionIdx;
                lcd_.clearLine(2);
                lcd_.clearLine(3);
            }

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

                const char *fmt = nullptr;
                time_t field1 = 0, field2 = 0;

                if(session->type() != SERVE)
                {
                    const time_t secsRemaining  = session->remainingTime(),
                                 days           = secsRemaining / SECS_PER_DAY,
                                 hours          = secsRemaining / SECS_PER_HOUR,
                                 minutes        = secsRemaining / SECS_PER_MINUTE;

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
        else
            lcd_.printAt(5, 2, "No sessions");
    }
}


void Display::displayTopMenu() noexcept
{
}


// buttonCallback() - reflects button-press/-release events into the active display handler object.  Note that this
// method is called within the context of the button manager thread.
//
void Display::buttonCallback(const ButtonId_t buttonId, const ButtonState_t state, void *arg) noexcept
{
    Display * const disp = reinterpret_cast<Display *> (arg);

    disp->buttonEvent(buttonId, state);
}


// buttonEvent() - handle button-press/-release events.  Note that this method is called within the context of the
// button manager thread.
//
void Display::buttonEvent(const ButtonId_t buttonId, const ButtonState_t state) noexcept
{
    if(buttonId == ROT_CW)
    {
        if(Registry::instance().buttonManager().button(ROT_CCW).state() == BUTTON_PRESSED)
            logDebug("--- anticlockwise");
        else
            logDebug("--- clockwise");
    }
    else
        logDebug("Display::buttonEvent() - button %d %s", buttonId, (state == BUTTON_PRESSED) ? "pressed" : "released");
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

