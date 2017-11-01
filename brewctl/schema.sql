--
-- effectortype
--
DROP TABLE IF EXISTS "effectortype";
CREATE TABLE "effectortype"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(16) NOT NULL,
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
-- profile
--
DROP TABLE IF EXISTS "profile";
CREATE TABLE "profile"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(16) NOT NULL);

--
-- profilestage
--
DROP TABLE IF EXISTS "profilestage";
CREATE TABLE "profilestage"(
    profile_id          INT UNSIGNED NOT NULL,
    stage               INT UNSIGNED NOT NULL,
    duration_hours      INT UNSIGNED NOT NULL,
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
    type                CHAR(16) NOT NULL,
    type_id             INT UNSIGNED NOT NULL);

--
-- session
--
DROP TABLE IF EXISTS "session";
CREATE TABLE "session"(
    id                  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    gyle                VARCHAR(255) NOT NULL,
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
    sensor_id           INT UNSIGNED NOT NULL,
    temperature         DECIMAL(4, 4) NOT NULL);

DROP INDEX IF EXISTS temperature_date_create_sensor_id;
CREATE UNIQUE INDEX temperature_date_create_sensor_id
    ON "temperature"(date_create, sensor_id);

--
-- temperaturesensor
--
DROP TABLE IF EXISTS "temperaturesensor";
CREATE TABLE "temperaturesensor"(
    role                CHAR(16) NOT NULL,
    session_id          INT UNSIGNED DEFAULT NULL,
    channel             INT UNSIGNED NOT NULL,
    thermistor_id       INT UNSIGNED NOT NULL);

--
-- thermistor
--
DROP TABLE IF EXISTS "thermistor";
CREATE TABLE "thermistor"(
    id                  INTEGER PRIMARY KEY NOT NULL,
    name                VARCHAR(255) NOT NULL,
    type                CHAR(3) NOT NULL,
    Tref_C              DECIMAL(4, 4) NOT NULL,
    Rref                DECIMAL(7, 3) NOT NULL,
    beta                DECIMAL(7, 3) NOT NULL,
    range_min           DECIMAL(4, 4) NOT NULL,
    range_max           DECIMAL(4, 4) NOT NULL);


DELETE FROM effectortype;
INSERT INTO effectortype(id, name, type, powerconsumption) VALUES
    (1, "FV heater 30W",    "Heater",  30.0),
    (2, "Coolant pump 18W", "Chiller", 18.0);

DELETE FROM profile;
INSERT INTO profile(id, name, type) VALUES
    (1, "Lager, three-stage", "Ferment"),
    (2, "Lager",              "Condition"),
    (3, "Ale, two-stage",     "Ferment"),
    (4, "Ale, two-stage",     "Condition");

DELETE FROM profilestage;
INSERT INTO profilestage(profile_id, stage, duration_hours, temperature) VALUES
    (1,  1, 15 * 24, 12.0),
    (1,  2,  3 * 24, 17.0),
    (2,  1,      24, 15.0),
    (2,  2,      24, 14.5),
    (2,  3,      24, 14.0),
    (2,  4,      24, 13.5),
    (2,  5,      24, 13.0),
    (2,  6,      24, 12.5),
    (2,  7,      24, 12.0),
    (2,  8,      24, 11.5),
    (2,  9,      24, 11.0),
    (2, 10,      24, 10.5),
    (2, 11,      24, 10.0),
    (2, 12,      24,  9.5),
    (2, 13,      24,  9.0),
    (2, 14,      24,  8.5),
    (2, 15,      24,  8.0),
    (2, 16,      24,  7.5),
    (2, 17,      24,  7.0),
    (2, 18,      24,  6.5),
    (2, 19,      24,  6.0),
    (2, 20,      24,  5.5),
    (2, 21, 10 * 24,  5.0),
    (3,  1, 14 * 24, 19.0),
    (4,  1, 30 * 24, 18.0);

DELETE FROM thermistor;
INSERT INTO thermistor(id, name, type, Tref_C, Rref, beta, range_min, range_max) VALUES
    (1, "B57891M0472K000", "NTC", 25.0, 4700.0, 3980.0, -5.0, 40.0);

-- remove this
DELETE FROM session;
INSERT INTO session(id, gyle, profile_id, date_create, date_start, date_finish) VALUES
    (1, "G107", 1, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, NULL);

DELETE FROM sessioneffector;
INSERT INTO sessioneffector(session_id, effectortype_id, channel) VALUES
    (1, 1, 0),
    (1, 2, 1);

DELETE FROM temperaturesensor;
INSERT INTO temperaturesensor(role, session_id, channel, thermistor_id) VALUES
    ("vessel",     1, 0, 1),
    ("ambient", NULL, 7, 1);

