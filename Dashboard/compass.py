"""Compass picture for the dashboard, as SVG text.

North is up and angles grow clockwise, like on a real compass. Every mark is
drawn pointing north and then turned with `rotate`, so there is no
trigonometry in this file.
"""

import angles

WIDTH = 320
HEIGHT = 430
CENTRE = 160
RADIUS = 120

WHITE = '#ffffff'
INK = '#0b0b0b'
SOFT_INK = '#52514e'
RIM = '#c3c2b7'
PANEL_BLUE = '#2a78d6'
SUN_YELLOW = '#eda100'
OLD_GREY = '#a3a29b'

LETTERS = (('N', CENTRE, CENTRE - RADIUS - 16), ('E', CENTRE + RADIUS + 16, CENTRE),
           ('S', CENTRE, CENTRE + RADIUS + 16), ('W', CENTRE - RADIUS - 16, CENTRE))


def turned(deg: float, shape: str) -> str:
    """Turn a shape that points north to the direction `deg`."""
    return f'<g transform="rotate({deg:.1f} {CENTRE} {CENTRE})">{shape}</g>'


def degrees(value: float | None) -> str:
    """Give back an angle as text, for example 180.6°."""
    if value is None:
        return 'no value'
    return f'{value:.1f}°'


def rose() -> str:
    """Draw the circle, a tick every 30 degrees and the letters N, E, S, W."""
    top = CENTRE - RADIUS
    parts = [f'<circle cx="{CENTRE}" cy="{CENTRE}" r="{RADIUS}" fill="{WHITE}" stroke="{RIM}" stroke-width="2"/>']
    for deg in range(0, 360, 30):
        if deg % 90 == 0:
            tick = f'<line x1="{CENTRE}" y1="{top}" x2="{CENTRE}" y2="{top + 14}" stroke="{SOFT_INK}" stroke-width="2"/>'
        else:
            tick = f'<line x1="{CENTRE}" y1="{top}" x2="{CENTRE}" y2="{top + 8}" stroke="{RIM}" stroke-width="2"/>'
        parts.append(turned(deg, tick))
    for letter, x, y in LETTERS:
        parts.append(f'<text x="{x}" y="{y}" text-anchor="middle" dominant-baseline="central" '
                     f'font-size="16" font-weight="bold" fill="{INK}">{letter}</text>')
    return ''.join(parts)


def power_on_mark(colour: str) -> str:
    """Dashed line: the direction the panel faced at power-on."""
    top = CENTRE - RADIUS
    return (f'<line id="power-on" x1="{CENTRE}" y1="{CENTRE}" x2="{CENTRE}" y2="{top + 18}" '
            f'stroke="{colour}" stroke-width="2" stroke-dasharray="5 4"><title>At power-on</title></line>')


def panel_mark(colour: str) -> str:
    """Thick arrow: the direction the panel faces now."""
    top = CENTRE - RADIUS
    return (f'<g id="panel"><title>Panel</title>'
            f'<line x1="{CENTRE}" y1="{CENTRE}" x2="{CENTRE}" y2="{top + 44}" '
            f'stroke="{colour}" stroke-width="5" stroke-linecap="round"/>'
            f'<polygon points="{CENTRE},{top + 24} {CENTRE - 9},{top + 46} {CENTRE + 9},{top + 46}" '
            f'fill="{colour}" stroke="{WHITE}" stroke-width="2" paint-order="stroke"/></g>')


def heading_mark(colour: str) -> str:
    """Small triangle inside the rim: the compass reading now."""
    top = CENTRE - RADIUS
    return (f'<polygon id="heading" points="{CENTRE},{top + 3} {CENTRE - 7},{top + 17} {CENTRE + 7},{top + 17}" '
            f'fill="{colour}" stroke="{WHITE}" stroke-width="2" paint-order="stroke"><title>Compass now</title></polygon>')


def sun_mark(colour: str, sun_up: bool) -> str:
    """Circle on the rim: filled when the sun is above the horizon, empty when below."""
    fill = colour if sun_up else WHITE
    return (f'<circle id="sun" cx="{CENTRE}" cy="{CENTRE - RADIUS}" r="9" fill="{fill}" '
            f'stroke="{SOFT_INK}" stroke-width="2"><title>Sun</title></circle>')


def legend_row(y: int, glyph: str, text: str) -> str:
    """One line under the compass: a small copy of the mark and its value."""
    return (f'{glyph}<text x="48" y="{y}" dominant-baseline="central" font-size="13" '
            f'fill="{INK}">{text}</text>')


def draw(panel: float | None, sun: float | None, sun_elevation: float | None,
         power_on: float | None, heading: float | None, live: bool) -> str:
    """Give back the compass as SVG text. A value that is None is not drawn.

    panel: direction the panel faces now. sun, sun_elevation: where the sun is.
    power_on: direction at power-on (PAN). heading: compass reading now (HDG).
    live: False draws everything in grey because no telemetry is arriving.
    """
    panel_colour = PANEL_BLUE if live else OLD_GREY
    sun_colour = SUN_YELLOW if live else OLD_GREY
    ink_colour = INK if live else OLD_GREY
    line_colour = SOFT_INK if live else OLD_GREY
    sun_up = sun_elevation is not None and sun_elevation > 0.0

    opening = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {WIDTH} {HEIGHT}" width="{WIDTH}" '
               f'height="{HEIGHT}" font-family="sans-serif" role="img" aria-label="Compass">')
    parts = [opening, rose()]
    if power_on is not None:
        parts.append(turned(power_on, power_on_mark(line_colour)))
    if panel is not None:
        parts.append(turned(panel, panel_mark(panel_colour)))
    if heading is not None:
        parts.append(turned(heading, heading_mark(ink_colour)))
    if sun is not None:
        parts.append(turned(sun, sun_mark(sun_colour, sun_up)))
    parts.append(f'<circle cx="{CENTRE}" cy="{CENTRE}" r="4" fill="{ink_colour}"/>')

    sun_text = f'Sun {degrees(sun)}'
    if sun is not None and sun_elevation is not None:
        if sun_up:
            sun_text += f', {sun_elevation:.1f}° above the horizon'
        else:
            sun_text += f', {-sun_elevation:.1f}° below the horizon'
    heading_text = f'Compass now {degrees(heading)}'
    if heading is not None and power_on is not None:
        heading_text += f' ({angles.difference(heading, power_on):+.1f}° from power-on)'

    panel_glyph = (f'<line x1="16" y1="326" x2="38" y2="326" stroke="{panel_colour}" '
                   f'stroke-width="5" stroke-linecap="round"/>')
    sun_glyph = (f'<circle cx="27" cy="350" r="7" fill="{sun_colour if sun_up else WHITE}" '
                 f'stroke="{SOFT_INK}" stroke-width="2"/>')
    heading_glyph = f'<polygon points="27,367 20,381 34,381" fill="{ink_colour}"/>'
    power_on_glyph = (f'<line x1="16" y1="398" x2="38" y2="398" stroke="{line_colour}" '
                      f'stroke-width="2" stroke-dasharray="5 4"/>')
    parts.append(legend_row(326, panel_glyph, f'Panel faces {degrees(panel)}'))
    parts.append(legend_row(350, sun_glyph, sun_text))
    parts.append(legend_row(374, heading_glyph, heading_text))
    parts.append(legend_row(398, power_on_glyph, f'At power-on {degrees(power_on)}'))
    if not live:
        parts.append(f'<text x="16" y="420" font-size="13" fill="{SOFT_INK}">No telemetry: last known values</text>')
    parts.append('</svg>')
    return ''.join(parts)