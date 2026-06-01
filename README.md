# ProjectSTM32Bousole

## Presentation

Ce projet est une boussole numerique pour carte `NUCLEO-L152RE` avec shield `Yncrea2`.

Le programme lit le magnetometre `LIS2MDL`, calcule un cap entre `0` et `359` degres, puis affiche une direction lisible sur l'afficheur 7 segments pilote par `MAX7219`. L'accelerometre `LIS2DW12` sert a detecter une inclinaison trop forte.

Le firmware utilise aussi une entree analogique `ADC1_IN0` sur `PA0`, un buzzer, une barre de 8 LEDs, le bouton bleu de la carte et une sortie serie `USART2` pour les traces `printf`.

## Materiel

| Element | Role |
|---|---|
| `NUCLEO-L152RE` | Carte de developpement |
| `STM32L152RET6` | Microcontroleur ARM Cortex-M3 |
| Shield `Yncrea2` | Shield pedagogique avec capteurs, LEDs, buzzer et afficheur |
| `LIS2MDL` | Magnetometre, mesure du champ magnetique |
| `LIS2DW12` | Accelerometre, detection d'inclinaison |
| `MAX7219` | Driver de l'afficheur 7 segments 4 digits |
| Barre LEDs `L0` a `L7` | Indication du secteur de direction |
| Buzzer `BZ1` | Signal sonore proche du nord |
| Bouton bleu `B1` | Demande d'affichage temporaire de l'ADC |
| `PA0` | Entree analogique de diagnostic |

## Utilisation

1. Ouvrir, compiler et flasher le projet avec la configuration `Debug` dans STM32CubeIDE.
2. Garder la carte aussi horizontale que possible.
3. Orienter le segment bleu vers la direction a mesurer.
4. Lire la direction sur l'afficheur 7 segments.

Affichages principaux :

```text
N000
E090
S180
W270
```

La lettre indique le secteur cardinal pointe par le segment bleu. Les degres correspondent au meme cap utilisateur.

Le buzzer emet un bip quand le cap affiche se rapproche du nord :

| Ecart au nord | Comportement |
|---:|---|
| `> 20` degres | Pas de bip |
| `6` a `20` degres | Bip court et lent |
| `0` a `5` degres | Bip plus long et plus frequent |

La barre de LEDs indique le secteur de direction courant. Une seule LED est allumee en fonctionnement normal.

Le bouton bleu affiche temporairement la valeur ADC lue sur `PA0` sous la forme `Axxx`. La valeur affichee est normalisee de `A000` a `A999` a partir de la mesure brute ADC 12 bits `0` a `4095`. Apres environ `1,2 s`, l'affichage revient automatiquement a la boussole.

## Messages Speciaux

| Message | Signification | Reaction du firmware |
|---|---|---|
| `INIT` | Demarrage en cours | Initialisation afficheur et capteurs |
| `TILT` | Carte trop inclinee | Cap ignore, buzzer coupe, LEDs extremes |
| `ERR` | Magnetometre absent ou lectures echouees | Buzzer coupe, LEDs eteintes |
| `Axxx` | Valeur ADC normalisee | Affichage temporaire apres appui sur `B1` |

## Compilation Et Flash

Le projet est prevu pour STM32CubeIDE.

1. Ouvrir le dossier du projet dans STM32CubeIDE.
2. Selectionner la configuration `Debug`.
3. Compiler puis flasher la carte.

Depuis le terminal, avec le toolchain STM32CubeIDE dans le `PATH` :

```sh
cd Debug
make all
```

Firmware genere :

```text
Debug/ProjectSTM32Bousole.elf
```

Note pratique : si le flash echoue avec le shield branche, debrancher le shield, lancer le flash, puis rebrancher le shield apres le demarrage du programme.

## Configuration CubeMX

| Donnee | Valeur |
|---|---|
| Fichier CubeMX | `ProjectSTM32Bousole.ioc` |
| Carte | `NUCLEO-L152RE` |
| MCU | `STM32L152RETx` / `STM32L152RET6` |
| Boitier | `LQFP64` |
| Firmware package | `STM32Cube FW_L1 V1.10.6` |
| Version CubeMX | `6.16.1` |
| Toolchain cible | `STM32CubeIDE` |
| Compilateur | `GCC` |
| Stack | `0x400` |
| Heap | `0x200` |

Peripheriques declares dans le `.ioc` :

| IP CubeMX | Utilisation |
|---|---|
| `RCC` | Horloge systeme |
| `SYS` | SysTick, SWD, SWO |
| `NVIC` | Interruptions systeme et EXTI |
| `USART2` | Port serie de debug |

`ADC1`, `TIM6`, l'I2C logiciel et le SPI logiciel sont initialises dans le code utilisateur. Ils ne sont pas declares comme IP CubeMX dediees, mais ils sont bien utilises par le firmware.

## Horloges

| Horloge | Valeur |
|---|---:|
| Source PLL | `HSI` a `16 MHz` |
| PLLMUL | `RCC_PLL_MUL6` |
| PLLDIV | `RCC_PLL_DIV3` |
| `SYSCLK` | `32 MHz` |
| `HCLK` / AHB | `32 MHz` |
| `PCLK1` / APB1 | `32 MHz` |
| `PCLK2` / APB2 | `32 MHz` |
| Frequence timers APB1/APB2 | `32 MHz` |
| SysTick | Base de temps HAL `1 ms` |

Le fichier `.ioc` contient aussi les pins oscillateur `PC14`, `PC15`, `PH0` et `PH1`, mais le code genere utilise l'oscillateur interne `HSI` comme source de PLL.

## Table Complete Des Pins Du IOC

| Pin | Label IOC / Code | Signal | Mode electrique | Utilisation |
|---|---|---|---|---|
| `PA0-WKUP1` | `myADC` | `GPIO_Analog` | Analogique, no pull | Entree `ADC1` canal 0 |
| `PA2` | `USART_TX` | `USART2_TX` | AF push-pull, no pull, very high speed | TX debug ST-LINK |
| `PA3` | `USART_RX` | `USART2_RX` | AF push-pull, no pull, very high speed | RX debug ST-LINK |
| `PA5` | `SEG_SCK` | `GPIO_Output` | Push-pull, high speed | Horloge SPI logiciel `MAX7219` |
| `PA7` | `SEG_MOSI` | `GPIO_Output` | Push-pull, high speed | Donnees SPI logiciel `MAX7219` |
| `PA8` | `SEG_CS` | `GPIO_Output` | Push-pull, high speed | Chip select `MAX7219`, actif bas |
| `PA13` | `TMS` | `SYS_JTMS-SWDIO` | Debug SWD | Programmation/debug |
| `PA14` | `TCK` | `SYS_JTCK-SWCLK` | Debug SWD | Programmation/debug |
| `PB1` | `L0` | `GPIO_Output` | Push-pull, low speed | LED secteur 0 |
| `PB2` | `L1` | `GPIO_Output` | Push-pull, low speed | LED secteur 1 |
| `PB3` | `SWO` | `SYS_JTDO-TRACESWO` | Trace SWO | Debug trace |
| `PB8` | `I2C_SCL` | `GPIO_Output` | Open-drain, pull-up, high speed | SCL I2C logiciel |
| `PB9` | `I2C_SDA` | `GPIO_Output` | Open-drain, pull-up, high speed | SDA I2C logiciel |
| `PB10` | `L2` | `GPIO_Output` | Push-pull, low speed | LED secteur 2 |
| `PB11` | `L3` | `GPIO_Output` | Push-pull, low speed | LED secteur 3 |
| `PB12` | `L4` | `GPIO_Output` | Push-pull, low speed | LED secteur 4 |
| `PB13` | `L5` | `GPIO_Output` | Push-pull, low speed | LED secteur 5 |
| `PB14` | `L6` | `GPIO_Output` | Push-pull, low speed | LED secteur 6 |
| `PB15` | `L7` | `GPIO_Output` | Push-pull, low speed | LED secteur 7 |
| `PC7` | `BZ1` | `GPIO_Output` | Push-pull, high speed | Buzzer |
| `PC13-WKUP2` | `B1 [Blue PushButton]` | `GPIO_EXTI13` | Entree interruption, no pull | Bouton bleu |
| `PC14-OSC32_IN` | Oscillateur LSE | `RCC_OSC32_IN` | LSE external oscillator | Reserve horloge basse frequence |
| `PC15-OSC32_OUT` | Oscillateur LSE | `RCC_OSC32_OUT` | LSE external oscillator | Reserve horloge basse frequence |
| `PH0-OSC_IN` | Oscillateur HSE | `RCC_OSC_IN` | HSE external clock source | Reserve horloge externe |
| `PH1-OSC_OUT` | Oscillateur HSE | `RCC_OSC_OUT` | Sortie HSE | Reserve horloge externe |
| `VP_SYS_VS_Systick` | `SysTick` | Virtuel | Interruption systeme | Base de temps HAL |

Niveaux initiaux principaux :

| Signal | Niveau au demarrage |
|---|---|
| `SEG_SCK`, `SEG_MOSI` | `0` |
| `SEG_CS` | `1` |
| `BZ1` | `0` |
| `I2C_SCL`, `I2C_SDA` | `1` |
| `L0` a `L7` | `0` |

## Peripheriques Et Parametres

| Peripherique | Configuration | Role |
|---|---|---|
| `USART2` | `115200 bauds`, `8N1`, pas de controle de flux, oversampling `16` | `printf` vers le port COM ST-LINK |
| `ADC1` | Canal `0` sur `PA0`, resolution 12 bits, conversion logiciel par `SWSTART` | Diagnostic analogique affiche par `B1` |
| `TIM6` | Horloge `32 MHz`, `PSC = 32000 - 1`, `ARR = 1000 - 1` | Interruption periodique de demande ADC |
| GPIO LEDs | `PB1`, `PB2`, `PB10` a `PB15` en sortie push-pull | Barre de 8 LEDs |
| GPIO buzzer | `PC7` en sortie push-pull | Bip proche du nord |
| `EXTI13` | Front montant sur `PC13` | Bouton bleu |
| I2C logiciel | `PB8` SCL, `PB9` SDA, open-drain avec pull-up | `LIS2MDL` et `LIS2DW12` |
| SPI logiciel | `PA5` SCK, `PA7` MOSI, `PA8` CS | `MAX7219` |

Calcul de TIM6 :

```text
F_TIM6 = 32 000 000 Hz
PSC    = 32 000 - 1
ARR    = 1 000 - 1
T_IRQ  = (PSC + 1) * (ARR + 1) / F_TIM6
       = 32 000 * 1 000 / 32 000 000
       = 1 s
```

La lecture ADC est demandee dans `TIM6_IRQHandler`, mais la conversion est faite dans la boucle principale pour garder l'interruption courte.

## Interruptions

| IRQ | Priorite | Declenchement | Handler | Travail fait |
|---|---:|---|---|---|
| `SysTick_IRQn` | `0` | Toutes les `1 ms` | `SysTick_Handler()` | `HAL_IncTick()` |
| `EXTI15_10_IRQn` | `1` | Front montant `PC13 / EXTI13` | `EXTI15_10_IRQHandler()` | Efface `EXTI_PR_PR13`, appelle `App_ButtonIrqHandler()` |
| `TIM6_IRQn` | `2` | Update event TIM6 toutes les `1 s` | `TIM6_IRQHandler()` | Efface `TIM_SR_UIF`, appelle `App_Timer6IrqHandler()` |

Priorite NVIC globale : `NVIC_PRIORITYGROUP_0`.

`App_ButtonIrqHandler()` applique un anti-rebond logiciel de `200 ms`, puis demande l'affichage ADC. `App_Timer6IrqHandler()` incremente un compteur de ticks TIM6 et une demande de lecture ADC.

## Capteurs I2C

Les deux capteurs sont pilotes par I2C logiciel sur `PB8/PB9`.

| Capteur | Adresse 7 bits | Registre test | Valeur attendue | Configuration |
|---|---:|---:|---:|---|
| `LIS2MDL` | `0x1E` | `WHO_AM_I = 0x4F` | `0x40` | `CFG_REG_A = 0x8C`, `CFG_REG_B = 0x02`, `CFG_REG_C = 0x10` |
| `LIS2DW12` | `0x19` | `WHO_AM_I = 0x0F` | `0x44` | `CTRL6 = 0x00`, `CTRL1 = 0x50` |

Le `LIS2MDL` fournit les axes magnetiques `X/Y/Z`. Le cap est calcule a partir de `X` et `Y`, puis corrige pour correspondre au segment bleu.

Le `LIS2DW12` fournit les axes accelerometre `X/Y/Z`. Si `abs(X)` ou `abs(Y)` depasse `1400` en brut, la carte est consideree trop inclinee et l'afficheur indique `TILT`.

## Afficheur MAX7219

Le `MAX7219` est pilote par SPI logiciel :

| Signal | Pin |
|---|---|
| `SCK` | `PA5` |
| `MOSI` | `PA7` |
| `CS` | `PA8` |

Configuration au demarrage :

| Registre MAX7219 | Valeur | Effet |
|---|---:|---|
| `DISPLAY_TEST` (`0x0F`) | `0x00` | Test afficheur desactive |
| `SCAN_LIMIT` (`0x0B`) | `0x03` | 4 digits utilises |
| `DECODE` (`0x09`) | `0x00` | Decodage BCD desactive, segments pilotes par table |
| `INTENSITY` (`0x0A`) | `0x08` | Luminosite moyenne |
| `SHUTDOWN` (`0x0C`) | `0x01` | Afficheur active |

## Fonctionnement Logiciel

1. `HAL_Init()` initialise le HAL et SysTick.
2. `SystemClock_Config()` configure `SYSCLK = 32 MHz`.
3. `MX_GPIO_Init()` configure toutes les pins du `.ioc`.
4. `MX_USART2_UART_Init()` initialise la sortie serie.
5. `ADC1_AppInit()` active et configure `ADC1`.
6. `TIM6_AppInit()` active TIM6 et son interruption.
7. `Compass_AppInit()` initialise le `MAX7219`, le `LIS2MDL` et le `LIS2DW12`.
8. La boucle principale appelle `Compass_AppTask()` en continu.

Dans `Compass_AppTask()` :

1. Un appui bouton affiche `Axxx`.
2. Une demande TIM6 lance une lecture ADC.
3. Toutes les `150 ms`, le firmware lit le magnetometre.
4. Si l'accelerometre detecte une inclinaison trop forte, l'affichage passe a `TILT`.
5. Sinon, le cap utilisateur est affiche, les LEDs sont mises a jour et le buzzer est gere.

## Donnees De Debug Serie

La sortie serie passe par `USART2` sur les pins ST-LINK de la NUCLEO :

```text
115200 bauds, 8 bits, no parity, 1 stop bit
```

Exemples de traces :

```text
Boussole: MAX7219 OK, LIS2MDL OK, LIS2DW12 accel OK
Cap affiche: 090 deg [E], ecart nord affiche 090 deg
Inclinaison trop forte: mesure du cap ignoree
```

## Fichiers Principaux

| Fichier | Role |
|---|---|
| `ProjectSTM32Bousole.ioc` | Configuration CubeMX et pinout |
| `Core/Src/main.c` | Logique applicative, ADC, TIM6, bus logiciels, capteurs, affichage |
| `Core/Inc/main.h` | Defines de pins et prototypes exportes |
| `Core/Src/stm32l1xx_it.c` | Handlers `SysTick`, `EXTI15_10`, `TIM6` |
| `Core/Src/stm32l1xx_hal_msp.c` | Initialisation MSP, UART, horloges systeme |
| `STM32L152RETX_FLASH.ld` | Script linker Flash |
| `STM32L152RETX_RAM.ld` | Script linker RAM |
| `FICHE_TECHNIQUE.md` | Fiche technique synthetique du projet |
| `Debug/ProjectSTM32Bousole.elf` | Firmware compile en configuration Debug |

## Contraintes Respectees

| Contrainte | Implementation |
|---|---|
| GPIO LED et bouton | LEDs `L0-L7`, bouton `B1` sur `PC13` |
| Timer | `TIM6`, interruption toutes les `1 s` |
| ADC | `ADC1` canal 0 sur `PA0` |
| UART | `USART2` pour `printf` |
| SPI ou I2C | SPI logiciel `MAX7219`, I2C logiciel `LIS2MDL/LIS2DW12` |
| Deux interruptions | `TIM6_IRQn` et `EXTI15_10_IRQn` |
| Deux capteurs shield | `LIS2MDL` et `LIS2DW12` |
| README exploitable | Pinout, IRQ, horloges, peripheriques et usage documentes |
