# ProjectSTM32Bousole — README technique complet

Boussole numérique embarquée sur **NUCLEO-L152RE** avec shield **Yncrea2**.
Auteurs : Wallner Lucas, Debayle Isaïe.

Ce README permet à toute personne reprenant le projet de le configurer depuis zéro, le flasher dans le bon ordre, et l'utiliser correctement.

---

## 1. Matériel requis

| Élément | Référence |
|---|---|
| Carte de développement | NUCLEO-L152RE |
| Microcontrôleur | STM32L152RET6 (Cortex-M3, 32 bits, sans FPU) |
| Shield | Yncrea2 (LEDs, buzzer, capteurs, afficheur 7-segments) |
| Magnétomètre | LIS2MDL (I²C, adresse `0x1E`) |
| Accéléromètre | LIS2DW12 (I²C, adresse `0x19`) |
| Afficheur | MAX7219 — 7-segments 4 digits (SPI) |
| Bouton utilisateur | B1 bleu sur PC13 |
| Câble | USB Mini-B (ST-LINK intégré à la carte) |

---

## 2. Logiciels requis

| Logiciel | Version conseillée | Rôle |
|---|---|---|
| STM32CubeIDE | 1.13 ou plus récent | Édition, compilation, flash |
| STM32CubeMX | 6.16.1 (intégré à CubeIDE) | Configuration des périphériques |
| Pilote ST-LINK | Inclus avec CubeIDE | Communication USB ↔ carte |
| Terminal série | PuTTY, Tera Term, ou moniteur série CubeIDE | Lecture des traces UART |

Aucune dépendance externe : le projet n'utilise que le HAL STM32 livré par CubeMX et la libc.

---

## 3. Configuration CubeMX

Le fichier `ProjectSTM32Bousole.ioc` contient déjà toute la configuration. Si on veut la reproduire à partir d'un projet vierge, voici les réglages exacts.

### 3.1 Sélection de la carte

- **Board Selector** → NUCLEO-L152RE
- À la question *"Initialize all peripherals with their default mode?"* → **Oui**

### 3.2 Horloge — Clock Configuration

C'est le point le plus important. La carte doit tourner à **32 MHz**.

| Paramètre | Valeur |
|---|---|
| Source de l'oscillateur | **HSI** (oscillateur interne 16 MHz) |
| PLL Source | HSI |
| PLLMUL | **×6** |
| PLLDIV | **÷3** |
| SYSCLK Source | **PLLCLK** |
| AHB Prescaler | ÷1 |
| APB1 Prescaler | ÷1 |
| APB2 Prescaler | ÷1 |
| Flash Latency | **1 WS** |

Résultat : HSI 16 MHz × 6 ÷ 3 = **32 MHz** sur SYSCLK, HCLK, APB1, APB2.

> Si vous voyez des fréquences en rouge dans la vue Clock Configuration, c'est qu'un paramètre est incohérent — vérifiez la latence flash (1 WS pour 32 MHz).

### 3.3 Périphériques activés dans CubeMX

| Périphérique | Mode | Notes |
|---|---|---|
| **SYS** | SysTick | Tick HAL |
| **RCC** | HSE = Crystal/Ceramic ; LSE = Crystal/Ceramic | Présents physiquement sur la NUCLEO mais inutilisés |
| **USART2** | Asynchronous | Mode `VM_ASYNC` |

Le reste — TIM6, ADC1, EXTI bouton, GPIO bus logiciels — est configuré **dans le code** (`main.c`) plutôt que dans CubeMX. C'est un choix conscient : le contrôle est total et le code n'est pas réécrasé à chaque régénération CubeMX.

### 3.4 USART2 — débit série

| Paramètre | Valeur |
|---|---|
| Mode | Asynchronous |
| Baud Rate | **115200 bits/s** |
| Word Length | **8 bits** |
| Parity | **None** |
| Stop Bits | **1** |
| Hardware Flow Control | None |
| Over Sampling | 16 |

Pins : **PA2 = USART2_TX**, **PA3 = USART2_RX** (reliés au ST-LINK virtual COM port).

### 3.5 Pin assignments (broches utilisées dans le code)

| Fonction | Broche | Mode |
|---|---|---|
| Bouton B1 (interruption) | PC13 | GPIO_EXTI13 (front montant) |
| LED utilisateur (héritée) | PA5 | GPIO_Output |
| ADC1 canal 0 | PA0 | Analog input |
| Buzzer BZ1 | PC7 | GPIO Output Push-Pull |
| SPI logiciel MAX7219 — SCK | PA5 | GPIO Output |
| SPI logiciel MAX7219 — MOSI | PA7 | GPIO Output |
| SPI logiciel MAX7219 — CS | PA8 | GPIO Output |
| I²C logiciel — SCL | PB8 | GPIO Open-Drain + Pull-up |
| I²C logiciel — SDA | PB9 | GPIO Open-Drain + Pull-up |
| Barre LEDs L0..L7 | PB1, PB2, PB10, PB11, PB12, PB13, PB14, PB15 | GPIO Output |

> Sur PA5 il y a un conflit logique avec la LED verte LD2 de la carte. Dans notre projet, **PA5 sert au SCK du MAX7219** — la LED verte clignote donc en rythme avec l'envoi SPI, ce qui n'est pas gênant.

### 3.6 NVIC — Interruptions

Cocher dans NVIC Settings :
- `EXTI line[15:10] interrupts` → priorité 1
- `TIM6 global interrupt` → priorité 2
- `System tick timer` → laissé par défaut

### 3.7 Code Generation

- *Settings → Code Generator* → cocher **"Generate peripheral initialization as a pair of '.c/.h' files per peripheral"** (optionnel mais propre)
- **Keep User Code when re-generating** : OUI (important — sans ça vous perdrez tout le code applicatif à la prochaine régénération CubeMX)

Puis **Project → Generate Code** (`Alt+K`).

---

## 4. Configuration CubeIDE

### 4.1 Ouverture du projet

1. Lancer STM32CubeIDE.
2. *File → Open Projects from File System*.
3. Sélectionner le dossier `ProjectSTM32Bousole`.
4. Le projet apparaît dans le Project Explorer.

### 4.2 Choix de la configuration de build

- En haut, sélectionner la configuration **Debug** (et non Release).
- *Project → Build Project* (`Ctrl+B`).

La sortie compilée se trouve dans `Debug/ProjectSTM32Bousole.elf`.

### 4.3 Configuration du Run/Debug

Le fichier `ProjectSTM32Bousole.launch` est déjà dans le projet. Sinon, en créer un :
- *Run → Debug Configurations → STM32 C/C++ Application → New launch configuration*
- **Project** : ProjectSTM32Bousole
- **C/C++ Application** : `Debug/ProjectSTM32Bousole.elf`
- Onglet **Debugger** :
  - Debug probe : **ST-LINK (ST-LINK GDB server)**
  - Interface : **SWD**
  - Reset behaviour : Software system reset
- Onglet **Startup** : laisser les valeurs par défaut.

### 4.4 Moniteur série pour lire l'UART

- *Window → Show View → Other → Terminal*
- Bouton **Open a Terminal** → **Serial Terminal**
- **Serial port** : sélectionner le port virtuel ST-LINK (souvent `COMx` sous Windows, `/dev/ttyACMx` sous Linux)
- **Baud rate** : **115200**
- **Data bits** : 8, **Parity** : None, **Stop bits** : 1, **Flow control** : None

Tout ce qu'envoie `printf` apparaîtra dans ce terminal.

---

## 5. Procédure de flash — IMPORTANT

> **Le shield Yncrea2 doit être branché APRÈS un premier flash sans shield.**
> Cette procédure est obligatoire à cause de potentiels conflits entre l'état initial des broches au démarrage (avant que le code ne les configure) et le câblage du shield.

### Procédure complète, dans l'ordre

#### Étape 1 — Flash initial **SANS le shield**

1. **Vérifier que le shield Yncrea2 N'est PAS branché** sur la NUCLEO.
2. Brancher la NUCLEO-L152RE au PC via USB (port ST-LINK Mini-B).
3. Dans CubeIDE, vérifier que la configuration **Debug** est sélectionnée.
4. Cliquer sur le bouton **Run** (flèche verte) ou *Project → Run* (`Ctrl+F11`).
5. Attendre la fin du flash. Le terminal série doit montrer un message d'init (`Boussole: MAX7219 OK, LIS2MDL absent, LIS2DW12 accel absent` — c'est normal, les capteurs sont sur le shield qui n'est pas connecté).
6. **Débrancher la carte du PC.**

#### Étape 2 — Brancher le shield

7. La carte étant **hors tension** (USB débranché), positionner le shield Yncrea2 au-dessus de la NUCLEO en respectant l'alignement des connecteurs Arduino (CN5, CN6, CN8, CN9).
8. Enfoncer le shield à fond, sans forcer en biais.
9. Vérifier visuellement qu'aucune broche n'est tordue et que le shield est bien à plat.

#### Étape 3 — Second flash **AVEC le shield**

10. Rebrancher la carte au PC via USB.
11. Cliquer à nouveau sur **Run** dans CubeIDE pour reflasher.
12. Le moniteur série doit maintenant afficher :
    ```
    Boussole: MAX7219 OK, LIS2MDL OK, LIS2DW12 accel OK
    ```
13. L'afficheur 7-segments doit montrer brièvement `INIT` puis afficher un cap (par exemple `N045`).

Si le moniteur affiche `LIS2MDL absent` à cette étape, le shield est mal enfoncé : débrancher, réenfoncer, reflasher.

---

## 6. Fonctionnement du programme

### 6.1 Démarrage

1. `HAL_Init()` initialise le HAL et la SysTick.
2. `SystemClock_Config()` règle l'horloge à 32 MHz.
3. `MX_GPIO_Init()` configure toutes les broches.
4. `MX_USART2_UART_Init()` ouvre l'UART à 115200 bauds.
5. `ADC1_AppInit()` active l'ADC1 en accès registre (CMSIS) sur le canal 0.
6. `TIM6_AppInit()` configure le timer TIM6 avec PSC = 31999 et ARR = 999 → interruption à environ **1 Hz**.
7. `Compass_AppInit()` initialise le MAX7219, affiche `INIT`, puis interroge le `WHO_AM_I` de chaque capteur (LIS2MDL = `0x40`, LIS2DW12 = `0x44`).

### 6.2 Boucle principale (`Compass_AppTask`)

Trois rythmes cohabitent dans la même boucle :

| Rythme | Source | Action |
|---|---|---|
| **150 ms** | Mesure de `HAL_GetTick` | Lecture du magnétomètre, calcul du cap, mise à jour afficheur/LEDs/buzzer |
| **≈ 1 s** | Interruption TIM6 → flag | Conversion ADC sur PA0 |
| **Sur événement** | Interruption EXTI13 (bouton) | Affiche temporairement la valeur ADC `Axxx` pendant 1,2 s |

### 6.3 Calcul du cap

1. Lecture I²C du LIS2MDL → composantes brutes Bx, By, Bz.
2. `Compass_HeadingFromMag(x, y)` : approximation entière de `atan2` par octants — pas de flottant.
3. `Compass_CorrectHeading()` : inversion du sens (capteur monté à l'envers).
4. `Compass_UserHeading()` : décalage de 180° pour aligner avec le segment bleu du shield.
5. `Compass_Cardinal()` : conversion en lettre N/E/S/W (8 secteurs de 45° centrés).

### 6.4 Détection d'inclinaison (TILT)

À chaque cycle, lecture de l'accéléromètre LIS2DW12. Si `|ax| > 1400` ou `|ay| > 1400` (≈ 30° d'inclinaison), affichage de `TILT`, allumage des LEDs extrêmes, coupure du buzzer. La boussole reprend automatiquement quand la carte revient à l'horizontale.

### 6.5 Gestion des erreurs (ERR)

Si la lecture du LIS2MDL échoue **3 fois consécutives**, affichage de `ERR`, extinction des LEDs et du buzzer. Le système se rétablit automatiquement quand les lectures redeviennent valides.

### 6.6 Buzzer — proximité du nord

| Écart au nord | Comportement |
|---|---|
| > 20° | Silence |
| 6° à 20° | Bip court, espacement ≈ 650 ms |
| 0° à 5° | Bip plus long, espacement ≈ 180 ms |

### 6.7 Trace UART (`printf`)

Chaque cycle écrit une ligne du type :
```
Cap affiche: 045 deg [E], ecart nord affiche 045 deg
```
ou en cas de détection :
```
Inclinaison trop forte: mesure du cap ignoree
```
Très utile pour le debug à distance ou pendant la démonstration.

---

## 7. Manipulation de la carte côté utilisateur

### 7.1 Tenir la carte

1. Tenir la NUCLEO + shield à **plat** dans la main, parallèle au sol.
2. Le **segment bleu** du shield (marqué visuellement sur la carte) est la **flèche de visée** : c'est lui qui pointe la direction à mesurer.

### 7.2 Lire l'afficheur

L'afficheur 7-segments à 4 digits indique en permanence :

| Affichage | Signification |
|---|---|
| `N000` | Vous pointez vers le nord magnétique |
| `E090` | Vous pointez vers l'est, à 90° |
| `S180` | Vous pointez vers le sud |
| `W270` | Vous pointez vers l'ouest |
| `N045` | Nord-est, 45° |
| `TILT` | Carte trop inclinée — remettez-la à plat |
| `ERR ` | Capteur indisponible — vérifiez le shield |
| `Axxx` | Valeur ADC de diagnostic après appui bouton (000–999) |

### 7.3 Utiliser la barre de LEDs

La LED allumée indique le **secteur de 45°** vers lequel pointe le segment bleu. C'est une rose des vents miniature — utile pour percevoir la direction d'un coup d'œil sans lire les chiffres.

### 7.4 Utiliser le buzzer

Tournez doucement la carte. Le buzzer commence à biper quand vous êtes à moins de **20°** du nord, et devient plus rapide en dessous de **5°**. Vous pouvez ainsi trouver le nord **sans regarder l'afficheur**.

### 7.5 Utiliser le bouton bleu

Appuyer une fois sur le bouton bleu (B1, PC13) → l'afficheur bascule pendant **1,2 seconde** en mode diagnostic ADC, affichage `Axxx` où xxx ∈ [000–999] correspond à la position du potentiomètre sur PA0 (normalisé depuis la valeur brute 12 bits 0–4095). Puis retour automatique en mode boussole.

### 7.6 Bonnes pratiques de mesure

- Tenir la carte **éloignée des sources métalliques** (téléphone, ordinateur, support en fer) pour limiter les distorsions magnétiques.
- Tenir la carte **horizontale** — la détection TILT empêche les mesures faussées par l'inclinaison.
- Pour la démonstration, prévoir un repère extérieur (par exemple le nord donné par une application boussole de smartphone) pour valider visuellement la mesure.

---

## 8. Conformité aux consignes du sujet

| Consigne | Réponse dans le projet |
|---|---|
| GPIO | LEDs L0–L7, buzzer PC7, bouton B1 |
| TIMER | TIM6 en interruption à 1 Hz |
| ADC | ADC1 canal 0 sur PA0 (12 bits) |
| UART | USART2 à 115200 bauds avec `printf` redirigé |
| SPI et/ou I²C | SPI logiciel (MAX7219) + I²C logiciel (LIS2MDL + LIS2DW12) |
| ≥ 2 périphériques en interruption | TIM6 + EXTI15_10 |
| ≥ 2 capteurs du shield | LIS2MDL + LIS2DW12 |

---

## 9. Compilation en ligne de commande (optionnel)

Si le toolchain STM32CubeIDE est dans le `PATH` :

```sh
cd Debug
make all
```

Le firmware produit est `Debug/ProjectSTM32Bousole.elf`.

Pour flasher en ligne de commande (avec ST-LINK) :

```sh
STM32_Programmer_CLI -c port=SWD -w Debug/ProjectSTM32Bousole.elf -rst
```

---

## 10. Dépannage rapide

| Symptôme | Cause probable | Solution |
|---|---|---|
| Afficheur reste à `INIT` ou ne s'allume pas | MAX7219 absent ou shield mal enfoncé | Vérifier connecteurs, refaire la procédure de flash en 2 temps |
| Affichage `ERR ` permanent | LIS2MDL non détecté | Vérifier le shield, redémarrer la carte |
| Affichage `TILT` même à plat | Calibration accéléromètre / orientation atypique | Vérifier que la carte est bien à plat, relancer |
| Cap incorrect mais cohérent (toujours décalé) | Perturbation magnétique locale | Éloigner les objets métalliques, prévoir calibration hard-iron |
| Rien sur le moniteur série | Mauvais port COM ou mauvais débit | Vérifier port ST-LINK virtual COM + baud 115200 |
| Le programme ne flash pas | Pilote ST-LINK non installé ou USB défectueux | Réinstaller drivers ST, tester un autre câble USB |

---

## 11. Fichiers principaux du projet

| Fichier | Rôle |
|---|---|
| `ProjectSTM32Bousole.ioc` | Configuration CubeMX (à ouvrir avec STM32CubeIDE) |
| `Core/Src/main.c` | Logique principale de la boussole |
| `Core/Inc/main.h` | Définitions de broches et prototypes |
| `Core/Src/stm32l1xx_it.c` | Handlers TIM6 et EXTI15_10 |
| `Debug/ProjectSTM32Bousole.elf` | Firmware compilé |
| `STM32L152RETX_FLASH.ld` | Linker script |
| `README.md` | Ce fichier |
| `FICHE_TECHNIQUE.md` | Détails techniques étendus |

---

**Projet prêt pour démonstration.**
Pour toute question : Wallner Lucas & Debayle Isaïe.
