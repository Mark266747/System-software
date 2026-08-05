from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QColor, QFont, QPen
from PyQt6.QtCore import Qt, QRect, QTimer
import subprocess
import re

NODES = [
    {"name": "ПК",        "color_ok": "#2ecc71"},
    {"name": "Роутер",    "color_ok": "#2ecc71"},
    {"name": "Провайдер", "color_ok": "#2ecc71"},
    {"name": "Сервер",    "color_ok": "#2ecc71"},
]

class ConnectionVisualizer(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumHeight(200)
        self._latency   = 0.0
        self._problem   = -1
        self._hops      = []


        self._timer = QTimer()
        self._timer.timeout.connect(self._run_traceroute)
        self._timer.start(60000)
        self._run_traceroute()

    def update_latency(self, ms: float):
        self._latency = ms
        if ms < 0:
            self._problem = 1
        elif ms > 200:
            self._problem = 2
        elif ms > 100:
            self._problem = 3
        else:
            self._problem = -1
        self.update()

    def _run_traceroute(self):
        import threading
        thread = threading.Thread(
            target=self._traceroute_worker,
            daemon=True
        )
        thread.start()

    def _traceroute_worker(self):
        try:
            result = subprocess.run(
                ["tracert", "-d", "-h", "5", "-w", "1000", "8.8.8.8"],
                capture_output=True, text=True,
                timeout=30, encoding="cp866"
            )
            hops = self._parse_traceroute(result.stdout)
            self._hops = hops
            self._update_problem_node(hops)
            self.update()
        except Exception as e:
            print(f"[Traceroute] Error: {e}")

    def _parse_traceroute(self, output: str) -> list:
        hops = []
        pattern = re.compile(
            r"\s*(\d+)\s+"
            r"(?:(\d+)\s+ms|\*)\s+"
            r"(?:(\d+)\s+ms|\*)\s+"
            r"(?:(\d+)\s+ms|\*)\s+"
            r"([\d.]+|\*)"
        )
        for line in output.splitlines():
            m = pattern.search(line)
            if m:
                times = [m.group(i) for i in (2, 3, 4) if m.group(i)]
                avg = sum(int(t) for t in times) / len(times) if times else -1
                hops.append({
                    "hop": int(m.group(1)),
                    "ip":  m.group(5) or "*",
                    "ms":  avg
                })
        return hops

    def _update_problem_node(self, hops: list):
        self._problem = -1
        prev_ms = 0
        for i, hop in enumerate(hops[:4]):
            if hop["ms"] < 0:
                self._problem = min(i + 1, 3)
                break
            if hop["ms"] - prev_ms > 80:
                self._problem = min(i + 1, 3)
                break
            prev_ms = hop["ms"]

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)

        w = self.width()
        h = self.height()
        n = len(NODES)
        r = 38
        spacing = w / (n + 1)

        for i, node in enumerate(NODES):
            cx = int(spacing * (i + 1))
            cy = h // 2

            if i < n - 1:
                next_cx = int(spacing * (i + 2))
                line_color = (QColor("#e74c3c")
                              if self._problem == i + 1
                              else QColor("#444466"))
                p.setPen(QPen(line_color, 3))
                p.drawLine(cx + r, cy, next_cx - r, cy)

            if i == self._problem:
                fill = QColor("#e74c3c")
            elif self._problem < 0 or i < self._problem:
                fill = QColor("#2ecc71")
            else:
                fill = QColor("#2d2d44")

            p.setPen(QPen(QColor("#aaaacc"), 2))
            p.setBrush(fill)
            p.drawEllipse(cx - r, cy - r, r * 2, r * 2)

            p.setPen(QColor("#ffffff"))
            p.setFont(QFont("Segoe UI", 9, QFont.Weight.Bold))
            p.drawText(
                QRect(cx - r, cy - r, r * 2, r * 2),
                Qt.AlignmentFlag.AlignCenter,
                node["name"]
            )

            if i < len(self._hops):
                ms = self._hops[i]["ms"]
                label = f"{ms:.0f}ms" if ms >= 0 else "*"
                p.setPen(QColor("#aaaacc"))
                p.setFont(QFont("Segoe UI", 8))
                p.drawText(
                    QRect(cx - r, cy + r + 4, r * 2, 20),
                    Qt.AlignmentFlag.AlignCenter,
                    label
                )

        p.setPen(QColor("#aaaacc"))
        p.setFont(QFont("Segoe UI", 9))
        status = f"Ping: {self._latency:.0f} мс"
        if self._problem >= 0:
            status += f"  ⚠ проблема: {NODES[self._problem]['name']}"
        p.drawText(10, h - 8, status)