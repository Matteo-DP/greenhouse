export type DeviceType = 'SENSOR' | 'LIGHT' | 'PUMP' | 'MOISTURE' | 'FAN'

export interface Device {
  id: string
  name: string
  description: string | null
  device_type: DeviceType
  location: string | null
  firmware: string | null
  unit: string | null
  created_at: string
  updated_at: string
}

export interface SensorReading {
  sensor_id: string
  time: string
  value: number
}

export interface LightSchedule {
  id: string
  name: string
  description: string | null
  light_on_start: string | null
  light_on_end: string | null
  brightness_level: number
  created_at: string
  updated_at: string
}

export interface LightSchedulePayload {
  name: string
  description?: string | null
  light_on_start?: string | null
  light_on_end?: string | null
  brightness_level: number
}

export interface WateringSchedule {
  id: string
  name: string
  duration_seconds: number
  moisture_threshold_max: number | null
  moisture_threshold_auto: number | null
  moisture_auto_enabled: boolean
  frequency_hours: number
  created_at: string
  updated_at: string
}

export interface WateringSchedulePayload {
  name: string
  duration_seconds: number
  moisture_threshold_max?: number | null
  moisture_threshold_auto?: number | null
  moisture_auto_enabled: boolean
  frequency_hours: number
}
