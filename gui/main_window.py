from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout,
    QHBoxLayout, QLabel, QStatusBar,
    QTabWidget
)
from PyQt6.QtCore import Qt
from ping_chart import PingChart
from conn_viz import ConnectionVisualizer
from report_builder import ReportBuilder

class MainWindow(QMainWindow):
    def __init__(self, ipc):
        super().__init__()
        self.ipc = ipc
        self.setWindowTitle("NetGuard")
        self.setMinimumSize(800, 500)
        self._build_ui()
        self._subscribe()

    def _build_ui(self):
            central = QWidget()
            self.setCentralWidget(central)
            main_layout = QVBoxLayout(central)

            tabs = QTabWidget()
            main_layout.addWidget(tabs)


            monitor_tab = QWidget()
            monitor_layout = QVBoxLayout(monitor_tab)

            top = QHBoxLayout()
            self.ping_label = QLabel("Ping: --")
            self.ping_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.ping_label.setStyleSheet("font-size: 28px; font-weight: bold;")
            top.addWidget(self.ping_label)

            self.jitter_label = QLabel("Jitter: --")
            self.jitter_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.jitter_label.setStyleSheet("font-size: 20px;")
            top.addWidget(self.jitter_label)

            self.loss_label = QLabel("Loss: --")
            self.loss_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.loss_label.setStyleSheet("font-size: 20px;")
            top.addWidget(self.loss_label)

            monitor_layout.addLayout(top)
            self.chart = PingChart()
            monitor_layout.addWidget(self.chart)
            tabs.addTab(monitor_tab, "Моніторинг")

            self.conn_viz = ConnectionVisualizer()
            tabs.addTab(self.conn_viz, "З'єднання")

            self.report = ReportBuilder()
            tabs.addTab(self.report, "Звіт ISP")

            self.status_bar = QStatusBar()
            self.setStatusBar(self.status_bar)
            self.status_bar.showMessage("Підключення...")

    def _subscribe(self):
        self.ipc.ping_received.connect(self._on_ping)
        self.ipc.alert_received.connect(self._on_alert)
        self.ipc.drop_received.connect(self._on_drop)

    def _on_ping(self, ms: float, jitter: float, loss: float):
        self.chart.add_point(ms)
        self.conn_viz.update_latency(ms)

        if ms < 0:
            self.ping_label.setText("TIMEOUT")
            self.ping_label.setStyleSheet(
                "font-size: 28px; font-weight: bold; color: red;")
        else:
            color = ("green" if ms < 100
                     else "orange" if ms < 200
                     else "red")
            self.ping_label.setText(f"Ping: {ms:.0f} мс")
            self.ping_label.setStyleSheet(
                f"font-size: 28px; font-weight: bold; color: {color};")

        self.jitter_label.setText(f"Jitter: {jitter:.1f} мс")
        self.loss_label.setText(f"Loss: {loss:.1f}%")
        self.status_bar.showMessage(
            f"Ping: {ms:.0f} мс  |  Jitter: {jitter:.1f} мс  |  Loss: {loss:.1f}%"
        )

    def _on_alert(self, code: str, value: float):
        self.status_bar.showMessage(f"Увага: {code} ({value:.1f}%)")

    def _on_drop(self):
        self.ping_label.setText("З'єднання перервано!")
        self.ping_label.setStyleSheet(
            "font-size: 28px; font-weight: bold; color: red;")