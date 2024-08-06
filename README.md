
## Expression pedal insert

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

## Expression pedal value
                               0c 00 00 00 02 01 e0 0b 02 70 54			Expression value




## Button press messages from pedal

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
    

## Light messages to pedal

```
01 00 00 00 01  01  01  FF  FF  00 00 00  00
```

Header      | tbd |  Light   |  Bank      | Brightness | tbd | Blue    | Green   | Red     | tbd
------------|-----|----------|------------|------------|-----|---------|---------|---------|----
01 00 00 00 | 01  |  00 - 05 |   01 or 02 | 00 - FF    | 00  | 00 - FF | 00 - FF | 00 - FF | 00


## Lamp settings
                               0a 00 00 00 xx xx xx                             Lamps
