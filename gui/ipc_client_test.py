import win32pipe
import win32file
import json
import time

PIPE_NAME = r"\\.\pipe\NetGuardPipe"

def main():
    print("Connecting to C++ engine...")

    while True:
        try:
            win32pipe.WaitNamedPipe(PIPE_NAME, 2000)
            break
        except Exception:
            print("[IPC] Waiting for engine to start...")
            time.sleep(1)

    try:
        pipe = win32file.CreateFile(
            PIPE_NAME,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0, None,
            win32file.OPEN_EXISTING,
            0, None
        )
        print("[IPC] Connected!")
    except Exception as e:
        print(f"[IPC] Cannot connect: {e}")
        return

    buffer = ""
    print("Waiting for ping data...\n")
    while True:
        try:
            _, data = win32file.ReadFile(pipe, 4096)
            buffer += data.decode("utf-8")
            while "\n" in buffer:
                line, buffer = buffer.split("\n", 1)
                if line.strip():
                    try:
                        msg = json.loads(line.strip())
                        handle_message(msg)
                    except json.JSONDecodeError:
                        pass
        except KeyboardInterrupt:
            print("\n[IPC] Stopped")
            break
        except Exception as e:
            print(f"[IPC] Error: {e}")
            break

    win32file.CloseHandle(pipe)

def handle_message(msg: dict):
    t = msg.get("type")
    if t == "ping":
        ms   = msg.get("ms", -1)
        loss = msg.get("loss", 0)
        jitter = msg.get("jitter", 0)
        if ms < 0:
            print(f"[Monitor] TIMEOUT  loss={loss:.1f}%")
        else:
            print(f"[Monitor] Ping={ms:.1f}ms  jitter={jitter:.1f}ms  loss={loss:.1f}%")
    elif t == "alert":
        print(f"[ALERT] {msg.get('code')} = {msg.get('value')}")
    elif t == "drop":
        print(f"[DROP] Connection dropped at {msg.get('ts')}")
    elif t == "healing":
        print(f"[HEALING] {msg.get('action')} -> {msg.get('result')}")

if __name__ == "__main__":
    main()