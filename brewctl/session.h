#ifndef SESSION_H_INC
#define SESSION_H_INC
/*
    session.h: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "effector.h"
#include "error.h"
#include "temperature.h"
#include "tempsensor.h"
#include <ctime>        // ::time()
#include <map>
#include <string>
#include <vector>


typedef std::pair<time_t, double> SessionStage_t;
typedef std::vector<SessionStage_t> SessionStages_t;

class Session
{
public:
                        Session(const int id, Error * const err = nullptr) noexcept;
                        ~Session() noexcept;

    Temperature         targetTemp() noexcept;
    Temperature         currentTemp() noexcept;
    bool                isActive() const noexcept;
    void                main() noexcept;

private:
    bool                updateEffectors(Error * const err = nullptr) noexcept;

    const int           id_;
    std::string         gyle_;
    int                 profile_;
    time_t              start_ts_;
    time_t              end_ts_;
    double              deadZone_;
    SessionStages_t     stages_;
    TempSensor *        tempSensorVessel_;
    Effector *          effectorHeater_;
    Effector *          effectorCooler_;

    time_t              effectorUpdateInterval_;
    time_t              lastEffectorUpdate_;
};

#endif // SESSION_H_INC

