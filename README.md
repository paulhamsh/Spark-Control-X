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

## Pedal overview - Spark Control mode

Spark Control mode is where the APP connects to the pedal     

When a button is pressedm the pedal writes a value to characteristic 362f71a0-6c96-11e3-981f-0800200c9a66   
When the button is released, ```0``` is written to characteristic 362f71a0-6c96-11e3-981f-0800200c9a66    
Each button is represented as a bit position, so multiple buttons are summed (or bitwise OR) together and sent as a single value to the characteristic.    

Example:
```
Press I and II         03
Press III and A        0C
Press IV and B         30
```

  **I**       |   **II**    |     **A**
--------------|-------------|----------
  01          |   02        |     04

  **III**     |  **IV**     |     **B**  
--------------|-------------|---------- 
  08          |   10        |     20   

For reference, the original Spark Control sends these values

```
I    II   III  IV
01   04   02   08
```  

## Pedal overview - LIVE mode

LIVE mode is where the AMP connects directly to the pedal, without any app involvement    

The Spark Control X has:
- six buttons
- two expression pedals (inputs)
- six lights (one for each button)
- power, amp and app connection lights

Button presses and expression pedal changes generate BLE messages from the pedal to the amp    
The amp sends messages to the pedal to control the lights    
The amp can also send request messages to the pedal to obtain its configuration    

The Spark Control X sends these messages:
- button press
- expression pedal cable insert / removal
- expression pedal value

It responds to these requests:
- firmware id    
- current 'bank'
- which button sends which message

It actions these messages:
- set light RGB values

Each configuration of the pedal has a set of six messages to send to the amp, each message assigned to a button.   
There are eight configuration 'banks' stored by the pedal    

Long pressing A increases the bank, Long pressing B decreases the bank.   
They are numbered 1 to 8. A long press of A when the bank is 8 has no effect. A long press of B when the bank is 7 has no effect.      

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



## Messages from pedal

### Button press messages from pedal


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


```
03  00 00 00  01
```

Header       |  Pedal           
-------------|------------------
03 00 00 00  |  00 - 15, FD, FE 


Banks are laid out in the order I, II, A, III, IV, B    
The bank configuration response (described below) defines which button sends which message    

```
                  I   II  A     III IV  B
14 00 00 00 03    00  01  0C    02  03  08   72 75
```


    
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

### Expression pedal insert

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

### Expression pedal value

This message shows the value of each expression pedal, in the range 0x0000 to 0xffff

Header       |  Pedal 1 | Value pedal 1 | Pedal 2 | Value pedal 2         
-------------|----------|---------------|---------|--------------
0c 00 00 00  |  01      | e0 0b         | 02      | 70 54


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

### Light messages to pedal

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

### Lamp settings

```
                               0a 00 00 00 xx xx xx                             Lamps
```

### Get pedal mapping

```
# Get pedal mappings
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

# Get pedal mappings
Amp send:   14 00 00 00 01 
Pedal send: 14 00 00 00 03 00 01 0C 02 03 08 72 75

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
