from sqlalchemy import Column, String, Integer, Float, Boolean, DateTime, ForeignKey, CheckConstraint, Index, Interval, Time, Date
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.orm import declarative_base
from datetime import datetime, time, date
import uuid

Base = declarative_base()

# ==================== DEVICES ====================

class Device(Base):
    __tablename__ = "devices"
    
    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    name = Column(String, nullable=False)
    description = Column(String, nullable=True)
    device_type = Column(String, nullable=False)  # SENSOR, LIGHT, PUMP, FAN, MOISTURE
    location = Column(String, nullable=True)
    created_at = Column(DateTime(timezone=True), default=datetime.utcnow)
    updated_at = Column(DateTime(timezone=True), default=datetime.utcnow, onupdate=datetime.utcnow)
    
    __table_args__ = (
        CheckConstraint("device_type IN ('SENSOR', 'LIGHT', 'PUMP', 'FAN', 'MOISTURE')"),
    )

# ==================== SENSOR READINGS ====================

class SensorReading(Base):
    __tablename__ = "sensor_readings"
    
    time = Column(DateTime(timezone=True), primary_key=True)
    sensor_id = Column(UUID(as_uuid=True), ForeignKey("devices.id"), primary_key=True)
    value = Column(Float, nullable=False)

# ==================== LIGHT SCHEDULES ====================

class LightSchedule(Base):
    __tablename__ = "light_schedules"
    
    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    name = Column(String, nullable=False)
    description = Column(String, nullable=True)
    light_on_start = Column(Time, nullable=True)
    light_on_end = Column(Time, nullable=True)
    brightness_level = Column(Integer, default=100, nullable=False)
    created_at = Column(DateTime(timezone=True), default=datetime.utcnow)
    updated_at = Column(DateTime(timezone=True), default=datetime.utcnow, onupdate=datetime.utcnow)
    
    __table_args__ = (
        CheckConstraint("brightness_level >= 0 AND brightness_level <= 100"),
    )

class LightScheduleDate(Base):
    __tablename__ = "light_schedules_dates"
    
    schedule_id = Column(UUID(as_uuid=True), ForeignKey("light_schedules.id"), primary_key=True)
    start_date = Column(Date, primary_key=True)
    enabled = Column(Boolean, default=True)

# ==================== WATERING SCHEDULES ====================

class WateringSchedule(Base):
    __tablename__ = "watering_schedules"
    
    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    name = Column(String, nullable=False)
    duration_seconds = Column(Integer, nullable=False)
    moisture_threshold_max = Column(Float, nullable=True)
    moisture_threshold_auto = Column(Float, nullable=True)
    moisture_auto_enabled = Column(Boolean, default=False)
    frequency_hours = Column(Integer, nullable=False)
    created_at = Column(DateTime(timezone=True), default=datetime.utcnow)
    updated_at = Column(DateTime(timezone=True), default=datetime.utcnow, onupdate=datetime.utcnow)
    
    __table_args__ = (
        CheckConstraint("duration_seconds > 0"),
        CheckConstraint("frequency_hours > 0"),
    )

class WateringScheduleDate(Base):
    __tablename__ = "watering_schedules_dates"
    
    schedule_id = Column(UUID(as_uuid=True), ForeignKey("watering_schedules.id"), primary_key=True)
    start_date = Column(Date, primary_key=True)
    enabled = Column(Boolean, default=True)

# ==================== LOGS & ALERTS ====================

class Log(Base):
    __tablename__ = "logs"
    
    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    device_id = Column(UUID(as_uuid=True), ForeignKey("devices.id", ondelete="CASCADE"), nullable=False)
    log_level = Column(String, nullable=False)  # DEBUG, INFO, WARNING, ERROR, CRITICAL
    message = Column(String, nullable=False)
    created_at = Column(DateTime(timezone=True), default=datetime.utcnow)
    
    __table_args__ = (
        CheckConstraint("log_level IN ('DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL')"),
    )

class Alert(Base):
    __tablename__ = "alerts"
    
    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    device_id = Column(UUID(as_uuid=True), ForeignKey("devices.id", ondelete="CASCADE"), nullable=False)
    alert_type = Column(String, nullable=False)
    severity = Column(String, default="INFO")  # INFO, WARNING, CRITICAL
    message = Column(String, nullable=False)
    resolved = Column(Boolean, default=False)
    created_at = Column(DateTime(timezone=True), default=datetime.utcnow)
    resolved_at = Column(DateTime(timezone=True), nullable=True)
    
    __table_args__ = (
        CheckConstraint("severity IN ('INFO', 'WARNING', 'CRITICAL')"),
        Index("idx_alerts_device_id", "device_id"),
        Index("idx_alerts_resolved", "resolved"),
    )
