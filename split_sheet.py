from PIL import Image
import os

# ---- CHANGE THESE ----
sheet_path = r"D:\BloodOfLegends\BloodOfLegends\assets\images\menu_char_sheet.png"
out_dir    = r"D:\BloodOfLegends\BloodOfLegends\assets\images\menu_char"
rows = 2
cols = 6
prefix = "swing_"   # output names: swing_0.png ... swing_11.png

os.makedirs(out_dir, exist_ok=True)

img = Image.open(sheet_path).convert("RGBA")
W, H = img.size

frame_w = W // cols
frame_h = H // rows

idx = 0
for r in range(rows):
    for c in range(cols):
        left   = c * frame_w
        top    = r * frame_h
        right  = left + frame_w
        bottom = top + frame_h

        frame = img.crop((left, top, right, bottom))
        frame.save(os.path.join(out_dir, f"{prefix}{idx}.png"))
        idx += 1

print("Done! Saved", idx, "frames to:", out_dir)
