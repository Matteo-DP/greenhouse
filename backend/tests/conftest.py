from __future__ import annotations

from datetime import datetime
from typing import Any, Callable
from uuid import UUID, uuid4

import pytest
from fastapi.testclient import TestClient

from database import get_db
from main import app


class FakeQuery:
    def __init__(self, session: "FakeSession", model: type):
        self.session = session
        self.model = model
        self._filters: list[Callable[[Any], bool]] = []
        self._order_key: str | None = None
        self._reverse = False
        self._limit: int | None = None

    def _extract_value(self, right: Any) -> Any:
        return getattr(right, "value", right)

    def _predicate_from_expression(self, expr: Any) -> Callable[[Any], bool]:
        left = getattr(expr, "left", None)
        right = getattr(expr, "right", None)

        key = getattr(left, "key", None)
        value = self._extract_value(right)

        def _pred(obj: Any) -> bool:
            return getattr(obj, key) == value

        return _pred

    def filter(self, *expressions: Any) -> "FakeQuery":
        for expr in expressions:
            self._filters.append(self._predicate_from_expression(expr))
        return self

    def order_by(self, ordering: Any) -> "FakeQuery":
        if hasattr(ordering, "element") and hasattr(ordering.element, "key"):
            self._order_key = ordering.element.key
        elif hasattr(ordering, "key"):
            self._order_key = ordering.key
        self._reverse = "DESC" in str(ordering).upper()
        return self

    def limit(self, value: int) -> "FakeQuery":
        self._limit = value
        return self

    def _apply(self) -> list[Any]:
        rows = list(self.session.store[self.model])
        for pred in self._filters:
            rows = [row for row in rows if pred(row)]
        if self._order_key:
            rows = sorted(rows, key=lambda x: getattr(x, self._order_key), reverse=self._reverse)
        if self._limit is not None:
            rows = rows[: self._limit]
        return rows

    def all(self) -> list[Any]:
        return self._apply()

    def first(self) -> Any | None:
        rows = self._apply()
        return rows[0] if rows else None


class FakeSession:
    def __init__(self) -> None:
        from models import (
            Alert,
            Device,
            LightSchedule,
            LightScheduleDate,
            Log,
            SensorReading,
            WateringSchedule,
            WateringScheduleDate,
        )

        self.models = {
            Device,
            SensorReading,
            LightSchedule,
            LightScheduleDate,
            WateringSchedule,
            WateringScheduleDate,
            Log,
            Alert,
        }
        self.store: dict[type, list[Any]] = {m: [] for m in self.models}

    def query(self, model: type) -> FakeQuery:
        return FakeQuery(self, model)

    def add(self, obj: Any) -> None:
        now = datetime.utcnow()

        if hasattr(obj, "id") and getattr(obj, "id", None) is None:
            setattr(obj, "id", uuid4())

        if hasattr(obj, "created_at") and getattr(obj, "created_at", None) is None:
            setattr(obj, "created_at", now)

        if hasattr(obj, "updated_at") and getattr(obj, "updated_at", None) is None:
            setattr(obj, "updated_at", now)

        if hasattr(obj, "resolved") and getattr(obj, "resolved", None) is None:
            setattr(obj, "resolved", False)

        if hasattr(obj, "enabled") and getattr(obj, "enabled", None) is None:
            setattr(obj, "enabled", True)

        self.store[type(obj)].append(obj)

    def delete(self, obj: Any) -> None:
        rows = self.store[type(obj)]
        self.store[type(obj)] = [row for row in rows if row is not obj]

    def commit(self) -> None:
        return None

    def refresh(self, obj: Any) -> None:
        return None

    def close(self) -> None:
        return None


@pytest.fixture
def fake_db() -> FakeSession:
    return FakeSession()


@pytest.fixture(autouse=True)
def fake_sensor_cache(monkeypatch: pytest.MonkeyPatch) -> dict[str, Any]:
    from routers import sensor_readings as sensor_router

    cache: dict[str, Any] = {}

    def _get_json(key: str):
        return cache.get(key)

    def _set_json(key: str, value: Any, ttl_seconds: int | None = None):
        cache[key] = value

    monkeypatch.setattr(sensor_router, "get_json", _get_json)
    monkeypatch.setattr(sensor_router, "set_json", _set_json)
    return cache


@pytest.fixture
def client(fake_db: FakeSession):
    def override_get_db():
        yield fake_db

    app.dependency_overrides[get_db] = override_get_db
    with TestClient(app) as test_client:
        yield test_client
    app.dependency_overrides.clear()
