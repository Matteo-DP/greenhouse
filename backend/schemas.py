from pydantic import BaseModel
from datetime import datetime, date, time
from uuid import UUID
from typing import Optional, List

# ==================== DEVICES ====================

class DeviceBase(BaseModel):
    name: str
    description: Optional[str] = None
    device_type: str  # SENSOR, LIGHT, PUMP, FAN
    location: Optional[str] = None

class DeviceCreate(DeviceBase):
    pass

class DeviceUpdate(BaseModel):
    name: Optional[str] = None
    description: Optional[str] = None
    location: Optional[str] = None

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
    log_level: str  # DEBUG, INFO, WARNING, ERROR, CRITICAL
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
    severity: str = "INFO"  # INFO, WARNING, CRITICAL
    message: str

class AlertCreate(AlertBase):
    pass

class AlertUpdate(BaseModel):
    resolved: Optional[bool] = None
    severity: Optional[str] = None
    message: Optional[str] = None

class Alert(AlertBase):
    id: UUID
    device_id: UUID
    resolved: bool
    created_at: datetime
    resolved_at: Optional[datetime] = None
    
    class Config:
        from_attributes = True
