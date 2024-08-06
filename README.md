
## Messages from pedal

### Expression pedal insert

```
0d 00 00 00 00					No expression
0d 00 00 00 01					Expression 1 plugged in
0d 00 00 00 02					Expression 2 plugged in
```

Header       |  Value           
-------------|----------
0d 00 00 00  |  00 - 02

Value | Explanation
------|----------------------------
00    | No expression pedal
01    | Expression pedal in input 1
02    | Expression pedal in input 2

### Expression pedal value

```
0c 00 00 00   02   01 e0 0b 02 70 54			Expression value
```

Header       |  Pedal   | Value           
-------------|----------|------------------
0c 00 00 00  |  01 - 02 | 01 e0 0b 02 70 54



### Button press messages from pedal

```
03  00 00 00  FE
```

Header       |  Pedal           
-------------|------------------
03 00 00 00  |  00 - 15, FD, FE 



    
Type      |  **I**       |   **II**    |     **A**
-----------|----------|---------|-------
Bank 1     |  00      |   01    |     0C
Bank 2     |  10      |   11    |     14   
Long press |          |         |     FD  

Type      |  **III**     |  **IV**    |     **B**  
-----------|----------|---------|------- 
Bank 1     |  02      |   03    |     08   
Bank 2     |  12      |   13    |     15  
Long press |          |         |     FE 
    

## Messages to pedal

### Light messages to pedal

```
01 00 00 00 01  01  01  FF  FF  00 00 00  00
```

Header      | tbd |  Light   |  Bank      | Brightness | tbd | Blue    | Green   | Red     | tbd
------------|-----|----------|------------|------------|-----|---------|---------|---------|----
01 00 00 00 | 01  |  00 - 05 |   01 or 02 | 00 - FF    | 00  | 00 - FF | 00 - FF | 00 - FF | 00


### Lamp settings
                               0a 00 00 00 xx xx xx                             Lamps

## Start-up sequence
```
0B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

Sending response 
0b 00 00 00 00 46 34 2e 31 2e 31 39 00
			   F  4  .  1  .  1  9

0D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 


Sending response 
0d 00 00 00 00


08 00 00 00 

Sending response 
08 00 00 00 01

01 00 00 00 01 07 02 10 00 00 00 00 00 
01 00 00 00 01 07 00 10 00 00 00 00 00 
14 00 00 00 01 

Sending response 
14 00 00 00 03 00 01 0C 02 03 08 72 75

01 00 00 00 01 00 01 10 00 00 00 00 00 
01 00 00 00 01 00 02 10 00 00 00 00 00 
01 00 00 00 01 01 01 10 000 00 00 00 00 
01 00 00 00 01 01 02 10 00 00 00 00 00
01 00 00 00 01 03 01 7F 00 00 FF 00 00 
01 00 00 00 01 03 02 7F 00 00 FF 00 00 
01 00 00 00 01 04 01 10 00 00 00 00 00 
01 00 00 00 01 04 02 10 00 00 00 00 00 
01 00 00 00 01 02 01 7F 00 00 FF 00 00 
01 00 00 00 01 02 02 7F 00 00 FF 00 00 
01 00 00 00 01 05 02 FF 00 FF FF FF 00 
01 00 00 00 01 05 01 10 00 00 00 00 00
```
