import os
import resvg_py

def generate_schematic():
    width = 1600
    height = 1050
    
    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}">')
    svg.append('''<defs>
<style>
  .sheet-bg { fill: #FFFDF9; }
  .grid-border { stroke: #000000; stroke-width: 1.5; fill: none; }
  .grid-inner { stroke: #000000; stroke-width: 0.8; fill: none; }
  .grid-text { font-family: "Courier New", monospace; font-size: 11px; font-weight: bold; fill: #000000; text-anchor: middle; dominant-baseline: middle; }
  .title-text { font-family: "Times New Roman", serif; font-size: 24px; font-weight: bold; fill: #000080; }
  .block-title { font-family: "Courier New", monospace; font-size: 16px; font-weight: bold; fill: #000080; }
  .comp-box { fill: #FFFFE6; stroke: #000000; stroke-width: 1.2; }
  .comp-name { font-family: Arial, Helvetica, sans-serif; font-size: 13px; font-weight: bold; fill: #000000; text-anchor: middle; dominant-baseline: middle; }
  .comp-ref { font-family: Arial, Helvetica, sans-serif; font-size: 11px; font-weight: bold; fill: #000080; }
  .comp-val { font-family: Arial, Helvetica, sans-serif; font-size: 9.5px; font-weight: bold; fill: #000080; }
  .pin-name { font-family: Arial, Helvetica, sans-serif; font-size: 8px; font-weight: bold; fill: #000000; }
  .pin-num { font-family: Arial, Helvetica, sans-serif; font-size: 7.5px; fill: #000080; }
  .wire { stroke: #000080; stroke-width: 1.2; fill: none; stroke-linecap: round; stroke-linejoin: round; }
  .wire-thick { stroke: #000080; stroke-width: 1.5; fill: none; }
  .dot { fill: #000080; }
  .net-label { font-family: Arial, Helvetica, sans-serif; font-size: 9px; font-weight: bold; fill: #800000; }
  .pwr-text { font-family: Arial, Helvetica, sans-serif; font-size: 9px; font-weight: bold; fill: #800000; text-anchor: middle; }
  .pwr-sym { stroke: #800000; stroke-width: 1.2; fill: none; }
  .gnd-sym { stroke: #000080; stroke-width: 1.2; fill: none; }
  .tb-line { stroke: #000000; stroke-width: 0.8; }
  .tb-label { font-family: Arial, Helvetica, sans-serif; font-size: 8px; fill: #555555; }
  .tb-val { font-family: Arial, Helvetica, sans-serif; font-size: 9px; font-weight: bold; fill: #000000; }
</style>
</defs>''')

    # Background
    svg.append(f'<rect width="{width}" height="{height}" class="sheet-bg" />')

    # Outer and Inner Grid Border
    margin = 30
    svg.append(f'<rect x="{margin}" y="{margin}" width="{width - 2*margin}" height="{height - 2*margin}" class="grid-border" />')
    svg.append(f'<rect x="{margin + 15}" y="{margin + 15}" width="{width - 2*(margin + 15)}" height="{height - 2*(margin + 15)}" class="grid-inner" />')

    # Grid markings (1-8 horizontal, A-D vertical)
    w_inner = width - 2*(margin + 15)
    h_inner = height - 2*(margin + 15)
    dx = w_inner / 8
    dy = h_inner / 4

    for i in range(8):
        x = margin + 15 + i * dx + dx / 2
        svg.append(f'<text x="{x}" y="{margin + 7.5}" class="grid-text">{i+1}</text>')
        svg.append(f'<text x="{x}" y="{height - margin - 7.5}" class="grid-text">{i+1}</text>')
        if i > 0:
            svg.append(f'<line x1="{margin + 15 + i*dx}" y1="{margin}" x2="{margin + 15 + i*dx}" y2="{margin + 15}" stroke="#000" stroke-width="0.8"/>')
            svg.append(f'<line x1="{margin + 15 + i*dx}" y1="{height - margin - 15}" x2="{margin + 15 + i*dx}" y2="{height - margin}" stroke="#000" stroke-width="0.8"/>')

    letters = ['A', 'B', 'C', 'D']
    for j in range(4):
        y = margin + 15 + j * dy + dy / 2
        svg.append(f'<text x="{margin + 7.5}" y="{y}" class="grid-text">{letters[j]}</text>')
        svg.append(f'<text x="{width - margin - 7.5}" y="{y}" class="grid-text">{letters[j]}</text>')
        if j > 0:
            svg.append(f'<line x1="{margin}" y1="{margin + 15 + j*dy}" x2="{margin + 15}" y2="{margin + 15 + j*dy}" stroke="#000" stroke-width="0.8"/>')
            svg.append(f'<line x1="{width - margin - 15}" y1="{margin + 15 + j*dy}" x2="{width - margin}" y2="{margin + 15 + j*dy}" stroke="#000" stroke-width="0.8"/>')

    # Title Block (Bottom Right)
    tb_w = 280
    tb_h = 90
    tb_x = width - margin - 15 - tb_w
    tb_y = height - margin - 15 - tb_h
    svg.append(f'<rect x="{tb_x}" y="{tb_y}" width="{tb_w}" height="{tb_h}" fill="#FFF" stroke="#000" stroke-width="1.2" />')
    svg.append(f'<line x1="{tb_x}" y1="{tb_y + 35}" x2="{tb_x + tb_w}" y2="{tb_y + 35}" class="tb-line" />')
    svg.append(f'<line x1="{tb_x}" y1="{tb_y + 60}" x2="{tb_x + tb_w}" y2="{tb_y + 60}" class="tb-line" />')
    svg.append(f'<line x1="{tb_x + 50}" y1="{tb_y + 35}" x2="{tb_x + 50}" y2="{tb_y + 60}" class="tb-line" />')
    svg.append(f'<line x1="{tb_x + 190}" y1="{tb_y + 35}" x2="{tb_x + 190}" y2="{tb_y + 60}" class="tb-line" />')
    svg.append(f'<line x1="{tb_x + 140}" y1="{tb_y + 60}" x2="{tb_x + 140}" y2="{tb_y + tb_h}" class="tb-line" />')

    svg.append(f'<text x="{tb_x + 5}" y="{tb_y + 12}" class="tb-label">Title</text>')
    svg.append(f'<text x="{tb_x + 35}" y="{tb_y + 24}" class="tb-val" font-size="14px">ESP32 Board</text>')

    svg.append(f'<text x="{tb_x + 5}" y="{tb_y + 44}" class="tb-label">Size</text>')
    svg.append(f'<text x="{tb_x + 18}" y="{tb_y + 54}" class="tb-val">A4</text>')

    svg.append(f'<text x="{tb_x + 55}" y="{tb_y + 44}" class="tb-label">Number</text>')
    svg.append(f'<text x="{tb_x + 85}" y="{tb_y + 54}" class="tb-val">ESP32-KEY-V1.0</text>')

    svg.append(f'<text x="{tb_x + 195}" y="{tb_y + 44}" class="tb-label">Revision</text>')
    svg.append(f'<text x="{tb_x + 230}" y="{tb_y + 54}" class="tb-val">1.0</text>')

    svg.append(f'<text x="{tb_x + 5}" y="{tb_y + 70}" class="tb-label">Date: 5/15/2020</text>')
    svg.append(f'<text x="{tb_x + 5}" y="{tb_y + 82}" class="tb-label">File: Schematic_esp32.SchDoc</text>')
    svg.append(f'<text x="{tb_x + 145}" y="{tb_y + 70}" class="tb-label">Sheet 1 of 1</text>')
    svg.append(f'<text x="{tb_x + 145}" y="{tb_y + 82}" class="tb-label">Drawn By: HW Team</text>')

    # Main Sheet Title
    svg.append(f'<text x="{width / 2}" y="85" class="title-text" text-anchor="middle">ESP32 Board</text>')

    # --- HELPER FUNCTIONS ---
    def add_pwr_arrow_up(x, y, label="3V3"):
        # Triangle apex at (x, y), base at y+8
        svg.append(f'<polygon points="{x},{y} {x-5},{y+8} {x+5},{y+8}" fill="#800000" stroke="#800000" stroke-width="0.8" />')
        svg.append(f'<text x="{x}" y="{y-4}" class="pwr-text" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="middle">{label}</text>')

    def add_pwr_arrow_down(x, y, label="5V"):
        # Triangle apex at (x, y), base at y-8
        svg.append(f'<polygon points="{x},{y} {x-5},{y-8} {x+5},{y-8}" fill="#800000" stroke="#800000" stroke-width="0.8" />')
        svg.append(f'<text x="{x}" y="{y+14}" class="pwr-text" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="middle">{label}</text>')

    def add_pwr_arrow_right(x, y, label="3V3"):
        # Triangle apex at (x, y), base at x-8
        svg.append(f'<polygon points="{x},{y} {x-8},{y-5} {x-8},{y+5}" fill="#800000" stroke="#800000" stroke-width="0.8" />')
        svg.append(f'<text x="{x+14}" y="{y+3.5}" class="pwr-text" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="start">{label}</text>')

    def add_gnd(x, y):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x}" y2="{y+8}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-8}" y1="{y+8}" x2="{x+8}" y2="{y+8}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-5}" y1="{y+12}" x2="{x+5}" y2="{y+12}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-2}" y1="{y+16}" x2="{x+2}" y2="{y+16}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x}" y="{y+26}" font-family="Arial, Helvetica, sans-serif" font-size="8.5px" font-weight="bold" fill="#000080" text-anchor="middle">GND</text>')

    def add_gnd_top_earth(x, y):
        # Earth ground pointing upwards on top of a wire at (x, y) with GND text ABOVE it
        svg.append(f'<line x1="{x-8}" y1="{y}" x2="{x+8}" y2="{y}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-5}" y1="{y-4}" x2="{x+5}" y2="{y-4}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-2}" y1="{y-8}" x2="{x+2}" y2="{y-8}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x}" y="{y-13}" font-family="Arial, Helvetica, sans-serif" font-size="8.5px" font-weight="bold" fill="#000080" text-anchor="middle">GND</text>')

    def add_gnd_horiz_left(x, y):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x-6}" y2="{y}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-6}" y1="{y-7}" x2="{x-6}" y2="{y+7}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-10}" y1="{y-4}" x2="{x-10}" y2="{y+4}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-14}" y1="{y-2}" x2="{x-14}" y2="{y+2}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x-18}" y="{y+3}" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080" text-anchor="end">GND</text>')

    def add_gnd_horiz_right(x, y):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x+6}" y2="{y}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x+6}" y1="{y-7}" x2="{x+6}" y2="{y+7}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x+10}" y1="{y-4}" x2="{x+10}" y2="{y+4}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x+14}" y1="{y-2}" x2="{x+14}" y2="{y+2}" class="gnd-sym" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x+18}" y="{y+3}" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080" text-anchor="start">GND</text>')

    def add_resistor_v(x, y, ref, val):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x}" y2="{y+6}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        pts = f"{x},{y+6} {x+5},{y+9} {x-5},{y+15} {x+5},{y+21} {x-5},{y+27} {x+5},{y+33} {x},{y+36}"
        svg.append(f'<polyline points="{pts}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x}" y1="{y+36}" x2="{x}" y2="{y+42}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x-8}" y="{y+18}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="end">{ref}</text>')
        svg.append(f'<text x="{x-8}" y="{y+30}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="end">{val}</text>')

    def add_resistor_h(x, y, ref, val):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x+6}" y2="{y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        pts = f"{x+6},{y} {x+9},{y-5} {x+15},{y+5} {x+21},{y-5} {x+27},{y+5} {x+33},{y-5} {x+36},{y}"
        svg.append(f'<polyline points="{pts}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x+36}" y1="{y}" x2="{x+42}" y2="{y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x+21}" y="{y-8}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="middle">{ref}</text>')
        svg.append(f'<text x="{x+21}" y="{y+16}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="middle">{val}</text>')

    def add_capacitor_v(x, y, ref, val):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x}" y2="{y+8}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x-7}" y1="{y+8}" x2="{x+7}" y2="{y+8}" class="wire-thick" stroke="#000080" stroke-width="1.5" fill="none" />')
        svg.append(f'<line x1="{x-7}" y1="{y+12}" x2="{x+7}" y2="{y+12}" class="wire-thick" stroke="#000080" stroke-width="1.5" fill="none" />')
        svg.append(f'<line x1="{x}" y1="{y+12}" x2="{x}" y2="{y+20}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x+9}" y="{y+8}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="start">{ref}</text>')
        svg.append(f'<text x="{x+9}" y="{y+18}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="start">{val}</text>')

    def add_capacitor_h(x, y, ref, val):
        svg.append(f'<line x1="{x}" y1="{y}" x2="{x+8}" y2="{y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<line x1="{x+8}" y1="{y-7}" x2="{x+8}" y2="{y+7}" class="wire-thick" stroke="#000080" stroke-width="1.5" fill="none" />')
        svg.append(f'<line x1="{x+12}" y1="{y-7}" x2="{x+12}" y2="{y+7}" class="wire-thick" stroke="#000080" stroke-width="1.5" fill="none" />')
        svg.append(f'<line x1="{x+12}" y1="{y}" x2="{x+20}" y2="{y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        svg.append(f'<text x="{x+10}" y="{y-10}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="middle">{ref}</text>')
        svg.append(f'<text x="{x+10}" y="{y+17}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="middle">{val}</text>')

    def add_chevron_left_input(x_tip, y, label):
        # Red chevron pointing right towards x_tip, text to the left. Wire only from x_tip.
        svg.append(f'<polygon points="{x_tip-15},{y-5} {x_tip-3},{y-5} {x_tip},{y} {x_tip-3},{y+5} {x_tip-15},{y+5} {x_tip-10},{y}" fill="#FFE6E6" stroke="#800000" stroke-width="0.9" />')
        svg.append(f'<text x="{x_tip-20}" y="{y+3}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="end">{label}</text>')

    def add_chevron_right_output(x_base, y, label):
        # Red chevron pointing right from x_base, text to the right. Wire ends at x_base.
        svg.append(f'<polygon points="{x_base},{y-5} {x_base+12},{y-5} {x_base+15},{y} {x_base+12},{y+5} {x_base},{y+5} {x_base+5},{y}" fill="#FFE6E6" stroke="#800000" stroke-width="0.9" />')
        svg.append(f'<text x="{x_base+20}" y="{y+3}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="start">{label}</text>')

    def add_net_label_horiz(x, y, label, align="left", text_pos="side"):
        if text_pos == "above":
            svg.append(f'<text x="{x}" y="{y-5}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="{align}">{label}</text>')
            if align == "left":
                svg.append(f'<line x1="{x}" y1="{y-3}" x2="{x+len(label)*6}" y2="{y-3}" stroke="#800000" stroke-width="0.8" fill="none" />')
            elif align == "right":
                svg.append(f'<line x1="{x}" y1="{y-3}" x2="{x-len(label)*6}" y2="{y-3}" stroke="#800000" stroke-width="0.8" fill="none" />')
            else:
                svg.append(f'<line x1="{x-len(label)*3}" y1="{y-3}" x2="{x+len(label)*3}" y2="{y-3}" stroke="#800000" stroke-width="0.8" fill="none" />')
        else:
            if align == "left":
                svg.append(f'<text x="{x-4}" y="{y+3}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="end">{label}</text>')
                svg.append(f'<line x1="{x-4}" y1="{y+4.5}" x2="{x-4-len(label)*5.5}" y2="{y+4.5}" stroke="#800000" stroke-width="0.8" fill="none" />')
            elif align == "right":
                svg.append(f'<text x="{x+4}" y="{y+3}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" text-anchor="start">{label}</text>')
                svg.append(f'<line x1="{x+4}" y1="{y+4.5}" x2="{x+4+len(label)*5.5}" y2="{y+4.5}" stroke="#800000" stroke-width="0.8" fill="none" />')

    def add_net_label_vert(x, y, label, direction="up"):
        if direction == "up":
            svg.append(f'<text x="{x+3}" y="{y-4}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" transform="rotate(-90 {x+3},{y-4})" text-anchor="start">{label}</text>')
            svg.append(f'<line x1="{x-2}" y1="{y-4}" x2="{x-2}" y2="{y-4-len(label)*5.5}" stroke="#800000" stroke-width="0.8" fill="none" />')
        elif direction == "down":
            svg.append(f'<text x="{x+3}" y="{y+4}" class="net-label" font-family="Arial, Helvetica, sans-serif" font-size="9px" font-weight="bold" fill="#800000" transform="rotate(90 {x+3},{y+4})" text-anchor="start">{label}</text>')
            svg.append(f'<line x1="{x-2}" y1="{y+4}" x2="{x-2}" y2="{y+4+len(label)*5.5}" stroke="#800000" stroke-width="0.8" fill="none" />')

    def add_dot(x, y):
        svg.append(f'<circle cx="{x}" cy="{y}" r="2.5" class="dot" fill="#000080" />')

    # =========================================================================
    # BLOCK 1: POWER (Bottom Left: X=120..480, Y=680..950)
    # =========================================================================
    svg.append('<text x="280" y="690" class="block-title" font-family="Courier New, monospace" font-size="16px" font-weight="bold" fill="#000080" text-anchor="middle">POWER</text>')

    # USB1 Connector (Generous width 65, height 75)
    u1_x = 210
    u1_y = 780
    u1_w = 65
    u1_h = 75
    svg.append(f'<rect x="{u1_x}" y="{u1_y}" width="{u1_w}" height="{u1_h}" class="comp-box" fill="#FFFFE6" stroke="#000000" stroke-width="1.2" />')
    svg.append(f'<text x="{u1_x + u1_w/2}" y="{u1_y + u1_h + 14}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="middle">USB1</text>')

    # USB1 Pins:
    # Pin 4 (VCC):
    svg.append(f'<text x="{u1_x+8}" y="796" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">VCC</text>')
    svg.append(f'<text x="{u1_x-4}" y="790" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">4</text>')
    svg.append(f'<line x1="{u1_x-15}" y1="792" x2="{u1_x}" y2="792" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # Pin 3 (D-):
    svg.append(f'<text x="{u1_x+8}" y="816" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">D-</text>')
    svg.append(f'<text x="{u1_x-4}" y="810" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">3</text>')
    svg.append(f'<line x1="{u1_x-40}" y1="812" x2="{u1_x}" y2="812" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_chevron_left_input(u1_x-40, 812, "USB_N")

    # Pin 2 (D+):
    svg.append(f'<text x="{u1_x+8}" y="836" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">D+</text>')
    svg.append(f'<text x="{u1_x-4}" y="830" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">2</text>')
    svg.append(f'<line x1="{u1_x-40}" y1="832" x2="{u1_x}" y2="832" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_chevron_left_input(u1_x-40, 832, "USB_P")

    # Pin 1 (GND):
    svg.append(f'<text x="{u1_x+8}" y="851" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">GND</text>')
    svg.append(f'<text x="{u1_x-4}" y="845" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">1</text>')
    svg.append(f'<line x1="{u1_x-15}" y1="847" x2="{u1_x}" y2="847" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="{u1_x-15}" y1="847" x2="{u1_x-15}" y2="872" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd(u1_x-15, 872)

    # VCC (Pin 4) routing to 5V power bus and U2
    svg.append(f'<line x1="{u1_x-15}" y1="792" x2="170" y2="792" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="170" y1="792" x2="170" y2="750" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(170, 750)
    
    # 5V_USB Power arrow above Pin 4
    svg.append(f'<line x1="170" y1="750" x2="170" y2="728" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(170, 720, "5V_USB")

    # Horizontal 5V line from x=170 to x=390 (Right angle corner!)
    svg.append(f'<line x1="170" y1="750" x2="390" y2="750" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(390, 750)
    
    # 5V Power arrow placed EXACTLY at the 90-degree corner at x=390
    svg.append(f'<line x1="390" y1="750" x2="390" y2="728" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(390, 720, "5V")

    # Vertical line down from x=390 to feed U2 and C10
    svg.append(f'<line x1="390" y1="750" x2="390" y2="870" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # C10 (10uF) at x=390, y=870
    add_capacitor_v(390, 870, "C10", "10uF")
    add_gnd(390, 890)

    # U2 (ME6211C33M5G SOT-23-5)
    u2_x = 440
    u2_y = 770
    u2_w = 75
    u2_h = 65
    svg.append(f'<rect x="{u2_x}" y="{u2_y}" width="{u2_w}" height="{u2_h}" class="comp-box" fill="#FFFFE6" stroke="#000000" stroke-width="1.2" />')
    svg.append(f'<text x="{u2_x + 8}" y="{u2_y - 6}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080">U2</text>')
    svg.append(f'<text x="{u2_x + u2_w/2}" y="{u2_y + u2_h + 14}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="middle">ME6211C33M5G</text>')

    # Pin 1: VIN (top-left, y=785)
    svg.append(f'<text x="{u2_x+5}" y="788.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">VIN</text>')
    svg.append(f'<text x="{u2_x-4}" y="781" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">1</text>')
    svg.append(f'<line x1="390" y1="785" x2="{u2_x}" y2="785" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(390, 785)

    # Pin 2: VSS (mid-left, y=802.5) -> GND
    svg.append(f'<text x="{u2_x+5}" y="806" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">VSS</text>')
    svg.append(f'<text x="{u2_x-4}" y="798.5" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">2</text>')
    svg.append(f'<line x1="{u2_x-15}" y1="802.5" x2="{u2_x}" y2="802.5" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd_horiz_left(u2_x-15, 802.5)

    # Pin 3: CE (bottom-left, y=820) -> connected directly to 5V line at x=390
    svg.append(f'<text x="{u2_x+5}" y="823.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">CE</text>')
    svg.append(f'<text x="{u2_x-4}" y="816" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">3</text>')
    svg.append(f'<line x1="390" y1="820" x2="{u2_x}" y2="820" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(390, 820)

    # Pin 4: NC (bottom-right, y=820)
    svg.append(f'<text x="{u2_x+u2_w-5}" y="823.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">NC</text>')
    svg.append(f'<text x="{u2_x+u2_w+4}" y="816" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">4</text>')
    svg.append(f'<line x1="{u2_x+u2_w}" y1="820" x2="{u2_x+u2_w+15}" y2="820" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # Pin 5: VOUT (top-right, y=785) -> 3V3 and C9
    svg.append(f'<text x="{u2_x+u2_w-5}" y="788.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">VOUT</text>')
    svg.append(f'<text x="{u2_x+u2_w+4}" y="781" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">5</text>')
    svg.append(f'<line x1="{u2_x+u2_w}" y1="785" x2="{u2_x+u2_w+35}" y2="785" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    
    # 3V3 arrow above VOUT
    vout_x = u2_x + u2_w + 35
    add_dot(vout_x, 785)
    svg.append(f'<line x1="{vout_x}" y1="{785}" x2="{vout_x}" y2="{748}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(vout_x, 740, "3V3")

    # C9 (22uF) below VOUT
    svg.append(f'<line x1="{vout_x}" y1="{785}" x2="{vout_x}" y2="{800}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_capacitor_v(vout_x, 800, "C9", "22uF")
    add_gnd(vout_x, 820)

    # =========================================================================
    # BLOCK 2: PROGRAM (Bottom Middle: X=630..815, Y=730..960)
    # =========================================================================
    prog_x = 630
    prog_y = 730
    prog_w = 185
    prog_h = 230
    svg.append(f'<rect x="{prog_x}" y="{prog_y}" width="{prog_w}" height="{prog_h}" fill="none" stroke="#800000" stroke-width="1.5" />')
    svg.append(f'<polygon points="{prog_x},{prog_y} {prog_x+10},{prog_y} {prog_x},{prog_y+10}" fill="#800000" />')
    svg.append(f'<text x="{prog_x + prog_w/2}" y="{prog_y + 26}" class="block-title" font-family="Courier New, monospace" font-size="16px" font-weight="bold" fill="#000080" text-anchor="middle">PROGRAM</text>')

    # J1 Header Box
    j1_x = prog_x + 25
    j1_y = prog_y + 60
    j1_w = 20
    j1_h = 130
    svg.append(f'<rect x="{j1_x}" y="{j1_y}" width="{j1_w}" height="{j1_h}" fill="#888888" stroke="#333333" stroke-width="1" />')
    svg.append(f'<text x="{j1_x + 10}" y="{j1_y - 6}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="middle">J1</text>')

    # Pin 6: GND
    p6_y = j1_y + 15
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p6_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">6</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p6_y}" x2="{j1_x + j1_w + 75}" y2="{p6_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd_horiz_right(j1_x + j1_w + 75, p6_y)

    # Pin 5: TXD0
    p5_y = j1_y + 35
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p5_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">5</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p5_y}" x2="{j1_x + j1_w + 60}" y2="{p5_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<text x="{j1_x + j1_w + 20}" y="{p5_y - 3}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080">TXD0</text>')

    # Pin 4: RXD0
    p4_y = j1_y + 55
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p4_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">4</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p4_y}" x2="{j1_x + j1_w + 60}" y2="{p4_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<text x="{j1_x + j1_w + 20}" y="{p4_y - 3}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080">RXD0</text>')

    # Pin 3: GPIO0
    p3_y = j1_y + 75
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p3_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">3</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p3_y}" x2="{j1_x + j1_w + 60}" y2="{p3_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<text x="{j1_x + j1_w + 20}" y="{p3_y - 3}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080">GPIO0</text>')

    # Pin 2: EN
    p2_y = j1_y + 95
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p2_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">2</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p2_y}" x2="{j1_x + j1_w + 60}" y2="{p2_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<text x="{j1_x + j1_w + 20}" y="{p2_y - 3}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000080">EN</text>')

    # Pin 1: 3V3 (Shifted cleanly to the right so text doesn't touch the arrow!)
    p1_y = j1_y + 115
    svg.append(f'<text x="{j1_x + j1_w + 4}" y="{p1_y - 2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080">1</text>')
    svg.append(f'<line x1="{j1_x + j1_w}" y1="{p1_y}" x2="{j1_x + j1_w + 50}" y2="{p1_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_right(j1_x + j1_w + 58, p1_y, "3V3")

    # =========================================================================
    # BLOCK 3: LED (Bottom Middle-Right: X=980, Y=730..950 - next to PROGRAM)
    # =========================================================================
    led_x = 980
    svg.append(f'<text x="{led_x}" y="730" class="block-title" font-family="Courier New, monospace" font-size="16px" font-weight="bold" fill="#000080" text-anchor="middle">LED</text>')

    # 3V3 Power arrow
    svg.append(f'<line x1="{led_x}" y1="{758}" x2="{led_x}" y2="{775}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(led_x, 750, "3V3")

    # R3 (10K)
    add_resistor_v(led_x, 775, "R3", "10K")
    
    # Wire from R3 directly to D3 Anode
    svg.append(f'<line x1="{led_x}" y1="{817}" x2="{led_x}" y2="{840}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # D3 (LED Blue)
    d3_y = 840
    svg.append(f'<polygon points="{led_x-10},{d3_y} {led_x+10},{d3_y} {led_x},{d3_y+12}" fill="#0000FF" stroke="#000080" stroke-width="1" />')
    svg.append(f'<line x1="{led_x-10}" y1="{d3_y+12}" x2="{led_x+10}" y2="{d3_y+12}" stroke="#000080" stroke-width="1.8" />')
    
    # Emission arrows (Bold and clean)
    svg.append(f'<line x1="{led_x+9}" y1="{d3_y+7}" x2="{led_x+18}" y2="{d3_y+16}" stroke="#000080" stroke-width="1.3" />')
    svg.append(f'<polygon points="{led_x+18},{d3_y+16} {led_x+14},{d3_y+13} {led_x+16},{d3_y+11}" fill="#000080" />')
    svg.append(f'<line x1="{led_x+14}" y1="{d3_y+3}" x2="{led_x+23}" y2="{d3_y+12}" stroke="#000080" stroke-width="1.3" />')
    svg.append(f'<polygon points="{led_x+23},{d3_y+12} {led_x+19},{d3_y+9} {led_x+21},{d3_y+7}" fill="#000080" />')

    svg.append(f'<text x="{led_x-14}" y="{d3_y+8}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="end">D3</text>')
    svg.append(f'<text x="{led_x+28}" y="{d3_y+8}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="start">BLUE</text>')

    # Wire from Cathode down and turning right to GPIO10
    svg.append(f'<line x1="{led_x}" y1="{d3_y+12}" x2="{led_x}" y2="885" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="{led_x}" y1="885" x2="{led_x+75}" y2="885" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    # GPIO10 label placed directly with red underline
    add_net_label_horiz(led_x+35, 885, "GPIO10", align="left", text_pos="above")

    # =========================================================================
    # BLOCK 4: AUTO-DOWNLOAD (Middle Right: Shifted down to Y=540..720, X=1280)
    # =========================================================================
    q1_bx = 1280
    q1_by = 540
    q2_bx = 1280
    q2_by = 630

    # DTR and RTS Net Labels on Left (Cleanly placed ABOVE the wire, no line through text)
    svg.append(f'<line x1="1140" y1="{q1_by}" x2="1215" y2="{q1_by}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(1160, q1_by, "DTR", align="left", text_pos="above")

    svg.append(f'<line x1="1140" y1="{q2_by}" x2="1215" y2="{q2_by}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(1160, q2_by, "RTS", align="left", text_pos="above")

    # Wire DTR to R2 (10k)
    add_dot(1215, q1_by)
    add_resistor_h(1215, q1_by, "R2", "10K")
    svg.append(f'<line x1="1257" y1="{q1_by}" x2="{q1_bx}" y2="{q1_by}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # Wire RTS to R4 (10k)
    add_dot(1215, q2_by)
    add_resistor_h(1215, q2_by, "R4", "10K")
    svg.append(f'<line x1="1257" y1="{q2_by}" x2="{q2_bx}" y2="{q2_by}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # Cross connections:
    svg.append(f'<line x1="1215" y1="{q1_by}" x2="1215" y2="580" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1215" y1="580" x2="1305" y2="580" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="580" x2="1305" y2="615" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    svg.append(f'<line x1="1215" y1="{q2_by}" x2="1215" y2="590" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1215" y1="590" x2="1305" y2="590" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="590" x2="1305" y2="555" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')

    # Q1 NPN Transistor (Prominent, Bold Emitter Arrow)
    svg.append(f'<line x1="{q1_bx}" y1="{q1_by-12}" x2="{q1_bx}" y2="{q1_by+12}" stroke="#000080" stroke-width="2.2" />')
    svg.append(f'<line x1="{q1_bx}" y1="{q1_by-6}" x2="1305" y2="{q1_by-18}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="{q1_by-18}" x2="1305" y2="{q1_by-35}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="{q1_by-35}" x2="1355" y2="{q1_by-35}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(1320, q1_by-35, "EN", align="left", text_pos="above")

    # Q1 Emitter (Bottom leg with large distinct arrow pointing down-right)
    svg.append(f'<line x1="{q1_bx}" y1="{q1_by+6}" x2="1305" y2="{q1_by+15}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<polygon points="1304,{q1_by+15} 1292,{q1_by+6} 1296,{q1_by+18}" fill="#000080" stroke="#000080" stroke-width="0.5" />')
    svg.append(f'<text x="1315" y="{q1_by+3}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080">Q1 8050 NPN</text>')

    # Q2 NPN Transistor (Prominent, Bold Emitter Arrow)
    svg.append(f'<line x1="{q2_bx}" y1="{q2_by-12}" x2="{q2_bx}" y2="{q2_by+12}" stroke="#000080" stroke-width="2.2" />')
    # Q2 Emitter (Top leg with large distinct arrow pointing up-right)
    svg.append(f'<line x1="{q2_bx}" y1="{q2_by-6}" x2="1305" y2="{q2_by-15}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<polygon points="1304,{q2_by-15} 1292,{q2_by-6} 1296,{q2_by-18}" fill="#000080" stroke="#000080" stroke-width="0.5" />')

    # Q2 Collector (Bottom leg)
    svg.append(f'<line x1="{q2_bx}" y1="{q2_by+6}" x2="1305" y2="{q2_by+18}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="{q2_by+18}" x2="1305" y2="{q2_by+35}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="1305" y1="{q2_by+35}" x2="1355" y2="{q2_by+35}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(1320, q2_by+35, "GPIO0", align="left", text_pos="above")
    svg.append(f'<text x="1315" y="{q2_by+3}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080">Q2 8050 NPN</text>')

    # =========================================================================
    # BLOCK 5: CH343P (U1, Upper Right: X=1200..1370, Y=200..360)
    # =========================================================================
    ch_x = 1200
    ch_y = 200
    ch_w = 170
    ch_h = 160
    svg.append(f'<rect x="{ch_x}" y="{ch_y}" width="{ch_w}" height="{ch_h}" class="comp-box" fill="#FFFFE6" stroke="#000000" stroke-width="1.2" />')
    svg.append(f'<text x="{ch_x - 15}" y="{ch_y - 8}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080">U1</text>')
    svg.append(f'<text x="{ch_x + ch_w/2}" y="{ch_y + ch_h/2 + 4}" class="comp-name" font-family="Arial, Helvetica, sans-serif" font-size="14px" font-weight="bold" fill="#000000" text-anchor="middle" dominant-baseline="middle">CH343P</text>')

    # Top Pins (12 down to 9): x=1240, 1275, 1310, 1345
    # Pin 12: DTR (x=1240)
    svg.append(f'<text x="1240" y="{ch_y+20}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1240,{ch_y+20})" text-anchor="end">DTR</text>')
    svg.append(f'<text x="1236" y="{ch_y-4}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">12</text>')
    svg.append(f'<line x1="1240" y1="{ch_y-40}" x2="1240" y2="{ch_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_vert(1240, ch_y-40, "DTR", "up")

    # Pin 11: DCD (x=1275)
    svg.append(f'<text x="1275" y="{ch_y+20}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1275,{ch_y+20})" text-anchor="end">DCD</text>')
    svg.append(f'<text x="1271" y="{ch_y-4}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">11</text>')
    svg.append(f'<line x1="1275" y1="{ch_y-40}" x2="1275" y2="{ch_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_vert(1275, ch_y-40, "DCD", "up")

    # Pin 10: ACT# (x=1310)
    svg.append(f'<text x="1310" y="{ch_y+20}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1310,{ch_y+20})" text-anchor="end">ACT#</text>')
    svg.append(f'<text x="1306" y="{ch_y-4}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">10</text>')
    svg.append(f'<line x1="1310" y1="{ch_y-40}" x2="1310" y2="{ch_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_vert(1310, ch_y-40, "ACT#", "up")

    # Pin 9: VBUS (x=1345) -> 5V
    svg.append(f'<text x="1345" y="{ch_y+20}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1345,{ch_y+20})" text-anchor="end">VBUS</text>')
    svg.append(f'<text x="1341" y="{ch_y-4}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">9</text>')
    svg.append(f'<line x1="1345" y1="{ch_y-42}" x2="1345" y2="{ch_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(1345, ch_y-50, "5V")

    # Left Pins (13..17):
    # Pin 13: RTS (y=230)
    svg.append(f'<text x="{ch_x+6}" y="233.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">RTS</text>')
    svg.append(f'<text x="{ch_x-4}" y="226" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">13</text>')
    svg.append(f'<line x1="{ch_x-45}" y1="230" x2="{ch_x}" y2="230" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(ch_x-45, 230, "RTS", "left")

    # Pin 14: DSR (y=255)
    svg.append(f'<text x="{ch_x+6}" y="258.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">DSR</text>')
    svg.append(f'<text x="{ch_x-4}" y="251" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">14</text>')
    svg.append(f'<line x1="{ch_x-45}" y1="255" x2="{ch_x}" y2="255" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(ch_x-45, 255, "DSR", "left")

    # Pin 15: CTS (y=280)
    svg.append(f'<text x="{ch_x+6}" y="283.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">CTS</text>')
    svg.append(f'<text x="{ch_x-4}" y="276" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">15</text>')
    svg.append(f'<line x1="{ch_x-45}" y1="280" x2="{ch_x}" y2="280" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(ch_x-45, 280, "CTS", "left")

    # Pin 16: RI (y=305)
    svg.append(f'<text x="{ch_x+6}" y="308.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">RI</text>')
    svg.append(f'<text x="{ch_x-4}" y="301" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">16</text>')
    svg.append(f'<line x1="{ch_x-45}" y1="305" x2="{ch_x}" y2="305" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(ch_x-45, 305, "RI", "left")

    # Pin 17: G -> GND (y=330)
    svg.append(f'<text x="{ch_x+6}" y="333.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">G</text>')
    svg.append(f'<text x="{ch_x-4}" y="326" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">17</text>')
    svg.append(f'<line x1="{ch_x-15}" y1="330" x2="{ch_x}" y2="330" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd_horiz_left(ch_x-15, 330)

    # Right Pins (8 down to 5):
    # Pin 8: UD- -> USB_N (y=230)
    svg.append(f'<text x="{ch_x+ch_w-6}" y="233.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">UD-</text>')
    svg.append(f'<text x="{ch_x+ch_w+4}" y="226" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">8</text>')
    svg.append(f'<line x1="{ch_x+ch_w}" y1="230" x2="{ch_x+ch_w+25}" y2="230" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_chevron_right_output(ch_x+ch_w+25, 230, "USB_N")

    # Pin 7: UD+ -> USB_P (y=255)
    svg.append(f'<text x="{ch_x+ch_w-6}" y="258.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">UD+</text>')
    svg.append(f'<text x="{ch_x+ch_w+4}" y="251" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">7</text>')
    svg.append(f'<line x1="{ch_x+ch_w}" y1="255" x2="{ch_x+ch_w+25}" y2="255" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_chevron_right_output(ch_x+ch_w+25, 255, "USB_P")

    # Pin 6: V3 -> C4 (0.1uF) to GND (y=280)
    svg.append(f'<text x="{ch_x+ch_w-6}" y="283.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">V3</text>')
    svg.append(f'<text x="{ch_x+ch_w+4}" y="276" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">6</text>')
    svg.append(f'<line x1="{ch_x+ch_w}" y1="280" x2="{ch_x+ch_w+20}" y2="280" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_capacitor_h(ch_x+ch_w+20, 280, "C4", "0.1uF")
    add_gnd_horiz_right(ch_x+ch_w+40, 280)

    # Pin 5: TXD -> TXD0 (y=305)
    svg.append(f'<text x="{ch_x+ch_w-6}" y="308.5" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">TXD</text>')
    svg.append(f'<text x="{ch_x+ch_w+4}" y="301" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">5</text>')
    svg.append(f'<line x1="{ch_x+ch_w}" y1="305" x2="{ch_x+ch_w+50}" y2="305" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(ch_x+ch_w+50, 305, "TXD0", "right")

    # Bottom Pins (1..4): x=1240, 1275, 1310, 1345
    # Pin 1: VIO (x=1240)
    svg.append(f'<text x="1240" y="{ch_y+ch_h-10}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1240,{ch_y+ch_h-10})" text-anchor="start">VIO</text>')
    svg.append(f'<text x="1236" y="{ch_y+ch_h+12}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">1</text>')
    svg.append(f'<line x1="1240" y1="{ch_y+ch_h}" x2="1240" y2="{ch_y+ch_h+30}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(1240, ch_y+ch_h+30)
    # Left to branch
    svg.append(f'<line x1="1240" y1="{ch_y+ch_h+30}" x2="1170" y2="{ch_y+ch_h+30}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(1170, ch_y+ch_h+30)
    # Up to 3V3
    svg.append(f'<line x1="1170" y1="{ch_y+ch_h+30}" x2="1170" y2="{ch_y+ch_h+18}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(1170, ch_y+ch_h+10, "3V3")
    # Down to C5 (1uF) -> GND
    svg.append(f'<line x1="1170" y1="{ch_y+ch_h+30}" x2="1170" y2="{ch_y+ch_h+45}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_capacitor_v(1170, ch_y+ch_h+45, "C5", "1uF")
    add_gnd(1170, ch_y+ch_h+65)

    # Pin 2: GND (x=1275)
    svg.append(f'<text x="1275" y="{ch_y+ch_h-10}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1275,{ch_y+ch_h-10})" text-anchor="start">GND</text>')
    svg.append(f'<text x="1271" y="{ch_y+ch_h+12}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">2</text>')
    svg.append(f'<line x1="1275" y1="{ch_y+ch_h}" x2="1275" y2="{ch_y+ch_h+25}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd(1275, ch_y+ch_h+25)

    # Pin 3: VDD5 (x=1310) -> 5V
    svg.append(f'<text x="1310" y="{ch_y+ch_h-10}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1310,{ch_y+ch_h-10})" text-anchor="start">VDD5</text>')
    svg.append(f'<text x="1306" y="{ch_y+ch_h+12}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">3</text>')
    svg.append(f'<line x1="1310" y1="{ch_y+ch_h}" x2="1310" y2="{ch_y+ch_h+30}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_down(1310, ch_y+ch_h+38, "5V")

    # Pin 4: RXD (x=1345) -> RXD0
    svg.append(f'<text x="1345" y="{ch_y+ch_h-10}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 1345,{ch_y+ch_h-10})" text-anchor="start">RXD</text>')
    svg.append(f'<text x="1341" y="{ch_y+ch_h+12}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">4</text>')
    svg.append(f'<line x1="1345" y1="{ch_y+ch_h}" x2="1345" y2="{ch_y+ch_h+30}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_vert(1345, ch_y+ch_h+30, "RXD0", "down")

    # =========================================================================
    # BLOCK 6: ESP32-PICO-D4 (U3, Upper Left / Center-Left: X=340..740, Y=200..550)
    # =========================================================================
    esp_x = 340
    esp_y = 200
    esp_w = 400
    esp_h = 350
    svg.append(f'<rect x="{esp_x}" y="{esp_y}" width="{esp_w}" height="{esp_h}" class="comp-box" fill="#FFFFE6" stroke="#000000" stroke-width="1.2" />')
    
    # Chip name right in the center of the box!
    svg.append(f'<text x="{esp_x + esp_w/2}" y="{esp_y + esp_h/2}" class="comp-name" font-family="Arial, Helvetica, sans-serif" font-size="15px" font-weight="bold" fill="#000000" text-anchor="middle" dominant-baseline="middle">ESP32-PICO-D4</text>')
    svg.append(f'<text x="{esp_x + esp_w - 15}" y="{esp_y + esp_h + 16}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080">U3</text>')

    # --- ESP32 LEFT PINS (1..12) ---
    left_pins = [
        (1, "VDDA", 225),
        (2, "LNA_IN", 250),
        (3, "VDD3P3", 275),
        (4, "VDD3P3", 300),
        (5, "SENSOR_VP", 330),
        (6, "SENSOR_CAPP", 355),
        (7, "SENSOR_CAPN", 380),
        (8, "SENSOR_VN", 405),
        (9, "CHIP_PU", 435),
        (10, "VDET_1", 465),
        (11, "VDET_2", 490),
        (12, "32K_XP", 515)
    ]

    for pnum, pname, py in left_pins:
        svg.append(f'<text x="{esp_x+8}" y="{py+2.5}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000">{pname}</text>')
        svg.append(f'<text x="{esp_x-4}" y="{py-2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">{pnum}</text>')

    # Pin 1, 3, 4 connect to 3V3 rail at x=295
    # NOTE: Pin 1 and 3 are T-junctions (with dot), Pin 4 is a 90-degree corner bend (NO junction dot!)
    pwr_rail_x = 295
    svg.append(f'<line x1="{pwr_rail_x}" y1="225" x2="{esp_x}" y2="225" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(pwr_rail_x, 225)
    svg.append(f'<line x1="{pwr_rail_x}" y1="275" x2="{esp_x}" y2="275" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(pwr_rail_x, 275)
    svg.append(f'<line x1="{pwr_rail_x}" y1="300" x2="{esp_x}" y2="300" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    # Vertical 3V3 rail from y=148 to y=300
    svg.append(f'<line x1="{pwr_rail_x}" y1="148" x2="{pwr_rail_x}" y2="300" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(pwr_rail_x, 140, "3V3")
    add_dot(pwr_rail_x, 148)

    # Parallel Decoupling Capacitors C6, C1, C2 at y=148
    svg.append(f'<line x1="150" y1="148" x2="{pwr_rail_x}" y2="148" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(160, 148)
    add_dot(200, 148)
    add_dot(240, 148)

    # C6 (1uF) at x=160
    add_capacitor_v(160, 148, "C6", "1uF")
    # C1 (10uF) at x=200
    add_capacitor_v(200, 148, "C1", "10uF")
    # C2 (0.1uF) at x=240
    add_capacitor_v(240, 148, "C2", "0.1uF")

    # GND rail for C6, C1, C2 at y=168
    svg.append(f'<line x1="160" y1="168" x2="240" y2="168" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(200, 168)
    add_gnd(200, 168)

    # Pin 2 (LNA_IN) -> E1 Antenna (Placed at x=225, y=240 - well clear of capacitor GND!)
    ant_x = 225
    ant_y = 240
    svg.append(f'<line x1="{ant_x}" y1="250" x2="{esp_x}" y2="250" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="{ant_x}" y1="250" x2="{ant_x}" y2="{ant_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<polygon points="{ant_x},{ant_y} {ant_x-8},{ant_y-14} {ant_x+8},{ant_y-14}" fill="none" stroke="#000080" stroke-width="1.2" />')
    svg.append(f'<line x1="{ant_x-8}" y1="{ant_y-14}" x2="{ant_x+8}" y2="{ant_y-14}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<text x="{ant_x}" y="{ant_y-18}" class="comp-ref" font-family="Arial, Helvetica, sans-serif" font-size="11px" font-weight="bold" fill="#000080" text-anchor="middle">E1</text>')
    svg.append(f'<text x="{ant_x-12}" y="{ant_y-6}" class="comp-val" font-family="Arial, Helvetica, sans-serif" font-size="9.5px" font-weight="bold" fill="#000080" text-anchor="end">Antenna</text>')

    # Pins 5..8 Net Labels
    # Pin 5 (SENSOR_VP):
    svg.append(f'<line x1="250" y1="330" x2="{esp_x}" y2="330" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 330, "SENSOR_VP", "left")

    # Pin 6 (SENSOR_CAPP -> GPIO37):
    svg.append(f'<line x1="250" y1="355" x2="{esp_x}" y2="355" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 355, "GPIO37", "left")

    # Pin 7 (SENSOR_CAPN -> GPIO38):
    svg.append(f'<line x1="250" y1="380" x2="{esp_x}" y2="380" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 380, "GPIO38", "left")

    # Pin 8 (SENSOR_VN):
    svg.append(f'<line x1="250" y1="405" x2="{esp_x}" y2="405" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 405, "SENSOR_VN", "left")

    # Pin 9 (CHIP_PU -> EN RC circuit)
    # Wire from Pin 9 left to x=150
    svg.append(f'<line x1="150" y1="435" x2="{esp_x}" y2="435" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_dot(150, 435)
    add_dot(250, 435)
    # EN Label cleanly ABOVE the wire (no wire cuts through text!)
    add_net_label_horiz(250, 435, "EN", align="left", text_pos="above")

    # Up: R1 (10K) to 3V3
    add_resistor_v(150, 360, "R1", "10K")
    svg.append(f'<line x1="150" y1="402" x2="150" y2="435" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    svg.append(f'<line x1="150" y1="360" x2="150" y2="338" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_pwr_arrow_up(150, 330, "3V3")

    # Down: C3 (1uF) to GND
    svg.append(f'<line x1="150" y1="435" x2="150" y2="470" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_capacitor_v(150, 470, "C3", "1uF")
    add_gnd(150, 490)

    # Pins 10..12 Net Labels
    # Pin 10 (VDET_1 -> GPIO34):
    svg.append(f'<line x1="250" y1="465" x2="{esp_x}" y2="465" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 465, "GPIO34", "left")

    # Pin 11 (VDET_2 -> GPIO35):
    svg.append(f'<line x1="250" y1="490" x2="{esp_x}" y2="490" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 490, "GPIO35", "left")

    # Pin 12 (32K_XP -> GPIO32):
    svg.append(f'<line x1="250" y1="515" x2="{esp_x}" y2="515" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_net_label_horiz(250, 515, "GPIO32", "left")

    # --- ESP32 TOP PINS (49 EPAD and 48..37) ---
    # Centered symmetrically on the ESP32 box (X: 340..740, center = 540)
    # EPAD (49) -> GND pointing UP with GND text ABOVE it!
    svg.append(f'<text x="385" y="215" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 385,215)" text-anchor="end">GND</text>')
    svg.append(f'<text x="381" y="194" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">49</text>')
    svg.append(f'<line x1="385" y1="160" x2="385" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
    add_gnd_top_earth(385, 160)

    # Top pins spaced symmetrically between x=419 and x=661 (12 pins with dx=22, centered at x=540)
    top_pins = [
        (48, "CAP1_NC", 419, "nc"),
        (47, "CAP2_NC", 441, "nc"),
        (46, "VDDA", 463, "pwr"),
        (45, "XTAL_P_NC", 485, "nc"),
        (44, "XTAL_N_NC", 507, "nc"),
        (43, "VDDA", 529, "pwr"),
        (42, "GPIO21", 551, "net"),
        (41, "U0TXD", 573, "txd0"),
        (40, "U0RXD", 595, "rxd0"),
        (39, "GPIO22", 617, "net"),
        (38, "GPIO19", 639, "net"),
        (37, "VDD3P3_CPU", 661, "pwr")
    ]

    for pnum, pname, px, ptype in top_pins:
        svg.append(f'<text x="{px}" y="215" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 {px},215)" text-anchor="end">{pname}</text>')
        # Pin numbers shifted to the left of the wire (text-anchor="end") so wire doesn't cut through!
        svg.append(f'<text x="{px-4}" y="194" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">{pnum}</text>')
        
        if ptype == "nc":
            svg.append(f'<line x1="{px}" y1="180" x2="{px}" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        elif ptype == "pwr":
            # Solidly connected from y=200 all the way to 3V3 arrow at y=148!
            svg.append(f'<line x1="{px}" y1="148" x2="{px}" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_pwr_arrow_up(px, 140, "3V3")
        elif ptype == "net":
            svg.append(f'<line x1="{px}" y1="150" x2="{px}" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_net_label_vert(px, 150, pname, "up")
        elif ptype == "txd0":
            svg.append(f'<line x1="{px}" y1="150" x2="{px}" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_net_label_vert(px, 150, "TXD0", "up")
        elif ptype == "rxd0":
            svg.append(f'<line x1="{px}" y1="150" x2="{px}" y2="200" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_net_label_vert(px, 150, "RXD0", "up")

    # --- ESP32 RIGHT PINS (36 down to 25) ---
    right_pins = [
        (36, "GPIO23", 225, "GPIO23"),
        (35, "GPIO18", 250, "GPIO18"),
        (34, "GPIO5", 275, "GPIO5"),
        (33, "SD_DATA_1(FLASH_SD1)", 305, "FLASH_SD1"),
        (32, "SD_DATA_0(FLASH_SD3)", 335, "FLASH_SD3"),
        (31, "SD_CLK(FLASH_CLK)", 365, "FLASH_CLK"),
        (30, "SD_CMD(FLASH_SD2)", 395, "FLASH_SD2"),
        (29, "SD_DATA_3/IO10", 425, "GPIO10"),
        (28, "SD_DATA_2/IO9", 455, "GPIO9"),
        (27, "IO17(FLASH_SD0)", 485, "FLASH_SD0"),
        (26, "VDD_SDIO_NC", 510, "NC"),
        (25, "IO16(FLASH_CS)", 535, "FLASH_CS")
    ]

    for pnum, pname, py, netname in right_pins:
        svg.append(f'<text x="{esp_x+esp_w-8}" y="{py+2.5}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" text-anchor="end">{pname}</text>')
        svg.append(f'<text x="{esp_x+esp_w+4}" y="{py-2}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="start">{pnum}</text>')
        
        if netname == "NC":
            svg.append(f'<line x1="{esp_x+esp_w}" y1="{py}" x2="{esp_x+esp_w+20}" y2="{py}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
        else:
            svg.append(f'<line x1="{esp_x+esp_w}" y1="{py}" x2="{esp_x+esp_w+45}" y2="{py}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_net_label_horiz(esp_x+esp_w+45, py, netname, "right")

    # --- ESP32 BOTTOM PINS (13 to 24) ---
    # Centered symmetrically between x=419 and x=661 (12 pins with dx=22, centered at x=540)
    bot_pins = [
        (13, "32K_XN", 419, "GPIO33"),
        (14, "GPIO25", 441, "GPIO25"),
        (15, "GPIO26", 463, "GPIO26"),
        (16, "GPIO27", 485, "GPIO27"),
        (17, "MTMS", 507, "GPIO14"),
        (18, "MTDI", 529, "GPIO12"),
        (19, "VDD3P3_RTC", 551, "3V3"),
        (20, "MTCK", 573, "GPIO13"),
        (21, "MTDO", 595, "GPIO15"),
        (22, "GPIO2", 617, "GPIO2"),
        (23, "GPIO0", 639, "GPIO0"),
        (24, "GPIO4", 661, "GPIO4")
    ]

    for pnum, pname, px, netname in bot_pins:
        svg.append(f'<text x="{px}" y="{esp_y+esp_h-12}" class="pin-name" font-family="Arial, Helvetica, sans-serif" font-size="8px" font-weight="bold" fill="#000000" transform="rotate(-90 {px},{esp_y+esp_h-12})" text-anchor="start">{pname}</text>')
        # Pin numbers shifted to the left of the wire (text-anchor="end") so wire doesn't cut through!
        svg.append(f'<text x="{px-4}" y="{esp_y+esp_h+12}" class="pin-num" font-family="Arial, Helvetica, sans-serif" font-size="7.5px" fill="#000080" text-anchor="end">{pnum}</text>')
        
        if netname == "3V3":
            # Pin 19 blue wire extends down below the bottom net labels (which end at y ~ esp_y+esp_h+95)
            # and then turns right under pins 20..24 with a 3V3 power arrow pointing right
            wire_drop_y = esp_y + esp_h + 105
            svg.append(f'<line x1="{px}" y1="{esp_y+esp_h}" x2="{px}" y2="{wire_drop_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            svg.append(f'<line x1="{px}" y1="{wire_drop_y}" x2="695" y2="{wire_drop_y}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_pwr_arrow_right(703, wire_drop_y, "3V3")
        else:
            svg.append(f'<line x1="{px}" y1="{esp_y+esp_h}" x2="{px}" y2="{esp_y+esp_h+45}" class="wire" stroke="#000080" stroke-width="1.2" fill="none" />')
            add_net_label_vert(px, esp_y+esp_h+45, netname, "down")

    svg.append('</svg>')
    
    return "\n".join(svg)

if __name__ == "__main__":
    out_dir = r"d:\Documents\GitHub\WiFi-Out-of-Band-Management\hardware"
    os.makedirs(out_dir, exist_ok=True)
    svg_content = generate_schematic()
    out_svg = os.path.join(out_dir, "schematic.svg")
    out_png = os.path.join(out_dir, "schematic.png")
    with open(out_svg, "w", encoding="utf-8") as f:
        f.write(svg_content)
    print(f"Schematic SVG written to: {out_svg}")
    
    png_bytes = resvg_py.svg_to_bytes(svg_path=out_svg)
    with open(out_png, "wb") as f:
        f.write(png_bytes)
    print(f"Schematic PNG successfully rendered to: {out_png}")
