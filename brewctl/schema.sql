CREATE TABLE IF NOT EXISTS "gyle"(
    id INTEGER PRIMARY KEY NOT NULL,
    name VARCHAR(255) NOT NULL,
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);

CREATE TABLE IF NOT EXISTS "session"(
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    gyle VARCHAR(255) NOT NULL,
    type CHAR(1) NOT NULL,
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    date_start DATETIME DEFAULT NULL);

CREATE TABLE IF NOT EXISTS "sessionstage"(
    session_id INT UNSIGNED NOT NULL,
    stage_id INT UNSIGNED NOT NULL,
    stage_duration INT UNSIGNED NOT NULL,
    targettemperature DECIMAL(4, 4));

CREATE TABLE IF NOT EXISTS "sessionsensor"(
    session_id INT UNSIGNED NOT NULL,
    sensor_id INT UNSIGNED NOT NULL);

CREATE TABLE IF NOT EXISTS "sessioneffector"(
    session_id INT UNSIGNED NOT NULL,
    effector_id INT UNSIGNED NOT NULL);

CREATE TABLE IF NOT EXISTS "temperature"(
    date_create DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    sensor_id INT UNSIGNED NOT NULL,
    temperature DECIMAL(4, 4) NOT NULL);

CREATE UNIQUE INDEX IF NOT EXISTS sessionstage_session_id_stage_id
    ON "sessionstage"(session_id, stage_id);

CREATE UNIQUE INDEX IF NOT EXISTS sessionsensor_session_id_sensor_id
    ON "sessionsensor"(session_id, sensor_id);

CREATE UNIQUE INDEX IF NOT EXISTS sessioneffector_session_id_effector_id
    ON "sessioneffector"(session_id, effector_id);

CREATE UNIQUE INDEX IF NOT EXISTS temperature_date_create_sensor_id
    ON "temperature"(date_create, sensor_id);

