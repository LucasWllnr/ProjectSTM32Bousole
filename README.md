# ProjectSTM32Bousole

## Presentation

Ce projet est une boussole numerique pour carte NUCLEO-L152RE avec shield Yncrea2.

Le programme lit le magnetometre LIS2MDL, calcule un cap entre 0 et 359 degres, puis affiche une direction lisible sur l'afficheur 7 segments MAX7219. L'accelerometre LIS2DW12 sert a detecter une inclinaison trop forte.

## Utilisation De La Boussole

1. Brancher et flasher la carte avec la configuration `Debug`.
2. Garder la carte aussi horizontale que possible.
3. Orienter le segment bleu vers la direction a mesurer.
4. Lire l'afficheur 7 segments.

L'affichage principal a le format suivant :

```text
N000
E090
S180
W270
```

La lettre indique la direction pointee par le segment bleu. Les degres correspondent a cette meme direction affichee.

Le buzzer emet un bip quand la direction affichee se rapproche du nord :

- pas de bip si l'ecart au nord est superieur a 20 degres ;
- bip lent entre 6 et 20 degres ;
- bip plus long et plus frequent entre 0 et 5 degres.

La barre de LEDs indique le secteur de direction courant.

Le bouton bleu affiche temporairement la valeur ADC lue sur PA0 sous la forme `Axxx`. La valeur affichee est normalisee de `A000` a `A999` a partir de la mesure brute ADC 12 bits `0` a `4095`. Apres environ 1,2 seconde, l'affichage revient automatiquement a la boussole.

## Messages Speciaux

- `TILT` : la carte est trop inclinee, la mesure du cap est ignoree.
- `ERR` : le magnetometre ne repond pas ou plusieurs lectures ont echoue.
- `Axxx` : valeur ADC de diagnostic apres appui sur le bouton bleu.

## Materiel Utilise

- NUCLEO-L152RE / STM32L152RET6
- Shield Yncrea2
- Magnetometre LIS2MDL
- Accelerometre LIS2DW12
- Afficheur 7 segments MAX7219
- Barre de LEDs
- Buzzer PC7
- Bouton bleu PC13
- Entree ADC PA0

## Peripheriques Utilises

- GPIO : LEDs, buzzer, bouton bleu, bus logiciels
- TIMER : TIM6 en interruption periodique
- ADC : ADC1 canal 0 sur PA0
- UART : USART2 pour `printf`
- I2C : I2C logiciel pour LIS2MDL et LIS2DW12
- SPI : SPI logiciel pour MAX7219
- Interruptions : TIM6 et EXTI15_10

Le projet respecte les contraintes du PPTX : GPIO, TIMER, ADC, UART, SPI/I2C, deux interruptions, deux capteurs du shield, README exploitable et presentation de 6 slides.

## Compilation

Le projet est prevu pour STM32CubeIDE.

1. Ouvrir le projet dans STM32CubeIDE.
2. Selectionner la configuration `Debug`.
3. Compiler puis flasher la carte.

Depuis le terminal, avec le toolchain STM32CubeIDE dans le `PATH` :

```sh
cd Debug
make all
```

Le firmware genere est :

```text
Debug/ProjectSTM32Bousole.elf
```
