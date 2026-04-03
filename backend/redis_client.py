import json
from typing import Any, Optional

import redis

from config import settings


redis_client = redis.Redis.from_url(settings.redis_url, decode_responses=True)


def ping_redis() -> bool:
    try:
        return bool(redis_client.ping())
    except Exception:
        return False


def set_json(key: str, value: Any, ttl_seconds: Optional[int] = None) -> None:
    payload = json.dumps(value, default=str)
    if ttl_seconds is not None:
        redis_client.setex(key, ttl_seconds, payload)
    else:
        redis_client.set(key, payload)


def get_json(key: str) -> Optional[dict]:
    raw = redis_client.get(key)
    if raw is None:
        return None
    return json.loads(raw)
