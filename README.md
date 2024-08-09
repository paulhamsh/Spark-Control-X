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

## Pedal overview (LIVE mode)

The Sark Control X has 6 buttons which are programmed to send 6 different BLE messages to the amp (LIVE mode).   
Internally, there are 8 banks which the pedal can be in.    
Long pressing A increases the bank, Long pressing B decreases the bank.   
They are numbered 1 to 8. A long press of A when the bank is 8 has no effect. A long press of B when the bank is 7 has no effect.      
When connected to the LIVE amp, the amp requests the bank number.  
The Spark Control X then sends the current bank.   
The amp then requests the values that each button sends.  
The pedal sends this as a single response message.     
When the amp receives the button press value, it responds with series of messages to turn the relevant leds on or off, and to set their colours. 


## Messages from pedal

### Spark control messages

Written to characteristic 362f71a0-6c96-11e3-981f-0800200c9a66   
The value below when pressed, and a 00 when released   
If multiple buttons pressed, then the values are summed and sent as one message. They represent one bit position each.   
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



### Expression pedal insert

```
0d 00 00 00 00					No expression
0d 00 00 00 01					Expression 1 plugged in
0d 00 00 00 02					Expression 2 plugged in
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

FORMAT NOT UNDERSTOOD YET

```
0c 00 00 00   01   01 e0 0b 02 70 54			Expression value

0c000000 01983702ffff
0c000000 01e84602ffff
0c000000 01b05202ffff
0c000000 01405a02ffff

0c000000 01781302a801
0c000000 017813029000
0c000000 01781302a801
0c000000 017813029800
0c000000 01781302c001
0c000000 017813029000
0c000000 01781302a001

```

Header       |  Pedal   | Value           
-------------|----------|------------------
0c 00 00 00  |  01 - 02 | 01 e0 0b 02 70 54



### Button press messages from pedal

```
03  00 00 00  01
```

Header       |  Pedal           
-------------|------------------
03 00 00 00  |  00 - 15, FD, FE 



    
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
Effect | Long A  | Switch to Tone
Effect | Long B  | Switch to Tone


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
