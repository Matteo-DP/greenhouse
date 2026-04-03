-- Enable TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

-- ----------------- SENSOR READINGS -------------------

-- Sensor readings table (hypertable for time-series data)
CREATE TABLE IF NOT EXISTS sensor_readings (
    time TIMESTAMPTZ NOT NULL,
    sensor_id UUID NOT NULL,
    value FLOAT NOT NULL,
);

-- Device configuration table
CREATE TABLE IF NOT EXISTS devices (
    id UUID NOT NULL REFERENCES sensor_readings(sensor_id) ON DELETE RESTRICT, -- don't delete device if there are readings
    name TEXT NOT NULL,
    description TEXT,
    device_type TEXT NOT NULL CHECK (device_type IN ('SENSOR', 'LIGHT', 'PUMP', 'FAN')),
    location TEXT,
    last_seen TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Create hypertable for efficient time-series storage
SELECT create_hypertable('sensor_readings', by_time_column_name => 'time', if_not_exists => TRUE);

-- Create index for faster queries
CREATE INDEX IF NOT EXISTS idx_sensor_readings_sensor_id_time ON sensor_readings (sensor_id, time DESC);

-- ----------------- SCHEDULES -------------------

-- Metadata tables for scheduling
CREATE TABLE IF NOT EXISTS light_schedules (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    description TEXT,
    light_on_start TIME,
    light_on_end TIME,
    light_interval INTERVAL GENERATED ALWAYS AS (light_on_end - light_on_start) STORED,
    brightness_level INTEGER CHECK (brightness_level >= 0 AND brightness_level <= 100) NOT NULL DEFAULT 100,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE CONSTRAINT IF NOT EXISTS chk_light_schedule_times CHECK (
    (light_on_start IS NOT NULL AND light_on_end IS NOT NULL AND light_on_start < light_on_end) OR
    (light_on_start IS NULL AND light_on_end IS NULL)
);

INSERT INTO light_schedules (name, description, light_on_start, light_on_end, brightness_level) VALUES
('Always on', 'Light is always on', '00:00:00', NULL, 100),
('Always off', 'Light is always off', NULL, NULL, 0);

CREATE TABLE IF NOT EXISTS light_schedules_dates (
    schedule_id UUID NOT NULL REFERENCES light_schedules(id) ON DELETE NO ACTION,
    start_date DATE NOT NULL, -- end date is the start date off the next schedule
    PRIMARY KEY (schedule_id, start_date)
    enabled BOOLEAN DEFAULT TRUE
)

CREATE TABLE IF NOT EXISTS watering_schedules (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    duration_seconds INTEGER NOT NULL,
    moisture_threshold_max FLOAT, -- threshold to NOT water if moisture is above this level
    moisture_threshold_auto FLOAT, -- threshold to automatically water
    moisture_auto_enabled BOOLEAN DEFAULT FALSE, -- whether to automatically water based on moisture threshold
    -- last_watered TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS watering_schedules_dates (
    schedule_id UUID NOT NULL REFERENCES watering_schedules(id) ON DELETE NO ACTION,
    start_date DATE NOT NULL, -- end date is the start date off the next schedule
    PRIMARY KEY (schedule_id, start_date)
    enabled BOOLEAN DEFAULT TRUE,
)
