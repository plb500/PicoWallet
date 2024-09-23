# PicoWallet
Code for an BIP32/BIP44 HD (Hierarchical Deterministic) hardware wallet implementation on the Raspberry Pico microcontroller using the Pico C SDK.

Can generate full hierearchy of derived keys either by generating new seed (using Pico's on board entropy sources) or from hard-coded seed. 

Able to generate and output BIP39-compliant mnemonic phrases. Also generates QR codes for importing private keys and for public addresses

User display/input is done using a [Waveshare 1.44" SPI LCD](https://www.waveshare.com/wiki/Pico-LCD-1.44)

![Image of prototype PicoWallet system](/images/pico_wallet_prototype_working.jpeg)
![Screenshot of serial terminal connected to PicoWallet](/images/serial_screenshot.png)


TODO:
- Browse keys from UI
- Import keys?



