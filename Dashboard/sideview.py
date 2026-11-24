"""Side view of the tracker for the dashboard, as SVG text.

The tracker is seen from the side: the panel faces to the right. The scale
on the right goes from 0 degrees (the horizon) up to 90 degrees (straight
up). Every mark is drawn pointing at the horizon and then turned up with
`rotate`, so there is no trigonometry in this file.

The view compares heights only. Whether the panel is also turned towards
the sun is shown by the compass.
"""

import compass

WIDTH = 320
HEIGHT = 390
AXIS_X = 104
AXIS_Y = 168
RADIUS = 128
GROUND_Y = 268

PART_GREY = '#d9d7cc'

NOT_MEASURED = ('Tilt = last target the right Nano took (EL),', 'not a measured angle.')


def lifted(deg: float, shape: str) -> str:
    """Turn a shape that points at the horizon (to the right) up by `deg`."""
    return f'<g transform="rotate({-deg:.1f} {AXIS_X} {AXIS_Y})">{shape}</g>'


def scale() -> str:
    """Draw the quarter circle from 0 to 90 degrees with a tick every 15 and a number every 30."""
    right = AXIS_X + RADIUS
    parts = [f'<path d="M {right} {AXIS_Y} A {RADIUS} {RADIUS} 0 0 0 {AXIS_X} {AXIS_Y - RADIUS}" '
             f'fill="none" stroke="{compass.RIM}" stroke-width="2"/>',
             f'<line x1="{AXIS_X}" y1="{AXIS_Y}" x2="{right}" y2="{AXIS_Y}" stroke="{compass.RIM}" '
             f'stroke-width="1" stroke-dasharray="3 4"/>']
    for deg in range(0, 91, 15):
        if deg % 30 == 0:
            tick = (f'<line x1="{right}" y1="{AXIS_Y}" x2="{right - 12}" y2="{AXIS_Y}" '
                    f'stroke="{compass.SOFT_INK}" stroke-width="2"/>')
            number = (f'<text x="{right + 20}" y="{AXIS_Y}" text-anchor="middle" dominant-baseline="central" '
                      f'font-size="12" fill="{compass.SOFT_INK}" '
                      f'transform="rotate({deg} {right + 20} {AXIS_Y})">{deg}°</text>')
            parts.append(lifted(deg, tick + number))
        else:
            tick = (f'<line x1="{right}" y1="{AXIS_Y}" x2="{right - 7}" y2="{AXIS_Y}" '
                    f'stroke="{compass.RIM}" stroke-width="2"/>')
            parts.append(lifted(deg, tick))
    return ''.join(parts)


def stand() -> str:
    """Draw the ground line, the base plate, the stepper motor and the mast."""
    return (f'<line x1="14" y1="{GROUND_Y}" x2="306" y2="{GROUND_Y}" stroke="{compass.SOFT_INK}" stroke-width="2"/>'
            f'<rect x="34" y="{GROUND_Y - 12}" width="150" height="12" fill="{PART_GREY}" '
            f'stroke="{compass.SOFT_INK}" stroke-width="1.5"/>'
            f'<rect x="{AXIS_X - 14}" y="{GROUND_Y - 32}" width="28" height="20" fill="{PART_GREY}" '
            f'stroke="{compass.SOFT_INK}" stroke-width="1.5"/>'
            f'<line x1="{AXIS_X}" y1="{GROUND_Y - 32}" x2="{AXIS_X}" y2="{AXIS_Y}" '
            f'stroke="{compass.SOFT_INK}" stroke-width="6"/>')


def panel_mark(colour: str, holder_colour: str) -> str:
    """Panel standing upright, with an arrow showing where its face points."""
    return (f'<g id="panel"><title>Panel</title>'
            f'<line x1="{AXIS_X}" y1="{AXIS_Y}" x2="{AXIS_X + 5}" y2="{AXIS_Y}" stroke="{holder_colour}" stroke-width="6"/>'
            f'<rect x="{AXIS_X + 5}" y="{AXIS_Y - 62}" width="9" height="124" rx="2" fill="{colour}" '
            f'stroke="{compass.WHITE}" stroke-width="1.5"/>'
            f'<line x1="{AXIS_X + 14}" y1="{AXIS_Y}" x2="{AXIS_X + 78}" y2="{AXIS_Y}" '
            f'stroke="{colour}" stroke-width="4" stroke-linecap="round"/>'
            f'<polygon points="{AXIS_X + 96},{AXIS_Y} {AXIS_X + 76},{AXIS_Y - 8} {AXIS_X + 76},{AXIS_Y + 8}" '
            f'fill="{colour}" stroke="{compass.WHITE}" stroke-width="2" paint-order="stroke"/></g>')


def sun_mark(colour: str, sun_up: bool) -> str:
    """Circle on the scale: filled with a dashed line to it when the sun is up, empty when it is down."""
    right = AXIS_X + RADIUS
    line = ''
    fill = compass.WHITE
    if sun_up:
        line = (f'<line x1="{AXIS_X}" y1="{AXIS_Y}" x2="{right - 10}" y2="{AXIS_Y}" stroke="{colour}" '
                f'stroke-width="2" stroke-dasharray="5 4"/>')
        fill = colour
    return (f'{line}<circle id="sun" cx="{right}" cy="{AXIS_Y}" r="9" fill="{fill}" '
            f'stroke="{compass.SOFT_INK}" stroke-width="2"><title>Sun</title></circle>')


def note(y: int, text: str) -> str:
    """One small grey line of text under the picture."""
    return f'<text x="16" y="{y}" dominant-baseline="central" font-size="12" fill="{compass.SOFT_INK}">{text}</text>'


def draw(elevation: float | None, sun_elevation: float | None, live: bool) -> str:
    """Give back the side view as SVG text. A value that is None is not drawn.

    elevation: tilt last taken by the right Nano (EL), 0 = upright, 90 = flat.
    sun_elevation: height of the sun (SEL), negative below the horizon.
    live: False draws everything in grey because no telemetry is arriving.
    """
    panel_colour = compass.PANEL_BLUE if live else compass.OLD_GREY
    sun_colour = compass.SUN_YELLOW if live else compass.OLD_GREY
    ink_colour = compass.INK if live else compass.OLD_GREY
    line_colour = compass.SOFT_INK if live else compass.OLD_GREY
    sun_up = sun_elevation is not None and sun_elevation > 0.0

    opening = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {WIDTH} {HEIGHT}" width="{WIDTH}" '
               f'height="{HEIGHT}" font-family="sans-serif" role="img" aria-label="Side view">')
    parts = [opening, scale(), stand()]
    if sun_elevation is not None:
        shown = sun_elevation if sun_up else 0.0
        parts.append(lifted(shown, sun_mark(sun_colour, sun_up)))
    if elevation is not None:
        parts.append(lifted(elevation, panel_mark(panel_colour, line_colour)))
    parts.append(f'<circle cx="{AXIS_X}" cy="{AXIS_Y}" r="4" fill="{ink_colour}"/>')

    panel_text = 'Panel tilt no value'
    if elevation is not None:
        panel_text = f'Panel tilted {elevation:.0f}° back from upright'
    sun_text = 'Sun no value'
    if sun_elevation is not None:
        if sun_up:
            sun_text = f'Sun {sun_elevation:.1f}° above the horizon'
        else:
            sun_text = f'Sun {-sun_elevation:.1f}° below the horizon'

    panel_glyph = (f'<line x1="16" y1="296" x2="38" y2="296" stroke="{panel_colour}" '
                   f'stroke-width="5" stroke-linecap="round"/>')
    sun_glyph = (f'<circle cx="27" cy="320" r="7" fill="{sun_colour if sun_up else compass.WHITE}" '
                 f'stroke="{compass.SOFT_INK}" stroke-width="2"/>')
    parts.append(compass.legend_row(296, panel_glyph, panel_text))
    parts.append(compass.legend_row(320, sun_glyph, sun_text))
    parts.append(note(344, NOT_MEASURED[0]))
    parts.append(note(359, NOT_MEASURED[1]))
    if not live:
        parts.append(f'<text x="16" y="380" font-size="13" fill="{compass.SOFT_INK}">No telemetry: last known values</text>')
    parts.append('</svg>')
    return ''.join(parts)