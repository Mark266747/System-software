import sys
import threading
import json
import time
import win32pipe
import win32file
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import QObject, pyqtSignal

class IpcClient(QObject):
    ping_received  = pyqtSignal(float, float, float)
    alert_received = pyqtSignal(str, float)
    drop_received  = pyqtSignal()

    def __init__(self):
        super().__init__()
        self._running = False

    def start(self):
        self._running = True
        thread = threading.Thread(target=self._read_loop, daemon=True)
        thread.start()

    def _read_loop(self):
            PIPE_NAME = r"\\.\pipe\NetGuardPipe"
            _last_error = ""
            while self._running:
                try:
                    win32pipe.WaitNamedPipe(PIPE_NAME, 2000)
                    pipe = win32file.CreateFile(
                        PIPE_NAME,
                        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                        0, None,
                        win32file.OPEN_EXISTING,
                        0, None
                    )
                    print("[IPC] Connected")
                    _last_error = ""
                    buffer = ""
                    while self._running:
                        _, data = win32file.ReadFile(pipe, 4096)
                        buffer += data.decode("utf-8")
                        while "\n" in buffer:
                            line, buffer = buffer.split("\n", 1)
                            if line.strip():
                                try:
                                    msg = json.loads(line.strip())
                                    self._dispatch(msg)
                                except json.JSONDecodeError:
                                    pass
                except Exception as e:
                    err_str = str(e)
                    if err_str != _last_error:
                        print(f"[IPC] Waiting for engine...")
                        _last_error = err_str
                    time.sleep(2)

    def _dispatch(self, msg: dict):
        t = msg.get("type")
        if t == "ping":
            self.ping_received.emit(
                msg.get("ms", -1),
                msg.get("jitter", 0),
                msg.get("loss", 0)
            )
        elif t == "alert":
            self.alert_received.emit(msg.get("code",""), msg.get("value", 0))
        elif t == "drop":
            self.drop_received.emit()

    def stop(self):
        self._running = False

    def send_command(self, cmd: dict):
            try:
                PIPE_NAME = r"\\.\pipe\NetGuardPipe"
                pipe = win32file.CreateFile(
                    PIPE_NAME,
                    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                    0, None,
                    win32file.OPEN_EXISTING,
                    0, None
                )
                data = (json.dumps(cmd) + "\n").encode("utf-8")
                win32file.WriteFile(pipe, data)
                win32file.CloseHandle(pipe)
                print(f"[IPC] Sent: {cmd}")
            except Exception as e:
                print(f"[IPC] Send error: {e}")

def main():
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)

    ipc = IpcClient()

    from tray import SystemTray
    tray = SystemTray(ipc)
    tray.show()

    ipc.start()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()