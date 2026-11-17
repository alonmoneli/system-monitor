import os
import unittest
from unittest.mock import patch

import psycopg
from fastapi.testclient import TestClient

from backend.app import app


TEST_DB = "dbname=system_monitor_test"


class EventIntegrationTest(unittest.TestCase):
    def test_posted_alert_appears_in_history(self):
        payload = {"kind": "alert", "cpu_percent": 85.5}

        with patch.dict(os.environ, {"MONITOR_DB": TEST_DB}):
            with TestClient(app) as client:
                created = client.post("/events", json=payload)
                self.assertEqual(created.status_code, 200, created.text)

                event_id = created.json()["id"]
                self.addCleanup(self.delete_test_event, event_id)

                history = client.get("/events")
                self.assertEqual(history.status_code, 200, history.text)

                matches = [row for row in history.json() if row["id"] == event_id]
                self.assertEqual(len(matches), 1)
                self.assertEqual(matches[0]["kind"], "alert")
                self.assertEqual(matches[0]["cpu_percent"], 85.5)

    def delete_test_event(self, event_id):
        with psycopg.connect(TEST_DB) as conn:
            conn.execute("DELETE FROM cpu_events WHERE id = %s", (event_id,))

    def test_cpu_percentage_above_100_is_rejected(self):
        with patch.dict(os.environ, {"MONITOR_DB": TEST_DB}):
            with TestClient(app) as client:
                response = client.post(
                    "/events",
                    json={"kind": "alert", "cpu_percent": 105.0},
                )

        self.assertEqual(response.status_code, 422, response.text)
