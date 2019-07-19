----------------------------------------------------
components:

    uc.:                        nfr52832
    pressure/temperature:       mpl3115A2
    accelero:                   MMA8452QR1           
    ext.mem.                    S25FL064LABNFA040
    
                                                                                  WSON8   (5x6mm)
                                S25FL064LABNFM040       1200pcs - 20.1  - 64Mb  - USON8   (4x4mm) 
                                MT25QU128ABA1EW7-0SIT   3700pcs - 10ron - 128Mb - WDFN-8  (6x5mm)   -max 2V !!!
                                S25FL064LABNFA040       473       13.5  - 64Mb  - USON8   (4x4mm)

                                
----------------------------------------------------
sampling:

    10Hz. (0.1sec / sample)
    
    sample types:                                                  sampled at
        - (A) altitude (0.05m)    16bit    range: 0 .. 3000.00        10Hz     - 0.05m resolution uint16
        - (T) temperature (0.5*C)  8bit    range: -40 .. 85           0.5Hz    - 0.5*C with 40*C offset uint8
        - (C) ctrl_sig. (1%)       8bit    range: -120 .. 120         4.5Hz    - 1% resolution with 120 offset uint8
        - (G) accelero (n/a)       8bit    range: 0 .. 250            4.5Hz    - adimensional logarythmic value with the G measured on the compound force vector (TBD)
        - (B) battery              8bit    range: 0 .. 250            0.5Hz    - cell detection will be done, value range:  1 cell  - 2.85V..4.35V (6mV)  
                                                                                                                            2 cells - calculated 1 cell value will be saved

    sequence:
        |--------- second 1 ----------|--------- second 2 ----------|
        |AC.AG.AC.AG.AC.AG.AC.AG.AC.AT|AC.AG.AC.AG.AC.AG.AC.AG.AB.AG|
   
    data sequence (ds):
        [id1] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][C] [A][A][T]
        [id2] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][C] [A][A][G] [A][A][B] [A][A][G]
        
        31 bytes / second

----------------------------------------------------
data storage - page allocation:
    
    256byte pages:
        [pg.header][ds][ds][ds]....[crc16]
    
        [pg.header]:
            - [00]        page written marker (if 0xff - page is free)
              [sinf]      session info
              [ss][ss]    16bit sequence counter (32k pages - with 64k value we can detect the start/end of sequence

            session info:
                1bit      1st. page recorded from power on
                1bit      last page recorded before power off
                3bits     number of recorded ds                
              
        [ds] x 8    31x8 bytes of payload     (8 seconds)
              
        [res][res]  reserved

        [crc][crc]  16 bit page CRC
        
        
            
----------------------------------------------------
memory blocks:

S25FL 064 L AB NFA040
- t page prog:  (256bytes)  450..1350us 
- t byte prog:   1st byte   75.. 90   us
- sector erase:  4k         65..320   ms
- 1/2 block er.  32k        300..600  ms
- block erase:   64k        450..1150 ms 
- c.erase:                  55 .. 150 sec



b0  -
b1  -  swappable blocks NV functional data
b2  -
b3  -  redundant mirror for b0/b1

b4 .. bN  - data log area

- memory array is deleted completely at 1st. start-up
- memory is written 1 page at a time
- if 4 pages till next undeleted block - delete it. (operation max. 600ms, page can hold 8 sec. recordings)

TODO: diag. if deletion is finished before power down.










