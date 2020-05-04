<h1 align="center">DECODE Proximity embedded</br>
  <sub>Implementing DP-3T (embedded nRF52 version)</sub>
</h1>

<p align="center">
  <a href="https://dyne.org">
    <img src="https://img.shields.io/badge/%3C%2F%3E%20with%20%E2%9D%A4%20by-Dyne.org-blue.svg" alt="Dyne.org">
  </a>
</p>

<br><br>


This project is a privacy-preserving and decentralized alternative to the Dutch
`#coronaApp`.

It will adhere to the technological demands published last week in the
[‘Veilig tegen corona’ manifesto](https://www.veiligtegencorona.nl/).

The decode-proximity provides virus tracking and a warning system with a
proximity alert. Every device broadcasts its proximity to other devices,
privately recording when two people have been close (only the device owners
know!).

If a user gets an alert from the device they can request a test for a corona
diagnosis. If positive they can scan a QRCode from the GGD to anonymously
upload a notice for all other users that have been close to them, to let them
know of the risk.

❗ Please note: there are NO personal data stored on the central server. The
secret key and the ephemeral ids can NOT be traced to identifiable people.

We are also developing a hardware port (This repository) to allow usage as a
keyring and without a mobile phone

Using RIOT-OS+nimble on Nordic nRF52.

Using wolfSSL/wolfCrypt for TLS and crypto.

🚧 This software is in **ALPHA stage** and are part of the
[DECODE project](https://decodeproject.eu) about data-ownership and
[technological sovereignty](https://www.youtube.com/watch?v=RvBRbwBm_nQ). Our
effort is that of improving people's awareness of how their data is processed
by algorithms, as well facilitate the work of developers to create along
[privacy by design principles](https://decodeproject.eu/publications/privacy-design-strategies-decode-architecture)
using algorithms that can be deployed in any situation without any change.


<details id="toc">
 <summary><strong>🚩 Table of Contents</strong> (click to expand)</summary>

* [Install](#-install)
* [Quick start](#-quick-start)
* [Configuration](#-configuration)
* [Hardware](#-hardware)
* [Testing](#-testing)
* [Troubleshooting & debugging](#-troubleshooting--debugging)
* [Acknowledgements](#-acknowledgements)
* [Links](#-links)
* [Contributing](#-contributing)
* [License](#-license)
</details>

***
## 💾 Install

Some dependencies are needed before you are able to build the project.

### Dependencies

You need to clone the RIOT-OS project
[More details](https://doc.riot-os.org/getting-started.html).

Once you cloned the repo you have to reference it in the `RIOTBASE` variable of
the
[Makefile](https://github.com/dyne/decode-proximity-hw/blob/master/Makefile#L8)


The GNU Arm Embedded Toolchain is also needed available on most operative
systems packaging systems eg.

```bash
apt install gcc-arm-none-eabi
```
### Building

To build the project jus run

```bash 
make
```
This will download all the submodule needed projects and compile.

When finished you'll find the binary application file under `./bin/nrf52dk/dp3t.elf`

### Flashing on device

```bash
make flash
```

***
## 🎮 Quick start

 > TODO

***
## 🔧 Configuration

> TODO

***

## 📟 Hardware

Below a brief list supported and tested hardware to begin with

**nRF52 DK** :heavy_check_mark:

<img height=80 alt="nRF52 DK" src="https://www.nordicsemi.com/-/media/Images/Products/DevKits/nRF52-Series/nRF52-DK/nRF52-DK.png?h=530&la=en&mw=350&w=350&hash=48D397A047A718030261F80211DC881F8EFEA55D">

https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52-DK

**Arduino NANO 33 IoT** :heavy_check_mark:

<img height=80 alt="nRF52 DK" src="https://store-cdn.arduino.cc/uni/catalog/product/cache/1/image/500x375/f8876a31b63532bbba4e781c30024a0a/a/b/abx00027_iso_1.jpg">

https://store.arduino.cc/arduino-nano-33-iot

**Pinetime** :white_check_mark:

<img height=80 alt="nRF52 DK" src="https://www.cnx-software.com/wp-content/uploads/2019/09/PineTime.jpg">

https://www.pine64.org/pinetime/

***

## 📋 Testing

***
## 🐛 Troubleshooting & debugging

***
## 😍 Acknowledgements

Copyright © 2020 by [Dyne.org](https://www.dyne.org) foundation, Amsterdam

Designed, written and maintained by Daniele Lacamera

**RIOT-OS+nimble** on Nordic nRF52.

**wolfSSL/wolfCrypt** for TLS and crypto.

***
## 🌐 Links

https://dyne.org/

https://www.decodeproject.eu/


***
## 👤 Contributing

Please first take a look at the [Dyne.org - Contributor License Agreement](CONTRIBUTING.md) then

1.  🔀 [FORK IT](../../fork)
2.  Create your feature branch `git checkout -b feature/branch`
3.  Commit your changes `git commit -am 'Add some fooBar'`
4.  Push to the branch `git push origin feature/branch`
5.  Create a new Pull Request
6.  🙏 Thank you


**[🔝 back to top](#toc)**

***
## 💼 License
    DECODE Proximity embedded
    Copyright (c) 2020 Dyne.org foundation, Amsterdam

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc., 59
    Temple Place, Suite 330, Boston, MA 02111-1307 USA

**[🔝 back to top](#toc)**

