import sqlite3
import os
from datetime import datetime
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QDateEdit,
    QFileDialog, QMessageBox, QTextEdit
)
from PyQt6.QtCore import QDate, Qt

DB_PATH = os.path.join(
    os.path.dirname(os.path.dirname(__file__)),
    "build", "Desktop_Qt_6_11_0_MSVC2022_64bit-Debug",
    "netguard.db"
)

class ReportBuilder(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._build_ui()

    def _build_ui(self):
        layout = QVBoxLayout(self)

        date_row = QHBoxLayout()
        date_row.addWidget(QLabel("Від:"))
        self.date_from = QDateEdit(QDate.currentDate().addDays(-7))
        self.date_from.setCalendarPopup(True)
        date_row.addWidget(self.date_from)

        date_row.addWidget(QLabel("До:"))
        self.date_to = QDateEdit(QDate.currentDate())
        self.date_to.setCalendarPopup(True)
        date_row.addWidget(self.date_to)
        date_row.addStretch()
        layout.addLayout(date_row)

        btn_row = QHBoxLayout()
        preview_btn = QPushButton("Переглянути статистику")
        preview_btn.clicked.connect(self._load_stats)
        btn_row.addWidget(preview_btn)

        pdf_btn = QPushButton("Згенерувати PDF")
        pdf_btn.clicked.connect(self._generate_pdf)
        btn_row.addWidget(pdf_btn)
        layout.addLayout(btn_row)

        self.preview = QTextEdit()
        self.preview.setReadOnly(True)
        self.preview.setStyleSheet("font-family: monospace; font-size: 12px;")
        layout.addWidget(self.preview)

    def _get_timestamps(self):
        ts_from = self.date_from.date().startOfDay().toMSecsSinceEpoch()
        ts_to   = self.date_to.date().endOfDay().toMSecsSinceEpoch()
        return ts_from, ts_to

    def _find_db(self):
        candidates = [
            DB_PATH,
            os.path.join(os.path.dirname(os.path.dirname(__file__)), "netguard.db"), "netguard.db",
        ]
        for path in candidates:
            if os.path.exists(path):
                return path
        return None

    def _load_stats(self):
        db_path = self._find_db()
        if not db_path:
            self.preview.setText("БД не знайдено.\nПереконайся що C++ сервіс запущений.")
            return

        ts_from, ts_to = self._get_timestamps()
        try:
            conn = sqlite3.connect(db_path)
            cur  = conn.cursor()

            cur.execute("""
                SELECT COUNT(*) FROM connection_drops
                WHERE ts BETWEEN ? AND ?
            """, (ts_from, ts_to))
            drop_count = cur.fetchone()[0]

            cur.execute("""
                SELECT COUNT(*), AVG(latency), MAX(latency),
                       AVG(jitter), AVG(loss)
                FROM ping_log
                WHERE ts BETWEEN ? AND ? AND timeout = 0
            """, (ts_from, ts_to))
            row = cur.fetchone()

            cur.execute("""
                SELECT ts, duration_ms, reason
                FROM connection_drops
                WHERE ts BETWEEN ? AND ?
                ORDER BY ts DESC LIMIT 10
            """, (ts_from, ts_to))
            drops = cur.fetchall()
            conn.close()

            text = []
            text.append("=" * 50)
            text.append("  NetGuard — Звіт про якість з'єднання")
            text.append("=" * 50)
            d_from = self.date_from.date().toString("dd.MM.yyyy")
            d_to   = self.date_to.date().toString("dd.MM.yyyy")
            text.append(f"Період: {d_from} — {d_to}\n")

            text.append("── Статистика ──────────────────────────")
            text.append(f"Всього пінгів:     {row[0] or 0}")
            text.append(f"Середня затримка:  {row[1]:.1f} мс" if row[1] else "Середня затримка:  —")
            text.append(f"Максимальна:       {row[2]:.1f} мс" if row[2] else "Максимальна:       —")
            text.append(f"Середній jitter:   {row[3]:.1f} мс" if row[3] else "Середній jitter:   —")
            text.append(f"Середня втрата:    {row[4]:.2f}%"   if row[4] else "Середня втрата:    —")
            text.append(f"Обривів:           {drop_count}\n")

            if drops:
                text.append("── Останні обриви ──────────────────────")
                for d in drops:
                    ts_str = datetime.fromtimestamp(
                        d[0] / 1000).strftime("%d.%m.%Y %H:%M:%S")
                    dur = f"{d[1]} мс" if d[1] else "—"
                    text.append(f"{ts_str}  тривалість: {dur}  причина: {d[2]}")

            self.preview.setText("\n".join(text))

        except Exception as e:
            self.preview.setText(f"Помилка читання БД:\n{e}")

    def _generate_pdf(self):
        db_path = self._find_db()
        if not db_path:
            QMessageBox.warning(self, "Помилка", "БД не знайдено.")
            return

        path, _ = QFileDialog.getSaveFileName(
            self, "Зберегти звіт",
            f"netguard_report_{datetime.now():%Y%m%d}.pdf",
            "PDF Files (*.pdf)"
        )
        if not path:
            return

        try:
            from reportlab.lib.pagesizes import A4
            from reportlab.lib.styles import getSampleStyleSheet
            from reportlab.lib.units import cm
            from reportlab.platypus import (
                SimpleDocTemplate, Paragraph,
                Spacer, Table, TableStyle
            )
            from reportlab.lib import colors

            doc    = SimpleDocTemplate(path, pagesize=A4,
                                       leftMargin=2*cm, rightMargin=2*cm,
                                       topMargin=2*cm,  bottomMargin=2*cm)
            styles = getSampleStyleSheet()
            story  = []

            d_from = self.date_from.date().toString("dd.MM.yyyy")
            d_to   = self.date_to.date().toString("dd.MM.yyyy")

            story.append(Paragraph(
                "NetGuard — Звіт якості з'єднання", styles["Title"]))
            story.append(Paragraph(
                f"Період: {d_from} — {d_to}", styles["Normal"]))
            story.append(Spacer(1, 0.5*cm))

            ts_from, ts_to = self._get_timestamps()
            conn = sqlite3.connect(db_path)
            cur  = conn.cursor()

            story.append(Paragraph("Статистика пінгу", styles["Heading2"]))
            cur.execute("""
                SELECT ts, latency, jitter, loss
                FROM ping_log
                WHERE ts BETWEEN ? AND ?
                ORDER BY ts DESC LIMIT 50
            """, (ts_from, ts_to))
            rows = cur.fetchall()

            if rows:
                tbl_data = [["Час", "Ping мс", "Jitter мс", "Loss %"]]
                for r in rows:
                    t = datetime.fromtimestamp(
                        r[0]/1000).strftime("%d.%m %H:%M:%S")
                    tbl_data.append([
                        t,
                        f"{r[1]:.1f}" if r[1] else "—",
                        f"{r[2]:.1f}" if r[2] else "—",
                        f"{r[3]:.1f}" if r[3] else "—",
                    ])
                tbl = Table(tbl_data,
                            colWidths=[5*cm, 3.5*cm, 3.5*cm, 3.5*cm])
                tbl.setStyle(TableStyle([
                    ("BACKGROUND", (0,0), (-1,0),
                     colors.HexColor("#2d2d44")),
                    ("TEXTCOLOR",  (0,0), (-1,0), colors.white),
                    ("FONTNAME",   (0,0), (-1,0), "Helvetica-Bold"),
                    ("FONTSIZE",   (0,0), (-1,-1), 9),
                    ("ROWBACKGROUNDS", (0,1), (-1,-1),
                     [colors.white, colors.HexColor("#f0f0f8")]),
                    ("GRID", (0,0), (-1,-1), 0.5,
                     colors.HexColor("#cccccc")),
                ]))
                story.append(tbl)
            else:
                story.append(Paragraph(
                    "Даних за цей період немає.", styles["Normal"]))


            story.append(Spacer(1, 0.5*cm))
            story.append(Paragraph("Обриви з'єднання", styles["Heading2"]))
            cur.execute("""
                SELECT ts, duration_ms, reason
                FROM connection_drops
                WHERE ts BETWEEN ? AND ?
                ORDER BY ts
            """, (ts_from, ts_to))
            drops = cur.fetchall()
            conn.close()

            if drops:
                drop_data = [["Час обриву", "Тривалість", "Причина"]]
                for d in drops:
                    t = datetime.fromtimestamp(
                        d[0]/1000).strftime("%d.%m.%Y %H:%M:%S")
                    drop_data.append([
                        t,
                        f"{d[1]} мс" if d[1] else "—",
                        d[2] or "—"
                    ])
                tbl2 = Table(drop_data,
                             colWidths=[5.5*cm, 4*cm, 6*cm])
                tbl2.setStyle(TableStyle([
                    ("BACKGROUND", (0,0), (-1,0),
                     colors.HexColor("#c0392b")),
                    ("TEXTCOLOR",  (0,0), (-1,0), colors.white),
                    ("FONTNAME",   (0,0), (-1,0), "Helvetica-Bold"),
                    ("FONTSIZE",   (0,0), (-1,-1), 9),
                    ("ROWBACKGROUNDS", (0,1), (-1,-1),
                     [colors.white, colors.HexColor("#fdf0f0")]),
                    ("GRID", (0,0), (-1,-1), 0.5,
                     colors.HexColor("#cccccc")),
                ]))
                story.append(tbl2)
            else:
                story.append(Paragraph(
                    "Обривів не зафіксовано.", styles["Normal"]))

            story.append(Spacer(1, 1*cm))
            story.append(Paragraph(
                f"Звіт згенеровано: "
                f"{datetime.now().strftime('%d.%m.%Y %H:%M')} | NetGuard v1.0",
                styles["Normal"]
            ))
            doc.build(story)
            QMessageBox.information(self, "Готово",
                                    f"Звіт збережено:\n{path}")

        except ImportError:
            QMessageBox.warning(self, "Помилка",
                "Встановіть reportlab:\npip install reportlab")
        except Exception as e:
            QMessageBox.critical(self, "Помилка генерації", str(e))