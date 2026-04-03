-- Enable TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;
CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- ----------------- SENSOR READINGS -------------------

-- Sensor readings table (hypertable for time-series data)
CREATE TABLE IF NOT EXISTS sensor_readings (
    time TIMESTAMPTZ NOT NULL,
    sensor_id UUID NOT NULL,
    value FLOAT NOT NULL
);

-- Device configuration table
CREATE TABLE IF NOT EXISTS devices (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    description TEXT,
    device_type TEXT NOT NULL CHECK (device_type IN ('SENSOR', 'LIGHT', 'PUMP', 'FAN')),
    location TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Create hypertable for efficient time-series storage
SELECT create_hypertable('sensor_readings', 'time', if_not_exists => TRUE);

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
    CONSTRAINT chk_light_schedule_times CHECK (
        (light_on_start IS NOT NULL AND light_on_end IS NOT NULL AND light_on_start < light_on_end) OR
        (light_on_end IS NULL AND light_on_start IS NULL)
    ),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

INSERT INTO light_schedules (name, description, light_on_start, light_on_end, brightness_level) VALUES
('Always on', 'Light is always on', '00:00:00', '23:59:59', 100),
('Always off', 'Light is always off', NULL, NULL, 0);

CREATE TABLE IF NOT EXISTS light_schedules_dates (
    schedule_id UUID NOT NULL REFERENCES light_schedules(id) ON DELETE NO ACTION,
    start_date DATE NOT NULL, -- end date is the start date off the next schedule
    PRIMARY KEY (schedule_id, start_date),
    enabled BOOLEAN DEFAULT TRUE
);

CREATE TABLE IF NOT EXISTS watering_schedules (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    duration_seconds INTEGER NOT NULL CHECK (duration_seconds > 0),
    moisture_threshold_max FLOAT, -- threshold to NOT water if moisture is above this level
    moisture_threshold_auto FLOAT, -- threshold to automatically water
    moisture_auto_enabled BOOLEAN DEFAULT FALSE, -- whether to automatically water based on moisture threshold
    frequency_hours INTEGER NOT NULL CHECK (frequency_hours > 0), -- can be more than 24h
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS watering_schedules_dates (
    schedule_id UUID NOT NULL REFERENCES watering_schedules(id) ON DELETE NO ACTION,
    start_date DATE NOT NULL, -- end date is the start date off the next schedule
    PRIMARY KEY (schedule_id, start_date),
    enabled BOOLEAN DEFAULT TRUE
);

-- ==================== ALERTS & MONITORING ====================

CREATE TABLE IF NOT EXISTS logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    device_id UUID NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    log_level TEXT NOT NULL CHECK (log_level IN ('DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL')),
    message TEXT NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Alerts table for system notifications
CREATE TABLE IF NOT EXISTS alerts (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    device_id UUID NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    alert_type TEXT NOT NULL,
    severity TEXT DEFAULT 'INFO' CHECK (severity IN ('INFO', 'WARNING', 'CRITICAL')),
    message TEXT NOT NULL,
    resolved BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    resolved_at TIMESTAMPTZ
);

CREATE INDEX IF NOT EXISTS idx_alerts_device_id ON alerts(device_id);
CREATE INDEX IF NOT EXISTS idx_alerts_resolved ON alerts(resolved);
