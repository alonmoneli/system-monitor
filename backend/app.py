from fastapi import FastAPI
from typing import Literal
from pydantic import BaseModel, Field

app = FastAPI()

@app.get("/health")
def health():
    return {"status": "ok"}

class CpuEvent(BaseModel):
    kind: Literal["alert", "recovered"]
    cpu_percent: float = Field(ge=0, le=100)

@app.post("/events")
def receive_event(event: CpuEvent):
    return {"received": True, "event": event}
