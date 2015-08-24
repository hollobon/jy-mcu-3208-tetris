# Tetris for the JY-MCU 3208 "Lattice Clock"

This is a Tetris-like falling blocks game for the JY-MCU 3208 "Lattice Clock" available from Deal
Extreme, Banggood etc.

The left button moves the current block left, the right button moves it right, and the centre
button rotates it. Holding the centre button drops the block quickly.

The block fall rate gradually increases over time.

At the moment there is no scoring.

To program using a USPasp with `avrdude`, do:

```
$ sudo avrdude -p m8 -c usbasp -U flash:w:tetris.hex
```

Copyright (C) Pete Hollobon 2015.

MIT Licenced.
