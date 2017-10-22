CREATE TABLE IF NOT EXISTS "effectortype"(
    id INTEGER PRIMARY KEY NOT NULL,
    name VARCHAR(255) NOT NULL,
    type CHAR(16) NOT NULL,
    powerconsumption DECIMAL(4, 4) NOT NULL);

CREATE TABLE IF NOT EXISTS "gyle"(
    id INTEGER PRIMARY KEY NOT NULL,
    name VARCHAR(255) NOT NULL,
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);

CREATE TABLE IF NOT EXISTS "profile"(
    id INTEGER PRIMARY KEY NOT NULL,
    name VARCHAR(255) NOT NULL,
    type CHAR(16) NOT NULL);

CREATE TABLE IF NOT EXISTS "profilestage"(
    profile_id INT UNSIGNED NOT NULL,
    stage INT UNSIGNED NOT NULL,
    duration_hours INT UNSIGNED NOT NULL,
    temperature DECIMAL(4, 4));

CREATE TABLE IF NOT EXISTS "session"(
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    gyle VARCHAR(255) NOT NULL,
    profile_id INT UNSIGNED NOT NULL,
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    date_start DATETIME DEFAULT NULL,
    date_finish DATETIME DEFAULT NULL);

CREATE TABLE IF NOT EXISTS "sessionsensor"(
    session_id INT UNSIGNED NOT NULL,
    sensor_id INT UNSIGNED NOT NULL);

CREATE TABLE IF NOT EXISTS "sessioneffector"(
    session_id INT UNSIGNED NOT NULL,
    effectortype_id INT UNSIGNED NOT NULL,
    num INT UNSIGNED NOT NULL DEFAULT 0);

CREATE TABLE IF NOT EXISTS "temperature"(
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    sensor_id INT UNSIGNED NOT NULL,
    temperature DECIMAL(4, 4) NOT NULL);

CREATE UNIQUE INDEX IF NOT EXISTS profilestage_profile_id_stage
    ON "profilestage"(profile_id, stage);

CREATE UNIQUE INDEX IF NOT EXISTS session_id_date_finish
    ON "session"(id, date_finish);

CREATE UNIQUE INDEX IF NOT EXISTS sessionsensor_session_id_sensor_id
    ON "sessionsensor"(session_id, sensor_id);

CREATE UNIQUE INDEX IF NOT EXISTS sessioneffector_session_id_effectortype_id_num
    ON "sessioneffector"(session_id, effectortype_id, num);

CREATE UNIQUE INDEX IF NOT EXISTS temperature_date_create_sensor_id
    ON "temperature"(date_create, sensor_id);

INSERT INTO profile(id, name, type) VALUES
    (1, "Lager, three-stage", "Ferment"),
    (2, "Lager", "Condition"),
    (3, "Ale, two-stage", "Ferment"),
    (4, "Ale, two-stage", "Condition");

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

