# Emulation of a Spark Control X using an ESP32    

<p align="center">
  <img src="https://github.com/paulhamsh/Spark-Control-X/blob/main/IMG_5301.jpg" width="400" title="Spark Control XYZ Picture 1">
  <img src="https://github.com/paulhamsh/Spark-Control-X/blob/main/IMG_5302.jpg" width="400" title="Spark Control XYZ Picture 2">
</p>

## To compile and run, using NimBLE-Arduino, make this change first

```
NimBLE      v1.4.2
LVGL        v9.1.0
LovyanGFX   v1.1.16
TAMC_GT911  v1.0.2
ESP32       v2.0.16
```


## The Spark Control X hardware pedal

The Spark Control X has:
- six buttons
- two expression pedals (inputs)
- six lights (one for each button)
- power, amp and app connection lights

The Spark Control X has two modes of use:
- connected to the Spark APP and the app sends the commands to the amp (Spark Control mode) - Spark 40, GO and MINI   
- connected to the Spark AMP directly - Spark LIVE and II

The ability to configure the Spark Control X via the LIVE and Spark II amps is not completely implemented by Positive Grid yet (as of 1 September 2024)     


## APP mode (Spark 40, GO, MINI)    

Spark Control mode is where the APP connects to the pedal     

When a button is pressed the pedal writes a value to characteristic 362f71a0-6c96-11e3-981f-0800200c9a66   
When the button is released, ```0``` is written to characteristic 362f71a0-6c96-11e3-981f-0800200c9a66    
Each button is represented as a bit position, so multiple buttons are summed (or bitwise OR) together and sent as a single value to the characteristic.    


  **I**       |   **II**    |     **A**
--------------|-------------|----------
  01          |   02        |     04

  **III**     |  **IV**     |     **B**  
--------------|-------------|---------- 
  08          |   10        |     20   
  

Example:
```
Press I and II         03               0000 0011
Press III and A        0C               0000 1100
Press IV and B         30               0011 0000
```


For reference, the original Spark Control sends these values

  **I**   | **II**  | **III** | **IV**
----------|---------|---------|----------
  01      |  04     |   02    |   08


## AMP mode (Spark LIVE, Spark II)   

LIVE mode is where the AMP connects directly to the pedal, without any app involvement.
BLE messages use UUIDs FFCA (from amp) and FFC9 (to amp).

Button presses and expression pedal changes generate BLE messages from the pedal to the amp    
The amp sends messages to the pedal to control the lights    
The amp can also send request messages to the pedal to obtain its configuration    

The Spark Control X sends these messages:
- button press
- expression pedal cable insert / removal
- expression pedal value

It responds to these requests:
- firmware id    
- what is the current 'profil'
- what is the profile name
- which button sends which message

It actions these messages:
- set light RGB values

Each configuration of the pedal has a set of six messages to send to the amp, each message assigned to a button.   
There are eight configuration 'profiles' stored by the pedal    

Long pressing A increases the profile, Long pressing B decreases the profile.   
The profile are numbered 1 to 8. A long press of A when the profile is 8 has no effect. A long press of B when the profile is 7 has no effect.    
There seem to be two profile for the looper - unsure of what these do.    


The amp recognises various button messages as either:   
- select a preset
- change to next preset up
- change to second set of presets
- effect on/off
- change to next / previous profile

The amp controls the lights on the pedal.   To do this it sends a message to change the RGB value for that light position. It needs to know which light corresponds to each pedal message, and this is sent from the pedal in a separate message.   

The sequence is:
- amp asks pedal for current profile
- pedal responds (1-8)
- amp asks pedal for button message order (same as mapping message to light location)
- pedal responds
- amp sends messages to change lamp colours and brightness (two messages per light, not clear yet why)


## Table of messages

Direction       | Message number | Response? | Details
----------------|----------------|-----------|-------------------------------
To amp          | 0x03           |           | Button press message
To amp          | 0x0c           |           | Expression pedal values
To amp          | 0x0d           |           | Expression pedal cable inserted / removed
To pedal        | 0x01           | No        | Set button lamp colours
To pedal        | 0x03           | No        | Set button message
To pedal        | 0x04           | No        | UNKNOWN
To pedal        | 0x07           | No        | Set the current profile
To pedal        | 0x08           | Yes       | Get current profile
To pedal        | 0x0a           | No        | Set other lamp colours (unused?)
To pedal        | 0x0b           | Yes       | Get firmware version
To pedal        | 0x0d           | Yes       | Get expression pedal status
To pedal        | 0x0e           | No        | Set expression pedal calibration
To pedal        | 0x0f           | Yes       | Get battery status
To pedal        | 0x10           | Yes       | UNKNOWN
To pedal        | 0x11           | Yes       | UNKNOWN
To pedal        | 0x12           | Yes       | Get profile name
To pedal        | 0x13           | No        | Set profile name
To pedal        | 0x14           | Yes       | Get profile message layout


## Messages from pedal

### Button press messages from pedal (0x03)

```
03  00 00 00  01
```

Header       |  Message           
-------------|------------------
03 00 00 00  |  Per table 

Message value | Message 
--------------|------------------
00            | Change to preset 1
01            | Change to preset 2
02            | Change to preset 3
03            | Change to preset 4
08            | Increment preset
09            | UNKNOWN
0C            | Change preset bank (red / green)
10            | Toggle Gate
11            | Toggle Comp / Wah
12            | Toggle Drive
13            | Toggle Mod / EQ
14            | Toggle Delay
15            | Toggle Reverb
20            | Creative Wah 1
21            | Custom Wah 1
22            | Creative Wah 2
23            | Custom Wah 3
30            | Tune on/off
31            | Tap tempo
42            | LOOPER ?
44            | LOOPER ?
48            | LOOPER ?
60            | YouTube Jump to start
61            | YouTube Fast forward 10s
62            | YouTube Rewind 10s
63            | YouTube Switch playback speed
64            | Music play/pause
70            | Expression Wah LFO
72            | Expression Volume
75            | Expression Music Volume
F9            | LOOPER ?
FA            | LOOPER ?
FB            | LOOPER ?
FC            | LOOPER ?
FD            | Profile up
FE            | Profile down

#### Standard button / message layout
    
Type       |  **I**       |   **II**    |     **A**
-----------|--------------|-------------|-------
Profile 1  |  00          |   01        |     0C
Profile 2  |  10          |   11        |     14   
Long press |              |             |     FD  

Type       |  **III**     |  **IV**     |     **B**  
-----------|--------------|-------------|------- 
Profile 1  |  02          |   03        |     08   
Profile 2  |  12          |   13        |     15  
Long press |              |             |     FE 

Default pedal configurations for two profiles - Tone and Effect   

Mode   | Control  |  Effect
-------|---------|--------------------------
Tone   | I       | Select tone 1 (or 5)
Tone   | II      | Select tone 2 (or 6)
Tone   | III     | Select tone 3 (or 7)
Tone   | IV      | Select tone 4 (or 8)
Tone   | A       | Change bank (green / red)
Tone   | B       | Cycle up the presets (0 to 3)
Tone   | Long A  | Switch to Effect
Tone   | Long B  | Switch to Effect
Effect | I       | Toggle Gate
Effect | II      | Toggle Comp / Wah
Effect | III     | Toggle Drive
Effect | IV      | Toggle Mod / EQ
Effect | A       | Toggle Delay
Effect | B       | Toggle Reverb
Effect | Long A  | Profile up
Effect | Long B  | Profile down



### Expression pedal value (0x0c)

This message shows the value of each expression pedal, in the range 0x0000 to 0xffff

Header       |  Pedal 1 | Value pedal 1 | Pedal 2 | Value pedal 2         
-------------|----------|---------------|---------|--------------
0c 00 00 00  |  01      | 0000 - ffff   | 02      | 0000 - ffff


```
0c000000 01 9837 02 ffff
0c000000 01 e846 02 ffff
0c000000 01 b052 02 ffff
0c000000 01 405a 02 ffff

0c000000 01 7813 02 a801
0c000000 01 7813 02 9000
0c000000 01 7813 02 a801
0c000000 01 7813 02 9800
0c000000 01 7813 02 c001
0c000000 01 7813 02 9000
0c000000 01 7813 02 a001

```
### Expression pedal insert (0x0d)

```
0d 00 00 00 00					No expression
0d 00 00 00 01					Expression 1 plugged in
0d 00 00 00 02					Expression 2 plugged in
0d 00 00 00 03                                  Both pedals plugged in
```

Header       |  Value           
-------------|----------
0d 00 00 00  |  00 - 03

Value | Explanation
------|--------------------------------
00    | No expression pedal
01    | Expression pedal in input 1
02    | Expression pedal in input 2
03    | Expression pedal in both inputs

### Expression pedal calibrate (0x0d)

Calibrates the expression pedal.    

```
Expression 1
0e 00 00 00 01 01				Start calibration
0e 00 00 00 02 01				Set toe down
0e 00 00 00 03 01				Set toe up

Expression 2
0e 00 00 00 01 02				Start calibration
0e 00 00 00 02 02				Set toe down
0e 00 00 00 03 02				Set toe up
```


## Messages to pedal

### Light messages to pedal (0x01)

```
01 00 00 00 01  01  01  FF  FF  00 00 00  00
```
	    
Header      | tbd |  Light   |  tbd       | Brightness | tbd | Blue    | Green   | Red     | tbd
------------|-----|----------|------------|------------|-----|---------|---------|---------|----
01 00 00 00 | 01  |  00 - 07 |   01 or 02 | 00 - FF    | 00  | 00 - FF | 00 - FF | 00 - FF | 00

Lamp number | Lamp
------------|-----
00          | I
01          | II
02          | A
03          | III
04          | IV
05          | B
06          | Amp
07          | App
??          | Power

### Button set message (0x03)

```
03  00 00 00  01 01 80 01 05 01 00
```

Header       |  Profile | Button  | tbd         | Message           
-------------|----------|---------|-------------|---------------------------
03 00 00 00  |  01 - 08 | 01 - 08 | 80 01 05 01 | Per table (see above 0x03)

Buttons 07 and 08 relate to the expression pedal use     

```
Control Creative Wah		3 0 0 0 1 1 80 6 5 1 20 1 5 1 22 
Control Custom Wah		3 0 0 0 1 1 80 6 5 1 21 1 5 1 23
YouTube Jump to start		3 0 0 0 1 1 80 1 5 1 60 
YouTube Fast forward 10s	3 0 0 0 1 1 80 2 5 1 61 
YouTube Rewind 10s		3 0 0 0 1 1 80 2 5 1 62
YouTube Switch playback speed	3 0 0 0 1 1 80 1 5 1 63 
Music play/pause		3 0 0 0 1 1 80 1 5 1 64 
Tuner on/off			3 0 0 0 1 1 80 1 5 1 30 
Tap tempo			3 0 0 0 1 1 80 1 5 1 31 


Expression Wah LFO		3 0 0 0 1 7 80 1 5 1 70 
Expression Volume		3 0 0 0 1 7 80 1 5 1 72 
Expression Music Volume		3 0 0 0 1 7 80 1 5 1 75
```
### Battery level (0x0f)

```
Amp send:   0f 00 00 00 00 
Pedal send: 0f 00 00 00 00 xx
```

xx in range 0x00 to 0x63 (0% to 99%)    

### Profile configuration mapping (0x14)

The profile configuration response defines which button sends which message       

```
                     I    II   A     III  IV   B
14 00 00 00   03     00   01   0C    02   03   08    72 75
```

#### Profiles as loaded on pedal by default

These are the messages sent by default as the pedal is shipped.    

Header       |  Profile | I     | II    | A     | III   |IV     | B     | Exp1 | Exp2         
-------------|----------|-------|-------|-------|-------|-------|-------|------|------
14 00 00 00  |  00      | FF    | FF    | FD    | FF    | FF    | FE    | FF   | FF
14 00 00 00  |  01      | 00    | 01    | 0C    | 02    | 03    | 08    | 72   | 75
14 00 00 00  |  02      | 10    | 11    | 14    | 12    | 13    | 15    | 72   | 75
14 00 00 00  |  03      | 00    | 01    | 0C    | 02    | 03    | 08    | 72   | 75
14 00 00 00  |  04      | 10    | 11    | 14    | 12    | 13    | 15    | 72   | 75
14 00 00 00  |  05      | 00    | 01    | 0C    | 02    | 03    | 08    | 72   | 75
14 00 00 00  |  06      | 10    | 11    | 14    | 12    | 13    | 15    | 72   | 75
14 00 00 00  |  07      | 00    | 01    | 0C    | 02    | 03    | 08    | 72   | 75
14 00 00 00  |  08      | 10    | 11    | 14    | 12    | 13    | 15    | 72   | 75
14 00 00 00  |  09      | FF    | 42    | 0C    | 44    | 48    | 08    | FF   | FF
14 00 00 00  |  0A      | FC    | FB    | 0C    | FA    | F9    | 08    | FF   | FF

A special profile 0 has the mapping of long press messages. These are the same long presses regardless of profile selected. Profile 0 cannot be selected.     
Profiles 9 and A cannot be selected.     

```
Amp send:   14 00 00 00 00 
Pedal send: 14 00 00 00 00 FF FF FD FF FF FE FF FF

Amp send:   14 00 00 00 01 
Pedal send: 14 00 00 00 01 00 01 0C 02 03 08 72 75

Amp send:   14 00 00 00 02 
Pedal send: 14 00 00 00 02 10 11 14 12 13 15 72 75

Amp send:   14 00 00 00 03 
Pedal send: 14 00 00 00 03 00 01 0C 02 03 08 72 75

Amp send:   14 00 00 00 04 
Pedal send: 14 00 00 00 05 10 11 14 12 13 15 72 75
```

#### The recommended profiles

The app recommends these profiles be set on the pedal.    

Profile number | Name            | I   | II  | A   | III | IV  | B   | Exp1 | Exp2
---------------|-----------------|-----|-----|-----|-----|-----|-----|------|------
1              | Preset Mode 1   | 00  | 01  | 0C  | 02  | 03  | 08  | 72   | 75  
2              | Stompbox Mode 1 | 10  | 11  | 14  | 12  | 13  | 15  | 72   | 75  
3              | Music/Preset 1  | 64  | 62  | 0C  | 63  | 60  | 30  | 72   | 75  
4              | Music/FX 1      | 64  | 12  | 09  | 13  | 15  | 08  | 72   | 75  
5              | Preset Mode 2   | 00  | 01  | 0C  | 02  | 03  | 30  | 72   | 75  
6              | Stompbox Mode 2 | 12  | 13  | 0C  | 14  | 15  | 08  | 72   | 75  
7              | Music/Preset 2  | 64  | 63  | 09  | 60  | 0C  | 08  | 72   | 75  
8              | Music/FX 2      | 64  | 63  | 13  | 60  | 12  | 15  | 72   | 75  

### Lamp settings (0x0a) - unused?

```
    0a 00 00 00 00 00 01        
```

Header      | Setting   
------------|---------
0a 00 00 00 | xx xx xx   

Setting     | Effect
------------|-----------------------------------
00 00 00    | Orange power, amp, I II III IV A B
00 00 01    | Blue amp
00 00 02    | Flashing pink power
00 00 04    | All off
00 00 08    | All off
00 00 10    | All off
00 00 20    | All off
00 00 40    | All off
00 00 80    | All off
00 01 00    | Blue amp and app
00 02 00    | Flashing red power
00 04 00    | Flashing red power
00 08 00    | All off
00 10 00    | All off
00 20 00    | All off
00 40 00    | All off
00 80 00    | Flashing blue amp
01 00 00    | All off
02 00 00    | All off
04 00 00    | Blue power
08 00 00    | Blue power
10 00 00    | Flashing red power, I II III IV A B
20 00 00    | Red app
40 00 00    | Blue amp
80 00 00    | Blue amp and app

```
    0a 00 00 00 00 00 00       Orange power, amp, I II III IV A B
    0a 00 00 00 00 00 01       Blue amp
    0a 00 00 00 00 00 02       Flashing pink power
    0a 00 00 00 00 00 04       All off
    0a 00 00 00 00 01 00       Blue amp and app
    0a 00 00 00 00 02 00       Flashing red power
    0a 00 00 00 00 04 00       Flashing red power
    0a 00 00 00 00 80 00       Flashing blue amp
    0a 00 00 00 04 00 00       Blue power
    0a 00 00 00 08 00 00       Blue power
    0a 00 00 00 10 00 00       Flashing red power, I II III IV A B
    0a 00 00 00 20 00 00       Red app
    0a 00 00 00 40 00 00       Blue amp
    0a 00 00 00 80 00 00       Blue amp and app
```

## Start-up sequence
```
# Unknown - Spark II
Amp send:   15 00 00 00 01 01

# Get firmware version
Amp send:   0B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Pedal send: 0b 00 00 00 00 46 34 2e 31 2e 31 39 00
#                          F  4  .  1  .  1  9

# Get expression pedal inputs
Amp send:   0D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
Pedal send: 0d 00 00 00 00

# Get profile
Amp send:   08 00 00 00 
Pedal send: 08 00 00 00 01

# Unknown - Spark II
Amp send:   11 00 00 00

# Get battery level
Amp send:   1F 00 00 00
Pedal send: 1F 00 00 00 60

# Unknown - Spark II
Amp send:   10 00 00 00

# Unknown - Spark II
Amp send:   04 00 00 00 00 00 00 00 04 04 00 00 64
Amp send:   04 00 00 00 01 00 00 00 04 04 00 00 64
Amp send:   04 00 00 00 02 00 00 00 04 04 00 00 64
Amp send:   04 00 00 00 03 00 00 00 04 04 00 00 64
Amp send:   04 00 00 00 04 00 00 00 04 04 00 00 64

# Clear pedal lights
Amp send:   01 00 00 00 01 07 02 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 07 00 10 00 00 00 00 00

# Get pedal mappings (for profile 1)
Amp send:   14 00 00 00 01 
Pedal send: 14 00 00 00 01 00 01 0C 02 03 08 72 75

# Get profile name
Amp send:   12 00 00 00 01
Pedal send: 12 00 00 00 01 50 72 6F 66 69 6C 65 20 23 31 00 00 00 00 00 00 00 00 00 00
#                          P  r  o  f  i  l  e     #  1

# Set pedal lights
Amp send:   01 00 00 00 01 00 01 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 00 02 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 01 01 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 01 02 10 00 00 00 00 00
Amp send:   01 00 00 00 01 03 01 7F 00 00 FF 00 00 
Amp send:   01 00 00 00 01 03 02 7F 00 00 FF 00 00 
Amp send:   01 00 00 00 01 04 01 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 04 02 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 02 01 7F 00 00 FF 00 00 
Amp send:   01 00 00 00 01 02 02 7F 00 00 FF 00 00 
Amp send:   01 00 00 00 01 05 02 FF 00 FF FF FF 00 
Amp send:   01 00 00 00 01 05 01 10 00 00 00 00 00


```

## Setting and reading profile names

```
>>> 13 00 00 00 03 50 61 75 6C 20 50 72 6F 66 69 6C 65 00 00 00 00 00 00 00 00
                   P  a  u  l     P  r  o  f  i  l  e

>>> 12 00 00 00 03
    12 00 00 00 03 50 61 75 6C 20 50 72 6F 66 69 6C 65 00 00 00 00 00 00 00 00
                   P  a  u  l     P  r  o  f  i  l  e

>>> 13 00 00 00 03 50 72 6F 69 00
                   P  r  o  f

>>> 12 00 00 00 03
    12 00 00 00 03 50 72 6F 69 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

```
    12 00 00 00 00 47 6C 6F 62 61 6C 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                   G  l  o  b  a  l
    12 00 00 00 02 50 72 6F 66 69 6C 65 20 23 31 00 00 00 00 00 00 00 00 00 00 
                   P  r  o  f  i  l  e     #  1
    12 00 00 00 02 50 72 6F 66 69 6C 65 20 23 32 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  2
    12 00 00 00 02 50 72 6F 66 69 6C 65 20 23 33 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  3
    12 00 00 00 04 50 72 6F 66 69 6C 65 20 23 34 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  4
    12 00 00 00 05 50 72 6F 66 69 6C 65 20 23 35 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  5
    12 00 00 00 06 50 72 6F 66 69 6C 65 20 23 36 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  6
    12 00 00 00 07 50 72 6F 66 69 6C 65 20 23 37 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  7
    12 00 00 00 08 50 72 6F 66 69 6C 65 20 23 38 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  8
    12 00 00 00 09 4C 6F 6F 70 65 72 20 23 31 00 00 00 00 00 00 00 00 00 00 00
	           L  o  o  p  e  r     #  1
    12 00 00 00 0A 4C 6F 6F 70 65 72 20 23 32 00 00 00 00 00 00 00 00 00 00 00
	           L  o  o  p  e  r     #  2
    12 00 00 00 0B 06 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

```


## BLE UUIDs in use

The pedal has multiple UUIDs, but the key ones for connecting to the Spark LIVE are:    

Type           | UUID | Properties
---------------|------|------------
Service        | FFC8 |   
Characteristic | FFC9 | Write
Characteristic | FFCA | Read Notify


And for normal Spark Control app connection: 

Type           | UUID                                 | Properties
---------------|--------------------------------------|------------
Service        | 7bdb8dc0-6c95-11e3-981f-0800200c9a66 |   
Characteristic | 362f71a0-6c96-11e3-981f-0800200c9a66 | Read Notify

## Other messages

```
>>> 0f 00 00 00              
    0f 00 00 00   33
>>> 10 00 00 00 
    10 00 00 00   00
>>> 10 00 00 00 
    10 00 00 00   1f
>>> 11 00 00 00 
    11 00 00 00   00
>>> 11 00 00 00 
    11 00 00 00   01
>>> 15 00 00 00   01 01
>>> 04 00 00 00   00 00 00 00 04 04 00 00 64
>>> 04 00 00 00   01 00 00 00 04 04 00 00 64
>>> 04 00 00 00   02 00 00 00 04 04 00 00 64
>>> 04 00 00 00   03 00 00 00 04 04 00 00 64
>>> 04 00 00 00   04 00 00 00 04 04 00 00 64

```

## Button command setting trace (app and amp)   

### App writes to LIVE or Spark II      

Get preset    

```
Get preset 1	29A: 01 
Response	39A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 31   01          08   00 01 0C 02 03 08 72 75
#		           P  r  e  s  e  t     M  o  d  e     1    Profile 1        00 01 0C 02 03 08 72 75
```

Set a preset    
```
Set preset 1	19A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 31   01          08   00 01 0C 02 03 08 72 75
#                          P  r  e  s  e  t     M  o  d  e      1 
Response	49A
```

### Amp writes to Spark Control X    

```
13 0 0 0 1 50 72 65 73 65 74 20 4D 6F 64 65 20 31 0 0 0 0 0 0 0
#          P  r  e  s  e  t     M  o  d  e     1
3 0 0 0 1 1 80 1 5 1 0 
3 0 0 0 1 2 80 1 5 1 1 
3 0 0 0 1 3 80 1 5 1 C 
3 0 0 0 1 4 80 1 5 1 2 
3 0 0 0 1 5 80 1 5 1 3 
3 0 0 0 1 6 80 1 5 1 8 
3 0 0 0 1 7 80 1 5 1 72 
3 0 0 0 1 8 80 1 5 1 75 
```


## App to Amp to Spark Control X statup sequence

```
                296:
                438

                376: 78 04 04 C2 C3 C2 3C 
                396: C3

Get profile 1	29A: 01 
Response 	39A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 31         01   08   00 01 0C 02 03 08 72 75 

Get profile 2	29A: 02 
Response        39A: 0F AF 53 74 6F 6D 70 62 6F 78 20 4D 6F 64 65 20 31   02   08   10 11 14 12 13 15 72 75 

Get profile 3	29A: 03 
Response        39A: 0E AE 4D 75 73 69 63 2F 50 72 65 73 65 74 20 31      03   08   64 62 0C 63 60 30 72 75 

Get profile 4   29A: 04 
Response        39A: 0A AA 4D 75 73 69 63 2F 46 58 20 31                  04   08   64 12 09 13 15 08 72 75 

Get profile 5   29A: 05 
Response        39A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 32         05   08   00 01 0C 02 03 30 72 75 

Get profile 6   29A: 06 
Response        39A: 0F AF 53 74 6F 6D 70 62 6F 78 20 4D 6F 64 65 20 32   06   08   12 13 0C 14 15 08 72 75 

Get profile 7   29A: 07 
Response        39A: 0E AE 4D 75 73 69 63 2F 50 72 65 73 65 74 20 32      07   08   64 63 09 60 0C 08 72 75 

Get profile 8   29A: 08 
Response        39A: 0A AA 4D 75 73 69 63 2F 46 58 20 32                  08   08   64 63 13 60 12 15 72 75 

Get profile	29E:
Response        39E: 5

Get firmware    29D:
Response        39D: 13 B3 53 70 61 72 6B 58 20 76 34 2E 31 2E 31 39 20 62 61 65 33    0A AA 33 33 37 36 37 32 34 37 30 37     07 A7 46 34 2E 31 2E 31 39 24 0 0 
                #          S  p  a  r  k  X     v  4  .  1  .  1  9     b  a  e  3           3  3  7  6  7  2  4  7  0  7            F  4  .  1  .  1  9  $

		3A1: 01 C2 # false
		3A1: 02 C2 # false

                1A0: C2 # false 
                4A0:
                19E: 05

                49E:
                39E: 05 

                29C:
                39C: C2 # false 

                29C:
                39C: C2 # false

```

## Sending preset information   

```
Set profile 1   19A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 31         01   08   00 01 0C 02 03 08 72 75 
		#          P  r  e  s  e  t     M  o  d  e     1 
Response        49A:

Set profile 2   19A: 0F AF 53 74 6F 6D 70 62 6F 78 20 4D 6F 64 65 20 31   02   08   10 11 14 12 13 15 72 75 
		#	   S  t  o  m  p  b  o  x     M  o  d  e     1
Response        49A:

Set profile 3   19A: 0E AE 4D 75 73 69 63 2F 50 72 65 73 65 74 20 31      03   08   64 62 0C 63 60 30 72 75 
                #          M  u  s  i  c  /  P  r  e  s  e  t     1  
Response        49A:

Set profile 4   19A: 0A AA 4D 75 73 69 63 2F 46 58 20 31                  04   08   64 12 09 13 15 08 72 75
                #          M  u  s  i  c  /  F  X     1
Response        49A:

Set profile 5   19A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 32         05   08   00 01 0C 02 03 30 72 75 
                #          P  r  e  s  e  t     M  o  d  e     2
Response        49A:

Set profile 6   19A: 0F AF 53 74 6F 6D 70 62 6F 78 20 4D 6F 64 65 20 32   06   08   12 13 0C 14 15 08 72 75 
                #          S  t  o  m  p  b  o  x     M  o  d  e     2
                39E: 05 
Response        49A:

Set profile 7   19A: 0E AE 4D 75 73 69 63 2F 50 72 65 73 65 74 20 32      07   08   64 63 09 60 0C 08 72 75 
		#	   M  u  s  i  c  /  P  r  e  s  e  t     2 
Response        49A:

Set profile 8   19A: 0A AA 4D 75 73 69 63 2F 46 58 20 32                  08   08   64 63 13 60 12 15 72 75 
		#          M  u  s  i  c  /  F  X     2
Response        49A:

Set profile 5   19A: 0D AD 50 72 65 73 65 74 20 4D 6F 64 65 20 32         05   08   08 01 0C 02 03 30 72 75        
		#          P  r  e  s  e  t     M  o  d  e     2 
Response        49A:

Get profile     29C:
Response        39E: 05 

                39C: C2 # false
                29C:
                39C: C2 # false
                29C:
                39C: C2 # false
                
```
## HCI trace

```
root@paul-mainpc:/home/paul# bluetoothctl 
[bluetooth]# scan le
Discovery started
[CHG] Controller 5C:F3:70:AA:C4:6A Discovering: yes
[CHG] Device E3:BA:44:C9:E5:62 RSSI: -67
[CHG] Device E3:BA:44:C9:E5:62 ManufacturerData Key: 0x0122
[CHG] Device E3:BA:44:C9:E5:62 ManufacturerData Value:
  00 12                                            ..              
[NEW] Device 79:C4:9E:1B:71:9F 79-C4-9E-1B-71-9F
[NEW] Device 42:08:7B:A6:A8:6A 42-08-7B-A6-A8-6A
[NEW] Device 56:50:04:8F:29:05 56-50-04-8F-29-05
[bluetooth]# scan off
[bluetooth]# exit

root@paul-mainpc:/home/paul# gatttool -b E3:BA:44:C9:E5:62 -I -t random
[E3:BA:44:C9:E5:62][LE]> connect
Attempting to connect to E3:BA:44:C9:E5:62
Connection successful
[E3:BA:44:C9:E5:62][LE]> primary
attr handle: 0x0001, end grp handle: 0x0008 uuid: 00001801-0000-1000-8000-00805f9b34fb
attr handle: 0x0009, end grp handle: 0x000f uuid: 00001800-0000-1000-8000-00805f9b34fb
attr handle: 0x0010, end grp handle: 0x001e uuid: 0000180a-0000-1000-8000-00805f9b34fb
attr handle: 0x001f, end grp handle: 0x002b uuid: 34452f38-9e44-46ab-b171-0cc578feb928
attr handle: 0x002c, end grp handle: 0x002f uuid: 6facfe71-a4c3-4e80-ba5c-533928830727
attr handle: 0x0030, end grp handle: 0x0033 uuid: 5cb68410-6774-11e4-9803-0800200c9a66
attr handle: 0x0034, end grp handle: 0x004c uuid: 97a16290-8c08-11e3-baa8-0800200c9a66
attr handle: 0x004d, end grp handle: 0x0056 uuid: 7bdb8dc0-6c95-11e3-981f-0800200c9a66
attr handle: 0x0057, end grp handle: 0x005a uuid: 03b80e5a-ede8-4b33-a751-6ce34ec4c700
attr handle: 0x005b, end grp handle: 0x0060 uuid: 0000ffc0-0000-1000-8000-00805f9b34fb
attr handle: 0x0061, end grp handle: 0x006a uuid: 0000180f-0000-1000-8000-00805f9b34fb
attr handle: 0x006b, end grp handle: 0x0070 uuid: 0000ffc8-0000-1000-8000-00805f9b34fb
attr handle: 0x0091, end grp handle: 0x0094 uuid: 8d53dc1d-1db7-4cd3-868b-8a527460aa84
[E3:BA:44:C9:E5:62][LE]> characteristics 0x0061 0x070
handle: 0x0062, char properties: 0x12, char value handle: 0x0063, uuid: 00002a19-0000-1000-8000-00805f9b34fb
handle: 0x0065, char properties: 0x12, char value handle: 0x0066, uuid: 00002beb-0000-1000-8000-00805f9b34fb
handle: 0x0068, char properties: 0x12, char value handle: 0x0069, uuid: 00002bea-0000-1000-8000-00805f9b34fb
handle: 0x006c, char properties: 0x08, char value handle: 0x006d, uuid: 0000ffc9-0000-1000-8000-00805f9b34fb
handle: 0x006e, char properties: 0x12, char value handle: 0x006f, uuid: 0000ffca-0000-1000-8000-00805f9b34fb
[E3:BA:44:C9:E5:62][LE]> char-read-hnd 6f
Characteristic value/descriptor: 03 00 00 00 03 
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 1400000001
[E3:BA:44:C9:E5:62][LE]> char-read-hnd 6f
Characteristic value/descriptor: 14 00 00 00 01 00 01 0c 02 03 08 72 75
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 1200000001
[E3:BA:44:C9:E5:62][LE]> char-read-hnd 6f
Characteristic value/descriptor: 12 00 00 00 01 50 61 75 6c 21 21 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
[E3:BA:44:C9:E5:62][LE]> char-desc 61 70
handle: 0x0061, uuid: 00002800-0000-1000-8000-00805f9b34fb
handle: 0x0062, uuid: 00002803-0000-1000-8000-00805f9b34fb
handle: 0x0063, uuid: 00002a19-0000-1000-8000-00805f9b34fb
handle: 0x0064, uuid: 00002902-0000-1000-8000-00805f9b34fb
handle: 0x0065, uuid: 00002803-0000-1000-8000-00805f9b34fb
handle: 0x0066, uuid: 00002beb-0000-1000-8000-00805f9b34fb
handle: 0x0067, uuid: 00002902-0000-1000-8000-00805f9b34fb
handle: 0x0068, uuid: 00002803-0000-1000-8000-00805f9b34fb
handle: 0x0069, uuid: 00002bea-0000-1000-8000-00805f9b34fb
handle: 0x006a, uuid: 00002902-0000-1000-8000-00805f9b34fb
handle: 0x006b, uuid: 00002800-0000-1000-8000-00805f9b34fb
handle: 0x006c, uuid: 00002803-0000-1000-8000-00805f9b34fb
handle: 0x006d, uuid: 0000ffc9-0000-1000-8000-00805f9b34fb
handle: 0x006e, uuid: 00002803-0000-1000-8000-00805f9b34fb
handle: 0x006f, uuid: 0000ffca-0000-1000-8000-00805f9b34fb
handle: 0x0070, uuid: 00002902-0000-1000-8000-00805f9b34fb
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 70 0100
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 1400000001
Notification handle = 0x006f value: 14 00 00 00 01 00 01 0c 02 03 08 72 75 
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 1200000001
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 1400000001
Notification handle = 0x006f value: 14 00 00 00 01 00 01 0c 02 03 08 72 75 
[E3:BA:44:C9:E5:62][LE]> char-write-cmd 6d 080000
Notification handle = 0x006f value: 08 00 00 00 01 

```

