# Arduino core dla PIC18F25K80 (SDCC + gputils)

Platforma Arduino (direct „hardware/") dla mikrokontrolera **Microchip PIC18F25K80**
(28-pin DIP). Kompilatorem jest **SDCC** (C), assemblerem/linkerem **gputils**,
a dzięki `arduino-cc` (opakowaniu na recepty `platform.txt`) całość działa wprost
z **Arduino CLI** oraz **Arduino IDE 2.x**.

> Uwaga: SDCC nie obsługuje C++ — zarówno skecze jak i core musza byc pisane w C.
> Pliki `.ino` kompiluja sie (sa przetwarzane jako C przez `arduino-cc`), ale nie
> ma obiektów, `String` itp.

---

## Status

| Etap | Stan |
|---|---|
| Kompilacja C → HEX (SDCC → gpasm → gplink) | ✔ działa |
| Core Arduino (GPIO, ADC, PWM, UART, czas, przerwania) | ✔ działa |
| Pełna integracja z `arduino-cli` (instalacja + compile) | ✔ działa |
| Ładowanie do układu (upload) | ⚠ stub — brak zintegrowanego programatora |

## Wymagane narzędzia

Narzędzia potrzebne **tylko do lokalnej budowy/rozwoju** (instalacja przez
Board Manager pobiera gotowy toolchain z platformą):

- **SDCC 4.5.x** (budowa z `-mpic16`)
- **gputils 1.4.x** (gpasm / gplink / gplib)
- **arduino-cli** (testowany 1.5.2) lub **Arduino IDE 2.x**
- Python 3 (do regeneracji biblioteki SFR i `release.sh`)

Ścieżki narzędzi konfiguruje się zmiennymi środowiskowymi (z domyślną wartością
wskazującą katalog roboczy projektu):

| Zmienna | Domyślnie | Co ustawia |
|---|---|---|
| `SDCC_BIN` | `/tmp/opencode/sdcc/bin` | katalog z `sdcc` |
| `GPUTILS_BIN` | `/tmp/opencode/gputils-deb/usr/bin` | katalog z `gpasm`/`gplink`/`gplib` |
| `SDCC_LIB` | `/tmp/opencode/sdcc/share/sdcc/lib/pic16` | biblioteki `libsdcc.lib`, `crt0iz.o` itd. |
| `DEV_LIB` | `<platforma>/toolchain/lib` | własne biblioteki (SFR, shim) |
| `LKR_FILE` | `<platforma>/toolchain/lkr/18f25k80_g.lkr` | plik linkerowy |
| `GPUTILS_INC_DIR` | `/tmp/opencode/gputils-1.4/header` | nagłówki p18f25k80.inc (auto-budowa shima) |

## Szybki start

### 0. Instalacja przez Arduino Board Manager (zalecane)

W Arduino IDE: **File → Preferences → Additional boards manager URLs**,
wklej:

```
https://raw.githubusercontent.com/marcin-filipiak/arduino_pic18f25k80/main/package_pic18f25k80_index.json
```

a następnie **Tools → Board → Boards Manager** → wyszukaj **PIC18F25K80** → Install.

W Arduino CLI:

```bash
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/marcin-filipiak/arduino_pic18f25k80/main/package_pic18f25k80_index.json
arduino-cli core update-index
arduino-cli core install kolgreen:pic18f25k80
```

Instalacja pobiera i rozpakowuje platformę wraz z **pełnym, osadzonym
toolchainem** (SDCC + gputils dla Linux x86_64) — nic więcej nie trzeba
instalować ręcznie. FQBN: `kolgreen:pic18f25k80:pic18f25k80`.

> Uwaga: plik `package_pic18f25k80_index.json` generuje skrypt
> `release.sh` (pakuje platformę + liczy SHA-256). Po zmianach w core
> wykonaj `./release.sh`, zaktualizuj wersję w `platform.txt` i wypchnij
> do repozytorium.

### 1. Instalacja lokalna (bez Board Manager)

```bash
./install.sh            # domyślnie instaluje do ~/Arduino/hardware/kolgreen/pic18f25k80
./install.sh /sciezka/do/katalogu/uzytkownika   # albo własny katalog
```

Platforma pojawia się jako `kolgreen:pic18f25k80:pic18f25k80`
(nazwa w menu: **PIC18F25K80 (28-pin)**).

### 2. Test kompilacji

```bash
arduino-cli compile -b kolgreen:pic18f25k80:pic18f25k80 examples/Blink
arduino-cli compile -b kolgreen:pic18f25k80:pic18f25k80 examples/SerialAin
```

Obraz wynikowy: `build/<skecz>.ino.hex` (Intel HEX, ładowany pod adres 0).

### 3. Test bez arduino-cli (tylko narzędziownia)

```bash
./toolchain/build.sh examples/Blink/Blink.c   # → examples/Blink/Blink.hex
```

## Mapowanie pinów

Numeracja: **pin = port×8 + bit** (port 0 = PORTA, 1 = PORTB, 2 = PORTC).

| Arduino | AVR | Funkcja | Arduino | AVR | Funkcja |
|---|---|---|---|---|---|
| 0 | RA0 | cyfrowy / AN0 / A0 | 13 | RB5 | **LED_BUILTIN** |
| 1 | RA1 | cyfrowy / AN1 / A1 | 14 | RB6 | cyfrowy |
| 2 | RA2 | cyfrowy / AN2 / A2 | 15 | RB7 | cyfrowy |
| 3 | RA3 | cyfrowy / AN3 / A3 | 16 | RC0 | cyfrowy |
| 4 | RA4 | cyfrowy | 17 | RC1 | cyfrowy |
| 5 | RA5 | cyfrowy / AN4 / A4 | 18 | RC2 | cyfrowy / **PWM (CCP1/P1A)** |
| 6 | RA6 | cyfrowy | 19 | RC3 | cyfrowy |
| 7 | RA7 | cyfrowy | 20 | RC4 | cyfrowy |
| 8 | RB0 | INT0 / AN10 / A5 | 21 | RC7 | **RX (EUSART1)** |
| 9 | RB1 | INT1 / AN8 / A6 | 22 | RC6 | **TX (EUSART1)** |
| 10 | RB2 | INT2 | | | |
| 11 | RB3 | cyfrowy | | | |
| 12 | RB4 | cyfrowy / AN9 / A7 | | | |

Przerwania zewnętrzne: `attachInterrupt(pin, func, mode)` obsługuje tylko piny
8 (INT0), 9 (INT1), 10 (INT2).

## Obsługa peryferiów

- **ADC** — 12-bitowy (wynik 0..4095), `analogRead(A0..A7)`; cyfry: 0–3, 5, 8, 9, 12.
  Vref+ = VDD, Vref− = VSS.
- **PWM** — tylko pin 18 (RC2/CCP1), `analogWrite(pin, 0..255)`; częstotliwość
  `Fosc/(4·256)` ≈ 15,6 kHz @ 16 MHz.
- **UART1** (EUSART1) — TX = pin 22 (RC6), RX = pin 21 (RC7). Obsługiwane
  prędkości przy 16 MHz: 4800, 9600, 19200, 38400, 57600 (115200 ma zbyt duży błąd).
- **Czas** — Timer0 16-bit, preload 0xF060, przerwanie co 1 ms (`millis()`
  działa). `micros()` ≈ 1 µs rozdzielczość.
- **Shift** — `shiftIn` / `shiftOut` (bit-bang).

## API (kolejkowo Cz-dla SDCC)

Standardowe: `pinMode`, `digitalWrite`, `digitalRead`, `analogRead`,
`analogWrite`, `millis`, `micros`, `delay`, `delayMicroseconds`,
`shiftIn`, `shiftOut`, `attachInterrupt`, `detachInterrupt`,
makra `min/max/abs/constrain/map?` i bity (`bitRead`, `bitSet`, ...), `HIGH/LOW`,
`INPUT/OUTPUT/INPUT_PULLUP`, `RISING/FALLING/CHANGE`.

Serial (bez obiektow — funkcje globalne `uart1_*`):

| Funkcja | Odpowiednik Arduino |
|---|---|
| `uart1_begin(baud)` | `Serial.begin()` |
| `uart1_write(c)` | `Serial.write()` |
| `uart1_print(s)` | `Serial.print(s)` |
| `uart1_println(s)` | `Serial.println(s)` |
| `uart1_print_int(v)` / `uart1_print_ulong(v)` | `Serial.print(liczba)` |
| `uart1_read()` / `uart1_available()` | `Serial.read()` / `Serial.available()` |

## Konfiguracja układu (bity CONFIG)

Domyślna konfiguracja wpisana jest w `cores/pic18f25k80/Arduino.c`:

```c
#pragma config FOSC=INTIO2, INTOSCSEL=LOW, SOSCSEL=DIG, RETEN=ON
#pragma config WDTEN=OFF
#pragma config BORPWR=HIGH, BBSIZ=BB1K, MSSPMSK=MSK7, CANMX=PORTC
#pragma config XINST=OFF
```

- Zegar wewnętrzny **16 MHz (INTIO2)**, wyłączony watchdog, **XINST=OFF**
  (wymagane przez SDCC). Zmiana zegara wymaga edycji `F_CPU` w
  `boards.txt`/`platform.txt` oraz wartości `TMR0_PRELOAD` w `cores/.../isr.c`.
- Config ląduje w pamięci konfiguracyjnej (pocz. 0x300000) w HEX.

## Struktura katalogu

```
pic18f25k80/
├── boards.txt            # definicja płytki (FQBN kolgreen:pic18f25k80:pic18f25k80)
├── platform.txt          # recepty kompilacji (używa tools/arduino-cc)
├── programmers.txt       # programatory (picprog — stub)
├── install.sh            # instalacja platformy do katalogu użytkownika Arduino
├── cores/pic18f25k80/    # implementacja core (C):
│   ├── Arduino.h/.c      #   API globalne, init(), setup()/loop()
│   ├── wiring.c          #   czas (Timer0), delay/millis a.m.in
│   ├── isr.c             #   jeden ISR high-priority (0x08) + dispatcher
│   ├── wiring_digital.c  #   GPIO (TRIS/LAT/PORT)
│   ├── wiring_analog.c   #   ADC 12-bit, PWM CCP1
│   ├── wiring_shift.c    #   shiftIn/shiftOut
│   ├── interrupt.c       #   INT0..INT2
│   └── serial.c          #   uart1_* (EUSART1)
├── variants/pic18f25k80/pins_arduino.h   # mapowanie pinów
├── toolchain/
│   ├── build.sh          # niskopoziomowy build C → HEX (bez arduino-cli)
│   ├── gen_sfr_lib.sh    # regeneracja libdev18f25k80.lib z p18f25k80.inc
│   ├── pic18f25k80.h     # nagłówek C: 453 × extern __at(...) __sfr
│   ├── pic18f25k80_bits.h# stałe bitowe rejestrów
│   ├── lkr/18f25k80_g.lkr
│   └── lib/
│       ├── libdev18f25k80.lib   # symbole SFR (absolutne UDATA sekcje)
│       └── gptr_shim.S          # obejście buga SDCC 4.5 (eeprom_gptr)
└── tools/
    ├── arduino-cc        # opakowanie: logika recept z platform.txt
    ├── arduino-cli       # (opcjonalnie, pobrany do testów)
    └── picprog-upload    # STUB — upload do uzupełnienia
```

## Jak to działa (narzędziownia)

`arduino-cc` tłumaczy recepty `platform.txt` na wywołania narzędzi:

```
C/C (.ino, .cpp, .c)
   │ sdcc -S -mpic16 -p18f25k80 --std-c99
   ▼
   .asm
   │ gpasm -c -p18f25k80
   ▼
   .o  ──►  core.a  (gplib)
   │
   ▼  gplink crt0iz.o + gptr_shim.o + libio/libdev/libsdcc/libc18f
   .hex
```

Kluczowe założenia:

- **SFR jako symbole:** SDCC generuje odwołania jak `_LATB`. gputils 1.4 nie
  pozwala zdefiniować absolutnego symbolu globalnego przez `EQU` w kodzie
  relokowalnym (Error[156]). Rozwiązanie — jedna **absolutna sekcja UDATA** na
  adres rejestru (0xE41..0xFFF, pokryte przez `PROTECTED DATABANK` w .lkr),
  `GLOBAL _Nazwa` + `_Nazwa:` + `RES 1`. Regeneracja: `gen_sfr_lib.sh`.
- **Jeden ISR na priorytet:** SDCC (pic16) pozwala tylko na jedno przerwanie na
  priorytet. Cały core gości w pojedynczym `void _isr(void) __interrupt(1)`
  (wektor 0x08, high-priority), który rozdziela Timer0 / INT0 / INT1 / INT2.
- **C-only:** `recipe.cpp.o.pattern` kieruje też `.cpp`/`.ino` do kompilatora C.
- **Bug SDCC 4.5.0:** obiekty `gptrget*.o`/`gptrload.o` w `libsdcc.lib`
  odwołują się do nieistniejących już nazw `___eeprom_gptrget1..4` /
  `___eeprom_gptrload` (zostały podzielone na `eeprom8`/`eeprom16`). Biblioteka
  `toolchain/lib/gptr_shim.S` przekazuje je (ta sama konwencja rejestrów) do
  `___eeprom8_gptr*`. Shim buduje się automatycznie przy linkowaniu, gdy go
  brakuje.

## Znane ograniczenia

- ❌ Brak C++ (SDCC); skecze implementowane w C (mimo `.ino`).
- ❌ Brak `String`, `bool` (używać `_Bool`/`unsigned char`), brak `Serial` jako
  obiektu — są funkcje `uart1_*`.
- ⚠ `analogWrite` tylko na pinie 18 (CCP1); inne piny ignorowane.
- ⚠ `analogRead` zwraca 12-bit (0..4095), nie 10-bit jak klasyczne AVR.
- ⚠ Ładowanie HEX do układu wymaga wypełnienia `tools/picprog-upload`
  (obecnie stub). Docelowo integracja z **PICprog** (Dekunukem) przez ICSP.
- ~Ostrzeżenia `warning 110` („conditional flow changed by optimizer") z gpasm
  w GPIO nie blokują budowy.
- Zegar i prędkości UART dobierane pod **16 MHz**.