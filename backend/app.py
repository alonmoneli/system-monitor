from fastapi import FastAPI
from typing import Literal
from pydantic import BaseModel, Field

import os
import psycopg
from psycopg.rows import dict_row

app = FastAPI()

@app.get("/health")
def health():
    return {"status": "ok"}

class CpuEvent(BaseModel):
    kind: Literal["alert", "recovered"]
    cpu_percent: float = Field(ge=0, le=100)

@app.get("/events")
def list_events():
    with psycopg.connect(os.getenv("MONITOR_DB", "dbname=system_monitor"), row_factory=dict_row) as conn:
        cursor = conn.execute(
            """
            SELECT id, received_at, kind, cpu_percent
            FROM cpu_events
            ORDER BY id DESC
            LIMIT 20
            """
        )
        return cursor.fetchall()

@app.post("/events")
def receive_event(event: CpuEvent):
    with psycopg.connect(os.getenv("MONITOR_DB", "dbname=system_monitor")) as conn:
        cursor = conn.execute(
            """
            INSERT INTO cpu_events (kind, cpu_percent)
            VALUES (%s, %s)
            RETURNING id
            """,
            (event.kind, event.cpu_percent),
        )
        event_id = cursor.fetchone()[0]

    return {"received": True, "id": event_id, "event": event}  
