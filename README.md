# PicoWallet
Code for an BIP32/BIP44 HD (Hierarchical Deterministic) hardware wallet implementation on the Raspberry Pico microcontroller using the Pico C SDK.

Can generate full hierearchy of derived keys either by generating new seed (using Pico's on board entropy sources) or from hard-coded seed. 

Able to generate and output BIP39-compliant mnemonic phrases. Also generates QR codes for importing private keys and for public addresses

TODO:
- Screen/button input (currently everything is just dumped out the serial port)
- Import keys?

![Screenshot of serial terminal connected to PicoWallet](/images/serial_screenshot.png)
