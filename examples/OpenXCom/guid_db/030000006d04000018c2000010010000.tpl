# Specific configuration file for Logitech Logitech RumblePad 2 USB

[xboxdrv]
silent = true
# Using the 'by-id' name is recomment, as it is static, while an
# /dev/input/eventX name can change depending on what other USB
# devices you use.
evdev = <MAP_EVDEV>

deadzone = 15%

# This displays events received from the controller, if you are
# working on a configuration you want to set this to true:
evdev-debug = false

# Grabbing the device prevents other applications from accessing it,
# this is needed most of the time te prevent applications from
# receiving events twice.
evdev-grab = true
# mimic-xpad = true
# dpad-as-button=true

[evdev-absmap]
<MAP_BUTTON:dpad_x> = dpad_x
<MAP_BUTTON:dpad_y> = dpad_y
<MAP_BUTTON:x1> = X1
<MAP_BUTTON:y1> = Y1
<MAP_BUTTON:x2> = X2
<MAP_BUTTON:y2> = Y2

[evdev-keymap]
<MAP_BUTTON:tl> = TL
<MAP_BUTTON:tr> = TR
<MAP_BUTTON:y> = Y
<MAP_BUTTON:x> = X
<MAP_BUTTON:b> = B
<MAP_BUTTON:a> = A
<MAP_BUTTON:lb> = LB
<MAP_BUTTON:lt> = LT
<MAP_BUTTON:rb> = RB
<MAP_BUTTON:rt> = RT
<MAP_BUTTON:start> = start
<MAP_BUTTON:back> = back

[calibration]

[axismap]
<AXISMAP:-Y1>=Y1
<AXISMAP:-Y2>=Y2

# Game specific settings
[ui-axismap]
x1=REL_X:10:20
y1=REL_Y:10:20
#y2=REL_WHEEL:5:100
#x2=REL_HWHEEL:5:100
#trigger=REL_WHEEL:5:100

[ui-buttonmap]
a=BTN_LEFT
b=BTN_RIGHT
x=BTN_MIDDLE
y=KEY_ENTER

dl=KEY_LEFT
dr=KEY_RIGHT
du=KEY_UP
dd=KEY_DOWN
back=KEY_ESC

# OpenXcom specific
RB+DD=KEY_PAGEDOWN
RB+DU=KEY_PAGEUP
RB+DL=KEY_LEFTBRACE
RB+DR=KEY_RIGHTBRACE
guide = KEY_F5
RB+guide = KEY_F9
back+start = exec:kill_openxcom.sh

# Keyboard support - write name of base
RT+DU=cycle-key-named:keyboard:KEY_DELETE+KEY_A+KEY_LEFT:KEY_DELETE+KEY_B+KEY_LEFT:KEY_DELETE+KEY_C+KEY_LEFT:KEY_DELETE+KEY_D+KEY_LEFT:KEY_DELETE+KEY_E+KEY_LEFT:KEY_DELETE+KEY_F+KEY_LEFT:KEY_DELETE+KEY_G+KEY_LEFT:KEY_DELETE+KEY_H+KEY_LEFT:KEY_DELETE+KEY_I+KEY_LEFT:KEY_DELETE+KEY_J+KEY_LEFT:KEY_DELETE+KEY_K+KEY_LEFT:KEY_DELETE+KEY_L+KEY_LEFT:KEY_DELETE+KEY_M+KEY_LEFT:KEY_DELETE+KEY_N+KEY_LEFT:KEY_DELETE+KEY_O+KEY_LEFT:KEY_DELETE+KEY_P+KEY_LEFT:KEY_DELETE+KEY_Q+KEY_LEFT:KEY_DELETE+KEY_R+KEY_LEFT:KEY_DELETE+KEY_S+KEY_LEFT:KEY_DELETE+KEY_T+KEY_LEFT:KEY_DELETE+KEY_U+KEY_LEFT:KEY_DELETE+KEY_V+KEY_LEFT:KEY_DELETE+KEY_W+KEY_LEFT:KEY_DELETE+KEY_X+KEY_LEFT:KEY_DELETE+KEY_Y+KEY_LEFT:KEY_DELETE+KEY_Z+KEY_LEFT
RT+DD=cycle-key-ref:keyboard
RT+DL=KEY_BACKSPACE:KEY_CAPSLOCK:1000
RT+DR=KEY_SPACE:KEY_CAPSLOCK:1000

# EOF #
