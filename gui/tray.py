from PyQt6.QtWidgets import QSystemTrayIcon, QMenu, QApplication
from PyQt6.QtGui import QIcon, QPixmap, QPainter, QColor
from PyQt6.QtCore import Qt

def make_icon(color: str) -> QIcon:
    px = QPixmap(16, 16)
    px.fill(Qt.GlobalColor.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.RenderHint.Antialiasing)
    p.setBrush(QColor(color))
    p.setPen(Qt.PenStyle.NoPen)
    p.drawEllipse(1, 1, 14, 14)
    p.end()
    return QIcon(px)

ICONS = {
    "ok":      make_icon("#2ecc71"),
    "warning": make_icon("#f39c12"),
    "error":   make_icon("#e74c3c"),
}

class SystemTray(QSystemTrayIcon):
    def __init__(self, ipc):
        super().__init__()
        self.ipc = ipc
        self.setIcon(ICONS["ok"])
        self.setToolTip("NetGuard — запуск...")
        self._build_menu()

        ipc.ping_received.connect(self._on_ping)
        ipc.alert_received.connect(self._on_alert)
        ipc.drop_received.connect(self._on_drop)

        self.activated.connect(self._on_activated)

    def _build_menu(self):
            menu = QMenu()

            self._status_action = menu.addAction("● Підключення...")
            self._status_action.setEnabled(False)
            menu.addSeparator()

            profiles_menu = menu.addMenu("Профіль")
            for profile, label in [
                ("gaming",  "Геймінг"),
                ("work",    "Робота"),
                ("balance", "Баланс"),
            ]:
                act = profiles_menu.addAction(label)
                act.triggered.connect(
                    lambda checked, p=profile: self._set_profile(p)
                )

            menu.addSeparator()

            healing_menu = menu.addMenu("Виправити зараз")
            for action, label in [
                ("flushdns", "Очистити DNS кеш"),
                ("renew",    "Оновити IP адресу"),
                ("winsock",  "Скинути Winsock"),
            ]:
                act = healing_menu.addAction(label)
                act.triggered.connect(
                    lambda checked, a=action: self._run_healing(a)
                )

            menu.addSeparator()

            dns_act = menu.addAction("Знайти найшвидший DNS")
            dns_act.triggered.connect(self._run_dns_bench)

            menu.addSeparator()

            open_act = menu.addAction("Відкрити панель")
            open_act.triggered.connect(self._open_window)
            menu.addSeparator()

            menu.addAction("Вийти").triggered.connect(QApplication.quit)
            self.setContextMenu(menu)

    def _set_profile(self, profile: str):
                self.ipc.send_command({"cmd": "set_profile", "profile": profile})
                self.showMessage("NetGuard",
                                 f"Профіль: {profile}",
                                 QSystemTrayIcon.MessageIcon.Information, 2000)

    def _run_healing(self, action: str):
                labels = {
                    "flushdns": "Очищення DNS кешу...",
                    "renew":    "Оновлення IP адреси...",
                    "winsock":  "Скидання Winsock...",
                }
                self.showMessage("NetGuard",
                                 labels.get(action, action),
                                 QSystemTrayIcon.MessageIcon.Information, 2000)
                self.ipc.send_command({"cmd": "run_healing", "action": action})

    def _run_dns_bench(self):
                self.showMessage("NetGuard",
                                 "DNS бенчмарк запущено...",
                                 QSystemTrayIcon.MessageIcon.Information, 2000)
                self.ipc.send_command({"cmd": "dns_bench"})

    def _on_ping(self, ms: float, jitter: float, loss: float):
        if ms < 0 or loss > 10:
            self.setIcon(ICONS["error"])
            self._status_action.setText(f"● Проблеми (loss {loss:.0f}%)")
            self.setToolTip(f"NetGuard — втрата пакетів {loss:.0f}%")
        elif ms > 150 or loss > 2:
            self.setIcon(ICONS["warning"])
            self._status_action.setText(f"● Нестабільно ({ms:.0f} мс)")
            self.setToolTip(f"NetGuard — {ms:.0f} мс")
        else:
            self.setIcon(ICONS["ok"])
            self._status_action.setText(f"● Стабільно ({ms:.0f} мс)")
            self.setToolTip(f"NetGuard — {ms:.0f} мс")

    def _on_alert(self, code: str, value: float):
        self.showMessage("NetGuard", f"Увага: {code} ({value:.1f}%)",
                         QSystemTrayIcon.MessageIcon.Warning, 4000)

    def _on_drop(self):
        self.setIcon(ICONS["error"])
        self.showMessage("NetGuard", "З'єднання перервано!",
                         QSystemTrayIcon.MessageIcon.Critical, 5000)

    def _on_activated(self, reason):
        if reason == QSystemTrayIcon.ActivationReason.DoubleClick:
            self._open_window()

    def _open_window(self):
        try:
            from main_window import MainWindow
            if not hasattr(self, "_window"):
                self._window = MainWindow(self.ipc)
            self._window.show()
            self._window.raise_()
        except Exception as e:
            import traceback
            print(f"[GUI] Window error: {e}")
            traceback.print_exc()

    def _quit(self):
        QApplication.quit()