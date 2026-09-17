# Arduino core ``ardupic`` (SDCC + gputils)

Platforma Arduino (direct „hardware/") dla mikrokontrolerów **Microchip PIC**:

| Płytka | Rdzeń | Architektura | Flash | RAM | ADC |
|---|---|---|---|---|---|
| **PIC18F25K80** (28-pin) | `pic18f25k80` | pic16 (SDCC `-mpic16`) | 32 kB | 2 kB | 12-bit, A0..A7 |
| **PIC18F2550** (28-pin) | `pic18f2550` | pic16 | 32 kB | 2 kB | 10-bit, A0..A10 |
| **PIC18F4550** (40-pin) | `pic18f4550` | pic16 | 32 kB | 2 kB | 10-bit, A0..A13 |
| **PIC16F877A** (40-pin) | `pic16f877a` | pic14 (SDCC `-mpic14`) | 8 kB | 368 B | 10-bit, A0..A7 |

Kompilatorem jest **SDCC** (C), assemblerem/linkerem **gputils**, a dzięki
`arduino-cc` (opakowaniu na recepty `platform.txt`) całość działa wprost z
**Arduino CLI** oraz **Arduino IDE 2.x**.

> Uwaga: SDCC nie obsługuje C++ — zarówno skecze jak i core muszą być pisane w C.
> Pliki `.ino` kompilują się (są przetwarzane jako C przez `arduino-cc`), ale nie
> ma obiektów, `String` itp.

---

## Status

| Etap | Stan |
|---|---|
| Kompilacja C → HEX (SDCC → gpasm → gplink) | ✔ działa |
| Core Arduino (GPIO, ADC, PWM, UART, czas, przerwania) | ✔ działa |
| Pełna integracja z `arduino-cli` (instalacja + compile) | ✔ działa |
| Ładowanie do układu (upload) | ⚠ stub — brak zintegrowanego programatora |

## Szybki start

### 0. Instalacja przez Arduino Board Manager (zalecane)

W Arduino IDE: **File → Preferences → Additional boards manager URLs**, wklej:

```
https://raw.githubusercontent.com/marcin-filipiak/ardupic/main/package_ardupic_index.json
```

a następnie **Tools → Board → Boards Manager** → wyszukaj **ardupic** → Install.

W Arduino CLI:

```bash
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/marcin-filipiak/ardupic/main/package_ardupic_index.json
arduino-cli core update-index
arduino-cli core install marcinfilipiak:ardupic
```

Instalacja pobiera i rozpakowuje platformę wraz z **pełnym, osadzonym
toolchainem** (SDCC + gputils dla Linux x86_64) — nic więcej nie trzeba
instalować ręcznie. FQBN: `marcinfilipiak:ardupic:<board>`.

> Uwaga: plik `package_ardupic_index.json` generuje skrypt `release.sh`
> (pakuje platformę + liczy SHA-256). Po zmianach w core wykonaj
> `./release.sh`, zaktualizuj wersję w `platform.txt` i wypchnij do repozytorium.

### 1. Instalacja lokalna (bez Board Manager)

```bash
./install.sh            # domyślnie instaluje do ~/Arduino/hardware/marcinfilipiak/ardupic
./install.sh /sciezka/do/katalogu/uzytkownika   # albo własny katalog
```

### 2. Test kompilacji

```bash
arduino-cli compile -b marcinfilipiak:ardupic:pic18f25k80  examples/Blink
arduino-cli compile -b marcinfilipiak:ardupic:pic18f2550   examples/Blink
arduino-cli compile -b marcinfilipiak:ardupic:pic18f4550   examples/SerialAin
arduino-cli compile -b marcinfilipiak:ardupic:pic16f877a   examples/SerialAin
```

Obraz wynikowy: `build/<skecz>.ino.hex` (Intel HEX, ładowany pod adres 0).

## Mapowanie pinów

Numacja: **pin = port×8 + bit** (port 0 = PORTA, 1 = PORTB, 2 = PORTC, ...).
Np. RC6 → 2×8+6 = 22.

### PIC18F25K80 (28-pin)

| Arduino | mikrokontroler | Funkcja | Arduino | mikrokontroler | Funkcja |
|---|---|---|---|---|---|
| 0 | RA0 | AN0/A0 | 13 | RB5 | **LED_BUILTIN** |
| 1 | RA1 | AN1/A1 | 14 | RB6 | cyfrowy |
| 2 | RA2 | AN2/A2 | 15 | RB7 | cyfrowy |
| 3 | RA3 | AN3/A3 | 16 | RC0 | cyfrowy |
| 4 | RA4 | cyfrowy | 17 | RC1 | cyfrowy |
| 5 | RA5 | AN4/A4 | 18 | RC2 | **PWM (CCP1/P1A)** |
| 6 | RA6 | cyfrowy | 19 | RC3 | cyfrowy |
| 7 | RA7 | cyfrowy | 20 | RC4 | cyfrowy |
| 8 | RB0 | INT0/AN10/A5 | 21 | RC7 | **RX** |
| 9 | RB1 | INT1/AN8/A6 | 22 | RC6 | **TX** |
| 10 | RB2 | INT2 | 25 | RD6 | cyfrowy |
| 11 | RB3 | AN9/A7 | 26 | RD7 | cyfrowy |
| 12 | RB4 | cyfrowy | | | |

### PIC18F2550 (28-pin)

| Arduino | mikrokontroler | Funkcja | Arduino | mikrokontroler | Funkcja |
|---|---|---|---|---|---|
| 0 | RA0 | AN0/A0 | 9 | RB1 | INT1/AN10/A6 |
| 1 | RA1 | AN1/A1 | 10 | RB2 | INT2/AN8/A7 |
| 2 | RA2 | AN2/A2 | 11 | RB3 | AN9/A8 |
| 3 | RA3 | AN3/A3 | 12 | RB4 | AN11/A9 |
| 5 | RA5 | AN4/A4 | 13 | RB5 | AN13/A10 /**
| 8 | RB0 | INT0/AN12/A5 | 18 | RC2 | **PWM (CCP1/P1A)** |

TX = 22 (RC6), RX = 23 (RC7). Wewnętrzny oscylator **8 MHz**.

### PIC18F4550 (40-pin)

Piny 0..6 = RA0..RA6, 8..15 = RB0..RB7, 16..23 = RC0..RC7,
24..31 = RD0..RD7, 32..34 = RE0..RE2. RA7 nie istnieje (guard w API).

Analog: A0..A4 = RA0..RA3/RA5, A5..A7 = RE0..RE2 (AN5..AN7),
A8=RB2/AN8, A9=RB3/AN9, A10=RB1/AN10, A11=RB4/AN11, A12=RB0/AN12,
A13=RB5/AN13. TX = 22, RX = 23. Wewnętrzny oscylator **8 MHz**.

### PIC16F877A (40-pin)

Piny 0..5 = RA0..RA5, 8..15 = RB0..RB7, 16..23 = RC0..RC7,
24..31 = RD0..RD7, 32..34 = RE0..RE2. RA6/RA7 to piny kryształu (OSC1/OSC2) —
nieużywalne. Analog A0..A3 = RA0..RA3, A4 = RA5 (AN4), A5..A7 = RE0..RE2
(AN5..AN7). TX = 22 (RC6), RX = 23 (RC7). Oscylator **zworka XT 4 MHz**.

## Obsługa peryferiów

- **ADC** — `analogRead(A0..A?)`: **12-bit (0..4095)** na K80, **10-bit (0..1023)**
  na 2550/4550/877A. Vref+ = VDD, Vref− = VSS. Podczas odczytu włączany jest
  tylko wybrany kanał ADC (kontrola PCFG/ADCON1), potem wracają piny cyfrowe.
- **PWM** — tylko pin 18 (CCP1/ECCP1/P1A), `analogWrite(pin, 0..255)`;
  częstotliwość `Fosc/(4·256)`.
- **UART1** — TX = pin 22 (RC6), RX = pin 23 (RC7). Funkcje globalne `uart1_*`
  (patrz niżej). Prędkości: 4800, 9600, 19200, 38400, 57600.
- **Czas** — `millis()` co 1 ms (Timer0 16-bit na PIC18, Timer1 na PIC16F877A);
  `micros()` z rozdzielczością ~0,5 µs (PIC18) / 1 µs (877A).
- **Shift** — `shiftIn` / `shiftOut` (bit-bang).

## API

Standardowe (wzorowane na Wiring, ale w C): `pinMode`, `digitalWrite`,
`digitalRead`, `analogRead`, `analogWrite`, `millis`, `micros`, `delay`,
`delayMicroseconds`, `shiftIn`, `shiftOut`, `attachInterrupt`,
`detachInterrupt`, makra `min/max/abs/constrain/radians/degrees/round/sq`
i bitowe (`bitRead`, `bitSet`, `bitClear`, `lowByte`, `highByte`, `bit`),
stałe `HIGH/LOW`, `INPUT/OUTPUT/INPUT_PULLUP`, `RISING/FALLING/CHANGE`.

Przerwania zewnętrzne:

| Płytka | Piny |
|---|---|
| PIC18F25K80 / 2550 / 4550 | 8 (INT0), 9 (INT1), 10 (INT2) |
| PIC16F877A | 8 (INT0, jedyne) |

Serial (bez obiektów — funkcje globalne `uart1_*`):

| Funkcja | Odpowiednik Arduino |
|---|---|
| `uart1_begin(baud)` | `Serial.begin()` |
| `uart1_write(c)` | `Serial.write()` |
| `uart1_print(s)` | `Serial.print(s)` |
| `uart1_println(s)` | `Serial.println(s)` |
| `uart1_print_int(v)` / `uart1_print_ulong(v)` | `Serial.print(liczba)` |
| `uart1_read()` / `uart1_available()` | `Serial.read()` / `Serial.available()` |

## Konfiguracja układu (bity CONFIG)

Siedziba w `cores/<board>/Arduino.c`:

- **PIC18F25K80**: `FOSC=INTIO2` → wewn. 16 MHz, `WDTEN=OFF`, `XINST=OFF`.
- **PIC18F2550 / 4550**: `FOSC=INTOSCIO_EC` → wewn. 8 MHz (RA6/RA7 jako I/O),
  `PWRT=ON`, `BOR=OFF`, `WDT=OFF`, `LVP=OFF`, `XINST=OFF`, `PBADEN=OFF` — przez
  `#pragma config` (gpasm zna nazwy z `gpcfg-table.c`).
- **PIC16F877A**: `__code unsigned int __at(0x2007) __CONFIG = 0x3FF1`
  (XT, WDT off, LVP off) — SDCC pic14 nie obsługuje `#pragma config`.

Zmiana zegara wymaga edycji `F_CPU` w `boards.txt` i preloadu timera w isr.c.

## Struktura katalogu

```
ardupic/
├── boards.txt            # definicje płytek (4 FQBN)
├── platform.txt          # recepty kompilacji (używa tools/arduino-cc)
├── programmers.txt       # programatory (picprog — stub)
├── install.sh            # instalacja do katalogu użytkownika Arduino
├── release.sh            # buduje paczkę zip + package_ardupic_index.json
├── cores/
│   ├── pic18f25k80/      # PIC18 (pic16) @ 16 MHz
│   ├── pic18f2550/       # PIC18 (pic16) @ 8 MHz
│   ├── pic18f4550/       # PIC18 (pic16) @ 8 MHz
│   └── pic16f877a/       # PIC16 (pic14) @ 4 MHz
├── variants/<board>/pins_arduino.h
├── toolchain/
│   ├── sdcc/  +  pic14/  # kompilator i biblioteki pic14
│   ├── gputils/          # gpasm / gplink / gplib
│   ├── gen_sfr_lib.sh    # regeneracja libdev<device>.lib
│   ├── <device>.h        # nagłówki SFR (extern __at(...) __sfr)
│   ├── <device>_bits.h   # stałe bitowe
│   ├── lkr/<device>_g.lkr
│   ├── header/           # *.inc dla gpasm
│   └── lib/              # libdev*.lib, gptr_shim.S
└── tools/
    ├── arduino-cc        # opakowanie: logika recept z platform.txt
    ├── arduino-cli       # (opcjonalnie, do testów)
    └── picprog-upload    # STUB — upload do uzupełnienia
```

## Jak to działa (narzędziownia)

`arduino-cc <mode> <DEVICE> ...` tłumaczy recepty `platform.txt` na wywołania
narzędzi (DEVICE np. `PIC18F2550` / `PIC16F877A` — z niego wyliczane są nazwy
procesora dla SDCC/gpasm/gplink):

```
C/C (.ino, .cpp, .c)
   │ sdcc -S -m<pic16|pic14> -p<device> --std-c99
   ▼
   .asm
   │ gpasm -c -p<device> (-I header/)
   ▼
   .o  ──►  core.a  (gplib)
   │
   ▼  gplink  <lkr> + crt0 (pic16) + libdev + libsdcc + libc...
   .hex
```

Na PIC16F877A (pic14) biblioteki: `libsdcc.lib` (startup + `__interrupt`),
`libc.lib`, `pic16f877a.lib` (SDCC non-free device lib). Dołączany jest
nagłówek SFR `pic16f877a.h` (SDCC non-free → `--no-warn-non-free`).

Kluczowe techniki:

- **SFR jako symbole (pic16):** SDCC generuje odwołania jak `_LATB`; gputils 1.4
  nie pozwala na globalny `EQU` w kodzie relokowalnym (Error[156]). Rozwiązanie:
  jedna **absolutna sekcja UDATA** na adres rejestru, pokryta przez
  `PROTECTED DATABANK` w `.lkr`; regeneracja: `gen_sfr_lib.sh <device>`.
- **Jeden ISR na priorytet (pic16):** pojedyncze `void _isr(void) __interrupt(1)`
  (wektor 0x08) rozdziela Timer0 / INT0 / INT1 / INT2. Pic14: pojedynczy
  `__interrupt(0)` (wektor 0x04), Timer1 / INT0.
- **C-only:** `recipe.cpp.o.pattern` kieruje też `.cpp`/`.ino` do C.
- **Bug SDCC 4.5.0:** obiekty z `libsdcc.lib` odwołują się do nieistniejących
  już nazw `___eeprom_gptr*` — obejście w `toolchain/lib/gptr_shim.S`
  (przekierowanie na `___eeprom8_gptr*`). Budowany automatycznie przy linku.

## Znane ograniczenia

- ❌ Brak C++ (SDCC); skecze w C (mimo `.ino`).
- ❌ Brak `String`, `bool` (używać `_Bool`/`unsigned char`), brak obiektu
  `Serial` — funkcje `uart1_*`.
- ⚠ `analogWrite` tylko na pinie 18 (CCP1); inne piny ignorowane.
- ⚠ Rozdzielczość ADC zależy od układu: 12-bit na K80, 10-bit na reszcie.
- ⚠ PIC16F877A nie ma rejestrów LAT — zapis do PORT (ryzyko RMW na PORTA/B).
- ⚠ Ładowanie HEX do układu wymaga wypełnienia `tools/picprog-upload`
  (obecnie stub). Docelowo integracja z **PICprog** (Dekunukem) przez ICSP.
- ~Ostrzeżenia `warning 110` („conditional flow changed by optimizer") z gpasm
  w GPIO nie blokują budowy.
- Zegary: K80 16 MHz (wewn.), 2550/4550 8 MHz (wewn.), 877A 4 MHz (XT).