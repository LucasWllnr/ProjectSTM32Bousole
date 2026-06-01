# Fiche Technique - ProjectSTM32Bousole

## Objectif

Le projet realise une boussole numerique embarquee sur STM32. La direction pointee par le segment bleu est affichee sur 4 digits sous la forme :

```text
N000
E090
S180
W270
```

La lettre, les degres, les LEDs et le buzzer utilisent le meme cap utilisateur. Le buzzer se declenche quand la direction affichee est proche du nord.

## Carte Et Materiel

| Element | Role |
|---|---|
| NUCLEO-L152RE | Carte STM32 principale |
| STM32L152RET6 | Microcontroleur ARM Cortex-M3 |
| Shield Yncrea2 | Shield pedagogique avec LEDs, buzzer, capteurs |
| LIS2MDL | Magnetometre, mesure le champ magnetique |
| LIS2DW12 | Accelerometre, detecte une inclinaison trop forte |
| MAX7219 | Driver de l'afficheur 7 segments |
| Barre LEDs L0-L7 | Indication du secteur de direction |
| Buzzer PC7 | Signal sonore quand le nord affiche est proche |
| Bouton bleu PC13 | Affiche temporairement la valeur ADC |
| PA0 | Entree analogique ADC1 canal 0 |

## Peripheriques Utilises

| Peripherique | Utilisation | Mode |
|---|---|---|
| GPIO | LEDs, buzzer, bouton, bus logiciels | HAL GPIO + registres |
| TIM6 | Cadence periodique de la lecture ADC | Interruption |
| EXTI15_10 | Interruption du bouton bleu PC13 | Interruption |
| ADC1 | Lecture analogique PA0 | Registres CMSIS |
| USART2 | Debug `printf` vers ST-LINK | HAL UART |
| I2C logiciel | Communication LIS2MDL et LIS2DW12 | Bit-banging PB8/PB9 |
| SPI logiciel | Communication MAX7219 | Bit-banging PA5/PA7/PA8 |

Le projet respecte la consigne des deux peripheriques en interruption avec `TIM6_IRQHandler` et `EXTI15_10_IRQHandler`.

## Fonctionnement General

1. Initialisation de l'horloge systeme a 32 MHz.
2. Initialisation GPIO, UART2, ADC1 et TIM6.
3. Initialisation MAX7219, LIS2MDL et LIS2DW12.
4. Lecture periodique du magnetometre LIS2MDL.
5. Calcul du cap brut a partir des axes X/Y.
6. Correction de l'orientation du capteur.
7. Transformation en cap utilisateur avec `Compass_UserHeading()`.
8. Affichage du cap utilisateur, mise a jour des LEDs et gestion du buzzer.
9. Verification de l'inclinaison avec LIS2DW12.
10. Gestion des erreurs `ERR` et `TILT`.

## Calcul Du Cap

Le magnetometre donne des valeurs brutes signees `x`, `y`, `z`.

La fonction `Compass_HeadingFromMag(x, y)` calcule un angle entre 0 et 359 degres avec une approximation entiere. Cette approche evite l'utilisation de fonctions flottantes lourdes comme `atan2`.

Le cap est ensuite corrige en deux etapes :

| Fonction | Role |
|---|---|
| `Compass_CorrectHeading()` | Corrige l'orientation du capteur magnetique |
| `Compass_UserHeading()` | Aligne le cap avec le segment bleu et l'affichage |

Le cap utilisateur est celui qui est affiche et utilise pour les LEDs et le buzzer.

## Affichage

L'afficheur MAX7219 est pilote en SPI logiciel :

| Signal | Pin |
|---|---|
| SCK | PA5 |
| MOSI | PA7 |
| CS | PA8 |

Les messages possibles sont :

| Affichage | Signification |
|---|---|
| `N000` | Segment bleu pointe vers le nord |
| `E090` | Segment bleu pointe vers l'est |
| `S180` | Segment bleu pointe vers le sud |
| `W270` | Segment bleu pointe vers l'ouest |
| `TILT` | Carte trop inclinee |
| `ERR` | Magnetometre absent ou lectures echouees |
| `Axxx` | Valeur ADC normalisee |

## Buzzer

Le buzzer est sur PC7 et il est pilote en GPIO par basculement rapide de la broche.

Le buzzer utilise le cap utilisateur, donc il est coherent avec l'affichage :

| Ecart au nord affiche | Comportement |
|---|---|
| Plus de 20 degres | Buzzer eteint |
| 6 a 20 degres | Bip court et lent |
| 0 a 5 degres | Bip plus long et plus frequent |

Si la carte est en `TILT` ou en `ERR`, le buzzer est coupe.

## Valeur ADC

L'ADC mesure la tension analogique presente sur PA0 avec ADC1 canal 0.

La conversion brute est sur 12 bits :

```text
0 a 4095
```

Pour l'affichage sur 4 digits, la valeur est normalisee :

```text
A000 a A999
```

Exemples :

| Tension PA0 | Valeur brute approximative | Affichage |
|---|---:|---|
| 0 V | 0 | `A000` |
| 1,65 V | 2048 | `A499` |
| 3,3 V | 4095 | `A999` |

Le bouton bleu PC13 affiche cette valeur pendant environ 1,2 seconde, puis la boussole revient automatiquement.

## Interruptions

### TIM6

TIM6 est configure avec :

```text
PSC = 32000 - 1
ARR = 1000 - 1
```

Avec une horloge de 32 MHz, cela genere une interruption environ toutes les secondes. L'interruption incremente une demande de lecture ADC. La lecture ADC elle-meme est faite dans la boucle principale pour garder l'interruption courte.

### EXTI15_10

Le bouton bleu PC13 declenche `EXTI15_10_IRQHandler`. Le handler appelle `App_ButtonIrqHandler()`, qui applique un anti-rebond logiciel de 200 ms et demande l'affichage temporaire de l'ADC.

## Gestion Des Erreurs

| Situation | Reaction |
|---|---|
| Magnetometre absent au demarrage | Affichage `ERR` |
| Plusieurs lectures LIS2MDL echouent | Affichage `ERR` |
| Carte trop inclinee | Affichage `TILT`, LEDs extremes, buzzer coupe |
| Accelerometre absent | La detection `TILT` est ignoree, la boussole continue |

## Conformite Aux Consignes PPTX

| Consigne | Etat |
|---|---|
| Presentation 5 a 6 slides | OK, `Projet STM32 (1).pptx` contient 6 slides |
| README exploitable | OK |
| GPIO LED et bouton | OK |
| TIMER | OK, TIM6 |
| ADC | OK, ADC1 sur PA0 |
| UART printf | OK, USART2 |
| SPI et/ou I2C | OK, SPI logiciel + I2C logiciel |
| Au moins 2 peripheriques en interruptions | OK, TIM6 + EXTI15_10 |
| Au moins 2 capteurs du shield | OK, LIS2MDL + LIS2DW12 |

## Fichiers Principaux

| Fichier | Role |
|---|---|
| `Core/Src/main.c` | Logique principale de la boussole |
| `Core/Inc/main.h` | Definitions de broches et prototypes exportes |
| `Core/Src/stm32l1xx_it.c` | Handlers TIM6 et EXTI15_10 |
| `README.md` | Guide utilisateur |
| `presentation_boussole.md` | Contenu de presentation en Markdown |
| `Projet STM32 (1).pptx` | Presentation 6 slides |
| `BOARD.md` | Reference materielle de la carte |
