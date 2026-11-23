"""Top view of the tracker for the dashboard, as SVG text.

The tracker is seen from above with north up, on the same circle as the
compass. The base plate is drawn the way it stands on the ground, the panel
the way it faces now. Every mark is drawn pointing north and then turned
with `compass.turned`, so there is no trigonometry in this file.

The panel is drawn as a bar: its top edge, as seen when it stands upright.
"""

import compass
import sideview

WIDTH = 320
HEIGHT = 444

BASE_FILL = '#f1efe6'

NOT_MEASURED = ('Turn = last target the right Nano took (AZ),', 'not a measured angle.')


def base_mark(line_colour: str) -> str:
    """Base plate with its board side pointing north: board, display and stepper motor."""
    centre = compass.CENTRE
    return (f'<g id="base"><title>Base</title>'
            f'<rect x="{centre - 72}" y="{centre - 48}" width="144" height="96" rx="4" fill="{BASE_FILL}" '
            f'stroke="{line_colour}" stroke-width="1.5"/>'
            f'<rect x="{centre + 2}" y="{centre - 43}" width="62" height="20" fill="{sideview.PART_GREY}" '
            f'stroke="{line_colour}" stroke-width="1"/>'
            f'<text x="{centre + 33}" y="{centre - 33}" text-anchor="middle" dominant-baseline="central" '
            f'font-size="9" fill="{line_colour}">board</text>'
            f'<rect x="{centre - 48}" y="{centre - 41}" width="40" height="9" fill="{sideview.PART_GREY}" '
            f'stroke="{line_colour}" stroke-width="1"/>'
            f'<rect x="{centre - 14}" y="{centre - 14}" width="28" height="28" fill="{sideview.PART_GREY}" '
            f'stroke="{line_colour}" stroke-width="1"/></g>')


def panel_mark(colour: str) -> str:
    """Panel as a bar across the mast, with an arrow showing where its face points."""
    centre = compass.CENTRE
    top = centre - compass.RADIUS
    return (f'<g id="panel"><title>Panel</title>'
            f'<line x1="{centre}" y1="{centre - 8}" x2="{centre}" y2="{top + 44}" '
            f'stroke="{colour}" stroke-width="5" stroke-linecap="round"/>'
            f'<polygon points="{centre},{top + 24} {centre - 9},{top + 46} {centre + 9},{top + 46}" '
            f'fill="{colour}" stroke="{compass.WHITE}" stroke-width="2" paint-order="stroke"/>'
            f'<rect x="{centre - 92}" y="{centre - 8}" width="184" height="13" rx="2" fill="{colour}" '
            f'stroke="{compass.WHITE}" stroke-width="1.5"/></g>')


def draw(panel: float | None, power_on: float | None, sun: float | None,
         sun_elevation: float | None, live: bool) -> str:
    """Give back the top view as SVG text. A value that is None is not drawn.

    panel: direction the panel faces now. power_on: direction the panel faced
    at power-on (PAN), which is the direction the base's board side faces.
    sun, sun_elevation: where the sun is.
    live: False draws everything in grey because no telemetry is arriving.
    """
    panel_colour = compass.PANEL_BLUE if live else compass.OLD_GREY
    sun_colour = compass.SUN_YELLOW if live else compass.OLD_GREY
    ink_colour = compass.INK if live else compass.OLD_GREY
    line_colour = compass.SOFT_INK if live else compass.OLD_GREY
    sun_up = sun_elevation is not None and sun_elevation > 0.0

    opening = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {WIDTH} {HEIGHT}" width="{WIDTH}" '
               f'height="{HEIGHT}" font-family="sans-serif" role="img" aria-label="Top view">')
    parts = [opening, compass.rose()]
    if power_on is not None:
        parts.append(compass.turned(power_on, base_mark(line_colour)))
    if panel is not None:
        parts.append(compass.turned(panel, panel_mark(panel_colour)))
    if sun is not None:
        parts.append(compass.turned(sun, compass.sun_mark(sun_colour, sun_up)))
    parts.append(f'<circle cx="{compass.CENTRE}" cy="{compass.CENTRE}" r="4" fill="{ink_colour}"/>')

    panel_glyph = (f'<line x1="16" y1="326" x2="38" y2="326" stroke="{panel_colour}" '
                   f'stroke-width="5" stroke-linecap="round"/>')
    base_glyph = (f'<rect x="17" y="343" width="20" height="14" rx="2" fill="{BASE_FILL}" '
                  f'stroke="{line_colour}" stroke-width="1.5"/>')
    sun_glyph = (f'<circle cx="27" cy="374" r="7" fill="{sun_colour if sun_up else compass.WHITE}" '
                 f'stroke="{compass.SOFT_INK}" stroke-width="2"/>')
    parts.append(compass.legend_row(326, panel_glyph, f'Panel faces {compass.degrees(panel)}'))
    parts.append(compass.legend_row(350, base_glyph, f'Base: board side faces {compass.degrees(power_on)}'))
    parts.append(compass.legend_row(374, sun_glyph, f'Sun {compass.degrees(sun)}'))
    parts.append(sideview.note(398, NOT_MEASURED[0]))
    parts.append(sideview.note(413, NOT_MEASURED[1]))
    if not live:
        parts.append(f'<text x="16" y="434" font-size="13" fill="{compass.SOFT_INK}">No telemetry: last known values</text>')
    parts.append('</svg>')
    return ''.join(parts)