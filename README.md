# Tetris for the JY-MCU 3208 "Lattice Clock"

This is a Tetris-like falling blocks game for the JY-MCU 3208 "Lattice Clock" available from Deal
Extreme, Banggood etc.

The left button moves the current block left, the right button moves it right, and the centre
button rotates it. Holding the centre button drops the block quickly.

The block fall rate gradually increases over time.

There is basic scoring: 1 point is scored per completed row, with 4, 8 and 16 points awarded for
completing 2, 3 or 4 rows respectively. The score is shown once the game ends (when there is no
space for a new block on the board).

There's a short video of it in action here: https://www.youtube.com/watch?v=NIG7UfFYCdY

To program using a USPasp with `avrdude`, do:

```
$ sudo avrdude -p m8 -c usbasp -U flash:w:tetris.hex
```

To write the EEPROM, which currently just holds the high score, do:

```
$ sudo avrdude -p m8 -c usbasp -U eeprom:w:tetris.eep
```

This must be done the first time the program is loaded.

Copyright (C) Pete Hollobon 2015.

MIT Licenced.
