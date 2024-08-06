


 
    
Type       |  1       |  2      |     3
-----------|----------|---------|-------
Label      |  I       |   II    |     A
Bank 1     |  00      |   01    |     0C
Bank 2     |  10      |   11    |     14   
Long press |          |         |     FD   
Long press |          |         |     FE   
Bank 2     |  12      |   13    |     15   
Bank 1     |  02      |   03    |     08   
Label      |  III     |   IV    |     B   

    

Light messages to pedal

```
01 00 00 00 01  01  01  FF  FF  00 00 00  00
```

Header          |  Light   |  Bank      | Brightness |  tbd | Blue    | Green   | Red     | tbd
----------------|----------|------------|------------|------|---------|---------|---------|----
01 00 00 00 01  |  00 - 05 |   01 or 02 | 00 - FF    | 00   | 00 - FF | 00 - FF | 00 - FF | 00
