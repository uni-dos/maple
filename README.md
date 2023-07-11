# Maple

An attempt at creating a floating, wayland compositor. Based on [tinywl] (https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/master/tinywl/tinywl.c) and inspired from various other wayland compositors.


## Why another wayland compositor

There currently aren’t many wayland compositors that I find suit me.
I like Gnome’s gesture implementation with smooth animation but there are applications that don’t have client side decorations (CSD) or do and don’t match Gnome’s style.
I like that KDE has server side decorations (SSD) that can attempt to make applications feel consistent if they don’t have CSD, but it’s gestures aren’t customizable and isn’t super stable.
Sway is great but it has no animations and relies on the keyboard. I could continue on but every compositor I tried has its strengths, but there is always something missing in my opinion.


## Planned features

* Protocols such as layer shell, etc.
* Workspaces (maybe with the ext-workspace protocol?)
* Animations (Hyperland is a great at this)
* XDG decoration support (see if I can use gtk to create SSD like many gtk x11 window managers)
* Touchpad gestures (swipe between workspaces, see all windows in a workspace/monitor)
* Automatic tiling (like pop shell)

## Current Status

I have implemented most of tinywl but it currently does not display clients. I need to finish implementing the xdg and xwayland logic.
After that it should be a minimum viable product without the features listed above.
