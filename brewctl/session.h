#ifndef SESSION_H_INC
#define SESSION_H_INC
/*
    session.h: manages a single fermentation/conditioning session

    Stuart Wallace <stuartw@atom.net>, October 2017.

    Part of brewctl
*/

#include "error.h"
#include "effector.h"
#include "temperature.h"
#include "temperaturesensor.h"
#include <ctime>        // ::time()
#include <map>
#include <string>
#include <vector>


typedef std::pair<time_t, double> SessionStage_t;
typedef std::vector<SessionStage_t> SessionStages_t;

class Session
{
public:
                                    Session(const int id, Error * const err);
    Temperature                     targetTemp();
    bool                            isActive() const;

private:
    bool                            updateEffectors(Error * const err);

    const int                       id_;
    std::string                     gyle_;
    int                             profile_;
    time_t                          start_ts_;
    time_t                          end_ts_;
    double                          deadZone_;
    SessionStages_t                 stages_;
    TemperatureSensor *             tempSensorVessel_;
    Effector *                      effectorHeater_;
    Effector *                      effectorCooler_;
};

#endif // SESSION_H_INC

