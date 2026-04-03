from pydantic_settings import BaseSettings, SettingsConfigDict

class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_file=".env", extra="ignore")

    database_url: str = "postgresql+psycopg2://greenhouse_user:greenhouse_password@localhost:5432/greenhouse_db"
    redis_url: str = "redis://localhost:6379/0"
    redis_sensor_cache_ttl_seconds: int = 3600
    api_title: str = "Greenhouse Management API"
    api_version: str = "1.0.0"
    debug: bool = False

settings = Settings()
