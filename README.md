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

In ```Adurino/libraries/NimBLE-Arduino/src/NimBLEDevice.cpp```

Comment out the PUBLIC line and add in the RANDOM line.   

```
uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_RANDOM;
//uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_PUBLIC;
```

DO NOT FORGET TO CHANGE THIS BACK BEFORE COMPILING ANOTHER PROGRAM!!!   

## The Spark Control X hardare pedal

The Spark Control X has:
- six buttons
- two expression pedals (inputs)
- six lights (one for each button)
- power, amp and app connection lights

## Spark Control mode

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

```
I    II   III  IV
01   04   02   08
```  

## LIVE mode

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
- what is the current 'bank'
- waht is the bank name
- which button sends which message

It actions these messages:
- set light RGB values

Each configuration of the pedal has a set of six messages to send to the amp, each message assigned to a button.   
There are eight configuration 'banks' stored by the pedal    

Long pressing A increases the bank, Long pressing B decreases the bank.   
The banks are numbered 1 to 8. A long press of A when the bank is 8 has no effect. A long press of B when the bank is 7 has no effect.    
There seem to be two banks for the looper - unsure of what these do.    


The amp recognises various button messages as either:   
- select a preset
- change to next preset up
- change to second set of presets
- effect on/off
- change to next / previous bank

The amp controls the lights on the pedal.   To do this it sends a message to change the RGB value for that light position. It needs to know which light corresponds to each pedal message, and this is sent from the pedal in a separate message.   

The sequence is:
- amp asks pedal for current bank
- pedal responds (1-8)
- amp asks pedal for button message order (same as mapping message to light location)
- pedal responds
- amp sends messages to change lamp colours and brightness (two messages per light, not clear yet why)


## Table of messages

### Unidirectional messages

Direction       | Message number | Details
----------------|----------------|------------------------------------------
To amp          | 0x03           | Button press message
To amp          | 0x0c           | Expression pedal values
To amp          | 0x0d           | Expression pedal cable inserted / removed
To pedal        | 0x01           | Set button lamp colours
To pedal        | 0x0a           | Set other lamp colours (unused?)
To pedal        | 0x0b           | Get firmware version
To pedal        | 0x13           | Set bank profile name

### Request / response messages

Direction       | Message number | Details
----------------|----------------|------------------------------------------
To pedal        | 0x0b           | Get firmware version
Respond to amp  | 0x0b           | Firmware version
To pedal        | 0x0d           | Get expression pedal status
Respond to amp  | 0x0d           | Expression pedal cable status (as above)
To pedal        | 0x08           | Get current bank
Respond to amp  | 0x08           | Current bank
To pedal        | 0x12           | Get bank name
Respond to amp  | 0x12           | Bank name (eg Profile #1, Looper #1)
To pedal        | 0x14           | Get bank message layout
Respond to amp  | 0x14           | Bank message layout

Note: 
- 0x12 doesn't seem to be used currently       
  
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
0C            | Change preset selection (red / green)
10            | Toggle Gate
11            | Toggle Comp / Wah
12            | Toggle Drive
13            | Toggle Mod / EQ
14            | Toggle Delay
15            | Toggle Reverb
FD            | Bank up
FE            | Bank down
F9            | Looper ?
FA            | Looper ?
FB            | Looper ?
FC            | Looper ?
42            | Looper ?
44            | Looper ?
48            | Looper ?


#### Standard button / message layout
    
Type       |  **I**       |   **II**    |     **A**
-----------|--------------|-------------|-------
Bank 1     |  00          |   01        |     0C
Bank 2     |  10          |   11        |     14   
Long press |              |             |     FD  

Type       |  **III**     |  **IV**     |     **B**  
-----------|--------------|-------------|------- 
Bank 1     |  02          |   03        |     08   
Bank 2     |  12          |   13        |     15  
Long press |              |             |     FE 

Default pedal configurations for two banks - Tone and Effect   

Mode   | Control  |  Effect
-------|---------|--------------------------
Tone   | I       | Select tone 1 (or 5)
Tone   | II      | Select tone 2 (or 6)
Tone   | III     | Select tone 3 (or 7)
Tone   | IV      | Select tone 4 (or 8)
Tone   | A       | Change bank (green / red)
Tone   | B       | Cycle up the bank (0 to 3)
Tone   | Long A  | Switch to Effect
Tone   | Long B  | Switch to Effect
Effect | I       | Toggle Gate
Effect | II      | Toggle Comp / Wah
Effect | III     | Toggle Drive
Effect | IV      | Toggle Mod / EQ
Effect | A       | Toggle Delay
Effect | B       | Toggle Reverb
Effect | Long A  | Bank up
Effect | Long B  | Bank down

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


### Bank configuration mapping (0x14)

The bank configuration response defines which button sends which message       

```
                   I    II   A     III  IV   B
14 00 00 00 03     00   01   0C    02   03   08    72 75
```

Header       |  Bank    | I     | II    | A     | III   |IV     | B     |tbd | tbd         
-------------|----------|-------|-------|-------|-------|-------|-------|----|----
14 00 00 00  |  00      | FF    | FF    | FD    | FF    | FF    | FE    | FF | FF
14 00 00 00  |  01      | 00    | 01    | 0C    | 02    | 03    | 08    | 72 | 75
14 00 00 00  |  02      | 10    | 11    | 14    | 12    | 13    | 15    | 72 | 75
14 00 00 00  |  09      | FF    | 42    | 0C    | 44    | 48    | 08    | FF | FF
14 00 00 00  |  0A      | FC    | FB    | 0C    | FA    | F9    | 08    | FF | FF

A special Bank 0 has the mapping of long press messages. These are the same long presses regardless of bank selected. Bank 0 cannot be selected.     

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
# Get firmware version
Amp send:   0B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Pedal send: 0b 00 00 00 00 46 34 2e 31 2e 31 39 00
#                          F  4  .  1  .  1  9

# Get expression pedal inputs
Amp send:   0D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
Pedal send: 0d 00 00 00 00

# Get bank
Amp send:   08 00 00 00 
Pedal send: 08 00 00 00 01

# Clear pedal lights
Amp send:   01 00 00 00 01 07 02 10 00 00 00 00 00 
Amp send:   01 00 00 00 01 07 00 10 00 00 00 00 00

# Get pedal mappings (for bank 1)
Amp send:   14 00 00 00 01 
Pedal send: 14 00 00 00 01 00 01 0C 02 03 08 72 75

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

## Setting and reading bank profile names

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
    12 00 00 00 01 0C 08 04 02 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  

    12 00 00 00 02 50 72 6F 66 69 6C 65 20 23 32 00 00 00 00 00 00 00 00 00 00
                   P  r  o  f  i  l  e     #  3
    12 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

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
>>> 0f 00 00 00 03             
    0f 00 00 00 33

>>> 10 00 00 00 03
    10 00 00 00 00
>>> 11 00 00 00 03
    11 00 00 00 00


