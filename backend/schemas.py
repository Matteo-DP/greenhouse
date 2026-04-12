from pydantic import BaseModel
from datetime import datetime, date, time
from uuid import UUID
from typing import Optional, List
from enum import Enum


class DeviceType(str, Enum):
    SENSOR = "SENSOR"
    LIGHT = "LIGHT"
    PUMP = "PUMP"
    MOISTURE = "MOISTURE"
    FAN = "FAN"


class LogLevel(str, Enum):
    DEBUG = "DEBUG"
    INFO = "INFO"
    WARNING = "WARNING"
    ERROR = "ERROR"
    CRITICAL = "CRITICAL"


class AlertSeverity(str, Enum):
    INFO = "INFO"
    WARNING = "WARNING"
    CRITICAL = "CRITICAL"

# ==================== DEVICES ====================

class DeviceBase(BaseModel):
    name: str
    description: Optional[str] = None
    device_type: DeviceType
    location: str = None
    firmware: str = None
    unit: Optional[str] = None

class DeviceCreate(DeviceBase):
    pass

class DeviceUpdate(BaseModel):
    name: Optional[str] = None
    description: Optional[str] = None
    location: str = None
    firmware: Optional[str] = None
    unit: Optional[str] = None

class Device(DeviceBase):
    id: UUID
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

# ==================== SENSOR READINGS ====================

class SensorReadingBase(BaseModel):
    value: float

class SensorReadingCreate(SensorReadingBase):
    sensor_id: UUID
    time: datetime

class SensorReading(SensorReadingBase):
    time: datetime
    sensor_id: UUID
    
    class Config:
        from_attributes = True

# ==================== LIGHT SCHEDULES ====================

class LightScheduleDateBase(BaseModel):
    start_date: date
    enabled: bool = True

class LightScheduleBase(BaseModel):
    name: str
    description: Optional[str] = None
    light_on_start: Optional[time] = None
    light_on_end: Optional[time] = None
    brightness_level: int = 100

class LightScheduleCreate(LightScheduleBase):
    pass

class LightScheduleUpdate(BaseModel):
    name: Optional[str] = None
    description: Optional[str] = None
    light_on_start: Optional[time] = None
    light_on_end: Optional[time] = None
    brightness_level: Optional[int] = None

class LightSchedule(LightScheduleBase):
    id: UUID
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

class LightScheduleDate(LightScheduleDateBase):
    schedule_id: UUID
    
    class Config:
        from_attributes = True

# ==================== WATERING SCHEDULES ====================

class WateringScheduleDateBase(BaseModel):
    start_date: date
    enabled: bool = True

class WateringScheduleBase(BaseModel):
    name: str
    duration_seconds: int
    moisture_threshold_max: Optional[float] = None
    moisture_threshold_auto: Optional[float] = None
    moisture_auto_enabled: bool = False
    frequency_hours: int

class WateringScheduleCreate(WateringScheduleBase):
    pass

class WateringScheduleUpdate(BaseModel):
    name: Optional[str] = None
    duration_seconds: Optional[int] = None
    moisture_threshold_max: Optional[float] = None
    moisture_threshold_auto: Optional[float] = None
    moisture_auto_enabled: Optional[bool] = None
    frequency_hours: Optional[int] = None

class WateringSchedule(WateringScheduleBase):
    id: UUID
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

class WateringScheduleDate(WateringScheduleDateBase):
    schedule_id: UUID
    
    class Config:
        from_attributes = True

# ==================== LOGS ====================

class LogBase(BaseModel):
    log_level: LogLevel
    message: str

class LogCreate(LogBase):
    pass

class Log(LogBase):
    id: UUID
    device_id: UUID
    created_at: datetime
    
    class Config:
        from_attributes = True

# ==================== ALERTS ====================

class AlertBase(BaseModel):
    alert_type: str
    severity: AlertSeverity = AlertSeverity.INFO
    message: str

class AlertCreate(AlertBase):
    pass

class AlertUpdate(BaseModel):
    resolved: Optional[bool] = None
    severity: Optional[AlertSeverity] = None
    message: Optional[str] = None

class Alert(AlertBase):
    id: UUID
    device_id: UUID
    resolved: bool
    created_at: datetime
    resolved_at: Optional[datetime] = None
    
    class Config:
        from_attributes = True