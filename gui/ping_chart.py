from PyQt6.QtWidgets import QWidget, QVBoxLayout
from collections import deque

try:
    import pyqtgraph as pg
    HAS_GRAPH = True
except ImportError:
    HAS_GRAPH = False

MAX_POINTS = 60

class PingChart(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._ms_buf   = deque(maxlen=MAX_POINTS)
        self._xs       = list(range(MAX_POINTS))
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        if HAS_GRAPH:
            pg.setConfigOptions(antialias=True)
            self._plot = pg.PlotWidget()
            self._plot.setBackground("#1e1e2e")
            self._plot.setLabel("left",   "мс")
            self._plot.setLabel("bottom", "останні 60 пінгів")
            self._plot.showGrid(x=True, y=True, alpha=0.3)
            self._plot.setYRange(0, 200)

            self._curve = self._plot.plot(
                pen=pg.mkPen(color="#00b4d8", width=2)
            )
            self._threshold = pg.InfiniteLine(
                pos=100, angle=0,
                pen=pg.mkPen(color="#e74c3c", width=1,
                             style=pg.QtCore.Qt.PenStyle.DashLine)
            )
            self._plot.addItem(self._threshold)
            layout.addWidget(self._plot)
        else:
            from PyQt6.QtWidgets import QLabel
            layout.addWidget(QLabel("pip install pyqtgraph"))

    def add_point(self, ms: float):
        if ms < 0:
            return
        self._ms_buf.append(ms)
        if HAS_GRAPH:
            data = list(self._ms_buf)
            self._curve.setData(list(range(len(data))), data)