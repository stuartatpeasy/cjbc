--
-- effectorlog
--
DROP TABLE IF EXISTS "effectorlog";
CREATE TABLE "effectorlog"(
    date_create         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    effector_id         INT UNSIGNED NOT NULL,
    newstate            TINYINT NOT NULL);

DROP INDEX IF EXISTS effectorlog_date_create_effector_id;
CREATE INDEX effectorlog_date_create_effector_id
    ON "effectorlog"(date_create, effector_id);

--
-- effectortype
--
DROP TABLE IF EXISTS "effectortype";
CREATE TABLE "effectortype"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(16) NOT NULL COLLATE NOCASE,
    powerconsumption    DECIMAL(4, 4) NOT NULL);

--
-- gyle
--
DROP TABLE IF EXISTS "gyle";
CREATE TABLE "gyle"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    date_create         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);

--
-- option
--
DROP TABLE IF EXISTS "option";
CREATE TABLE "option"(
    name                VARCHAR(255) PRIMARY KEY NOT NULL,
    value               TEXT DEFAULT NULL);

--
-- profile
--
DROP TABLE IF EXISTS "profile";
CREATE TABLE "profile"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(16) NOT NULL COLLATE NOCASE);

--
-- profilestage
--
DROP TABLE IF EXISTS "profilestage";
CREATE TABLE "profilestage"(
    profile_id          INT UNSIGNED NOT NULL,
    stage               INT UNSIGNED NOT NULL,
    duration_hours      INT UNSIGNED,
    temperature         DECIMAL(4, 4));

DROP INDEX IF EXISTS profilestage_profile_id_stage;
CREATE UNIQUE INDEX profilestage_profile_id_stage
    ON "profilestage"(profile_id, stage);

--
-- sensortype
--
DROP TABLE IF EXISTS "sensortype";
CREATE TABLE "sensortype"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(16) NOT NULL COLLATE NOCASE,
    type_id             INT UNSIGNED NOT NULL);

--
-- session
--
DROP TABLE IF EXISTS "session";
CREATE TABLE "session"(
    id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    gyle_id             INT UNSIGNED NOT NULL,
    profile_id          INT UNSIGNED NOT NULL,
    date_create         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    date_start          DATETIME DEFAULT NULL,
    date_finish         DATETIME DEFAULT NULL);

DROP INDEX IF EXISTS session_id_date_finish;
CREATE UNIQUE INDEX session_id_date_finish
    ON "session"(id, date_finish);

--
-- sessioneffector
--
DROP TABLE IF EXISTS "sessioneffector";
CREATE TABLE "sessioneffector"(
    session_id          INT UNSIGNED NOT NULL,
    effectortype_id     INT UNSIGNED NOT NULL,
    channel             INT UNSIGNED NOT NULL);

DROP INDEX IF EXISTS sessioneffector_session_id_effectortype_id_channel;
CREATE UNIQUE INDEX sessioneffector_session_id_effectortype_id_channel
    ON "sessioneffector"(session_id, effectortype_id, channel);

--
-- temperature
--
DROP TABLE IF EXISTS "temperature";
CREATE TABLE "temperature"(
    date_create         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    sensor_id           TINYINT UNSIGNED NOT NULL,
    temperature         DECIMAL(4, 6) NOT NULL);

DROP INDEX IF EXISTS temperature_date_create_sensor_id;
CREATE UNIQUE INDEX temperature_date_create_sensor_id
    ON "temperature"(date_create, sensor_id);

--
-- temperaturesensor
--
DROP TABLE IF EXISTS "temperaturesensor";
CREATE TABLE "temperaturesensor"(
    role                CHAR(16) NOT NULL COLLATE NOCASE,
    session_id          INT UNSIGNED NOT NULL,
    channel             INT UNSIGNED NOT NULL,
    thermistor_id       INT UNSIGNED NOT NULL);

--
-- thermistor
--
DROP TABLE IF EXISTS "thermistor";
CREATE TABLE "thermistor"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(3) NOT NULL COLLATE NOCASE,
    Tref_C              DECIMAL(4, 4) NOT NULL,
    Rref                DECIMAL(7, 3) NOT NULL,
    beta                DECIMAL(7, 3) NOT NULL,
    range_min           DECIMAL(4, 4) NOT NULL,
    range_max           DECIMAL(4, 4) NOT NULL);


DELETE FROM effectortype;
INSERT INTO effectortype(id, name, type, powerconsumption) VALUES
    (1, "FV heater 30W",    "heater",  30.0),
    (2, "Coolant pump 18W", "cooler", 18.0);

DELETE FROM profile;
INSERT INTO profile(id, name, type) VALUES
    (   1, "Lager, three-stage",    "ferment"),
    (   2, "Lager",                 "condition"),
    (   3, "Ale, two-stage",        "ferment"),
    (   4, "Ale, two-stage",        "condition"),
    (1000, "Light lager",           "serve"),
    (1001, "Pilsner",               "serve"),
    (1002, "Munich Helles",         "serve"),
    (1003, "Weissbier",             "serve"),
    (1004, "Kölsch",                "serve"),
    (1005, "IPA",                   "serve"),
    (1006, "American pale ale",     "serve"),
    (1007, "Porter",                "serve"),
    (1008, "Stout",                 "serve"),
    (1009, "Belgian ale",           "serve"),
    (1010, "Sour ale",              "serve"),
    (1011, "Bock",                  "serve"),
    (1012, "English bitter",        "serve"),
    (1013, "Scottish ale",          "serve"),
    (1014, "Imperial stout",        "serve"),
    (1015, "Belgian strong ale",    "serve"),
    (1016, "Doppelbock",            "serve"),
    (2000, "Bottling",              "serve");       -- Profile used to chill beer in preparation for bottling
    
DELETE FROM profilestage;
INSERT INTO profilestage(profile_id, stage, duration_hours, temperature) VALUES
    (   1,  1, 15 * 24, 12.0),      -- Lager, three-stage, ferment
    (   1,  2,  3 * 24, 17.0),
    (   2,  1,      24, 15.0),      -- Lager, condition
    (   2,  2,      24, 14.5),
    (   2,  3,      24, 14.0),
    (   2,  4,      24, 13.5),
    (   2,  5,      24, 13.0),
    (   2,  6,      24, 12.5),
    (   2,  7,      24, 12.0),
    (   2,  8,      24, 11.5),
    (   2,  9,      24, 11.0),
    (   2, 10,      24, 10.5),
    (   2, 11,      24, 10.0),
    (   2, 12,      24,  9.5),
    (   2, 13,      24,  9.0),
    (   2, 14,      24,  8.5),
    (   2, 15,      24,  8.0),
    (   2, 16,      24,  7.5),
    (   2, 17,      24,  7.0),
    (   2, 18,      24,  6.5),
    (   2, 19,      24,  6.0),
    (   2, 20,      24,  5.5),
    (   2, 21, 10 * 24,  5.0),
    (   3,  1, 14 * 24, 19.0),      -- Ale, two-stage, ferment
    (   4,  1, 30 * 24, 18.0),      -- Ale, two-stage, condition
    (1000,  1,    NULL,  4.0),      -- Light lager
    (1001,  1,    NULL,  6.0),      -- Pilsner
    (1002,  1,    NULL,  6.0),      -- Munich Helles
    (1003,  1,    NULL,  6.0),      -- Weissbier
    (1004,  1,    NULL,  6.0),      -- Kölsch
    (1005,  1,    NULL,  9.0),      -- IPA
    (1006,  1,    NULL,  9.0),      -- American pale ale
    (1007,  1,    NULL,  9.0),      -- Porter
    (1008,  1,    NULL,  9.0),      -- Stout
    (1009,  1,    NULL, 12.0),      -- Belgian ale
    (1010,  1,    NULL, 12.0),      -- Sour ale
    (1011,  1,    NULL, 12.0),      -- Bock
    (1012,  1,    NULL, 12.0),      -- English bitter
    (1013,  1,    NULL, 12.0),      -- Scottish ale
    (1014,  1,    NULL, 15.0),      -- Imperial stout
    (1015,  1,    NULL, 15.0),      -- Belgian strong ale
    (1016,  1,    NULL, 15.0),      -- Doppelbock
    (2000,  1,    NULL,  2.0);      -- Profile used to chill beer in preparation for bottling

DELETE FROM thermistor;
INSERT INTO thermistor(id, name, type, Tref_C, Rref, beta, range_min, range_max) VALUES
    (1, "B57891M0472K000", "NTC", 25.0, 4700.0, 3980.0, -5.0, 40.0);

-- remove this
DELETE FROM gyle;
INSERT INTO gyle(id, name, date_create) VALUES
    (112, "Nelson Sauvin lager", '2018-01-13 15:00:00'),
    (113, "Rye lager", '2018-01-13 20:00:00');

DELETE FROM session;
INSERT INTO session(id, gyle_id, profile_id, date_create, date_start, date_finish) VALUES
    (1, 112, 1, '2018-01-13 15:00:00', '2018-01-13 15:00:00', NULL),
    (2, 113, 1, '2018-01-13 18:00:00', '2018-01-13 18:00:00', NULL);

DELETE FROM sessioneffector;
INSERT INTO sessioneffector(session_id, effectortype_id, channel) VALUES
    (1, 1, 0),
    (1, 2, 1),
    (2, 1, 2),
    (2, 2, 3);

DELETE FROM temperaturesensor;
INSERT INTO temperaturesensor(role, session_id, channel, thermistor_id) VALUES
    ("vessel",  1, 0, 1),
    ("ambient", 0, 7, 1),
    ("vessel", 2, 1, 1);

DELETE FROM temperature;
VACUUM;

