import os
from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    database_url: str = "postgresql+psycopg2://greenhouse_user:greenhouse_password@localhost:5432/greenhouse_db"
    api_title: str = "Greenhouse Management API"
    api_version: str = "1.0.0"
    debug: bool = False

    class Config:
        env_file = ".env"

settings = Settings()
