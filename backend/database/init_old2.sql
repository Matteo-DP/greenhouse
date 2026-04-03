-- Enable TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;

-- ==================== SENSOR READINGS ====================

-- Sensor readings table (hypertable for time-series data)
CREATE TABLE IF NOT EXISTS sensor_readings (
    time TIMESTAMPTZ NOT NULL,
    sensor_id UUID NOT NULL,
    sensor_type TEXT NOT NULL,  -- e.g. 'TEMPERATURE', 'HUMIDITY', 'MOISTURE'
    unit TEXT NOT NULL,          -- e.g. 'celsius', 'percent', 'ppm'
    value FLOAT NOT NULL,
    UNIQUE(time, sensor_id, sensor_type)
);

-- Device configuration table
CREATE TABLE IF NOT EXISTS devices (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    description TEXT,
    device_type TEXT NOT NULL CHECK (device_type IN ('SENSOR', 'LIGHT', 'PUMP', 'FAN')),
    location TEXT,
    enabled BOOLEAN DEFAULT TRUE,
    last_seen TIMESTAMPTZ,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Add foreign key constraint to sensor_readings
ALTER TABLE sensor_readings ADD CONSTRAINT fk_sensor_readings_device_id
    FOREIGN KEY (sensor_id) REFERENCES devices(id) ON DELETE RESTRICT;

-- Create hypertable for efficient time-series storage
SELECT create_hypertable('sensor_readings', by_time_column_name => 'time', if_not_exists => TRUE);

-- Create indexes for faster queries
CREATE INDEX IF NOT EXISTS idx_sensor_readings_sensor_id_time ON sensor_readings (sensor_id, time DESC);
CREATE INDEX IF NOT EXISTS idx_sensor_readings_type ON sensor_readings (sensor_type);
CREATE INDEX IF NOT EXISTS idx_devices_status ON devices(enabled);

-- ==================== SCHEDULES ====================

-- Light scheduling table
CREATE TABLE IF NOT EXISTS light_schedules (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    description TEXT,
    enabled BOOLEAN DEFAULT TRUE,
    light_on_start TIME NOT NULL,
    light_on_end TIME NOT NULL,
    brightness_level INTEGER CHECK (brightness_level >= 0 AND brightness_level <= 100) NOT NULL DEFAULT 100,
    days_of_week TEXT[] DEFAULT ARRAY['MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT', 'SUN'],
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    CONSTRAINT chk_light_schedule_times CHECK (light_on_start < light_on_end)
);

CREATE INDEX IF NOT EXISTS idx_light_schedules_enabled ON light_schedules(enabled);

-- Junction table: devices to light schedules
CREATE TABLE IF NOT EXISTS device_light_schedules (
    device_id UUID NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    schedule_id UUID NOT NULL REFERENCES light_schedules(id) ON DELETE CASCADE,
    enabled BOOLEAN DEFAULT TRUE,
    PRIMARY KEY (device_id, schedule_id)
);

CREATE INDEX IF NOT EXISTS idx_device_light_schedules_device ON device_light_schedules(device_id);
CREATE INDEX IF NOT EXISTS idx_device_light_schedules_schedule ON device_light_schedules(schedule_id);

-- Watering scheduling table
CREATE TABLE IF NOT EXISTS watering_schedules (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name TEXT NOT NULL,
    description TEXT,
    enabled BOOLEAN DEFAULT TRUE,
    frequency_hours INTEGER NOT NULL CHECK (frequency_hours > 0),
    duration_seconds INTEGER NOT NULL CHECK (duration_seconds > 0),
    moisture_threshold_max FLOAT,          -- don't water if above this level
    moisture_threshold_auto FLOAT,         -- auto-water if below this level
    moisture_auto_enabled BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_watering_schedules_enabled ON watering_schedules(enabled);

-- Junction table: devices to watering schedules
CREATE TABLE IF NOT EXISTS device_watering_schedules (
    device_id UUID NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    schedule_id UUID NOT NULL REFERENCES watering_schedules(id) ON DELETE CASCADE,
    last_watered TIMESTAMPTZ,
    next_watering TIMESTAMPTZ,
    enabled BOOLEAN DEFAULT TRUE,
    PRIMARY KEY (device_id, schedule_id)
);

CREATE INDEX IF NOT EXISTS idx_device_watering_schedules_device ON device_watering_schedules(device_id);
CREATE INDEX IF NOT EXISTS idx_device_watering_schedules_schedule ON device_watering_schedules(schedule_id);
CREATE INDEX IF NOT EXISTS idx_device_watering_schedules_next ON device_watering_schedules(next_watering);

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
