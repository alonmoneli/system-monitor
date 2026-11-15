from fastapi import FastAPI
from typing import Literal
from pydantic import BaseModel, Field
import psycopg

app = FastAPI()

@app.get("/health")
def health():
    return {"status": "ok"}

class CpuEvent(BaseModel):
    kind: Literal["alert", "recovered"]
    cpu_percent: float = Field(ge=0, le=100)

@app.post("/events")
def receive_event(event: CpuEvent):
    with psycopg.connect("dbname=system_monitor") as conn:
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
