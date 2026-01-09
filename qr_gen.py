from pathlib import Path
import os
import sys
import qrcode

INPUT_FILE = Path("here is location of txt file that contain qr name")
OUTPUT_DIR = Path("here is output folder location")

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

INVALID = set('<>:"/\\|?*')

def safe_filename(text: str) -> str:
    s = ''.join('_' if c in INVALID else c for c in text).strip()[:150]
    return s or "unnamed"


def main():
    if not INPUT_FILE.exists():
        print(f"Input file not found: {INPUT_FILE}", file=sys.stderr)
        sys.exit(1)

    items = [line.strip() for line in INPUT_FILE.read_text(encoding="utf-8").splitlines()]
    items = [i for i in items if i]

    created = 0
    errors = 0

    for data in items:
        try:
            qr = qrcode.QRCode(version=1, error_correction=qrcode.constants.ERROR_CORRECT_L, box_size=10, border=4)
            qr.add_data(data)
            qr.make(fit=True)
            img = qr.make_image(fill_color="black", back_color="white")
            img.save(OUTPUT_DIR / f"{safe_filename(data)}.png")
            created += 1
        except Exception as e:
            errors += 1
            print(f"Failed: {data} -> {e}", file=sys.stderr)

    print(f"Output: {OUTPUT_DIR}")
    print(f"Total: {len(items)} | Created: {created} | Errors: {errors}")


if __name__ == "__main__":
    main()
