# Presentation - Boussole numerique STM32

## Slide 1 - Presentation de l'application

**Boussole numerique embarquee sur STM32**

- Objectif : afficher un cap magnetique lisible en temps reel.
- Plateforme : NUCLEO-L152RE / STM32L152RET6, projet CubeMX.
- Capteur principal : magnetometre LIS2MDL via I2C logiciel.
- Capteur secondaire : accelerometre LIS2DW12 pour detecter une inclinaison trop forte.
- Sorties utilisateur : afficheur 7 segments MAX7219, barre de LEDs, bouton, buzzer et debug UART.

**Message oral :** l'application transforme des mesures magnetiques brutes en une indication de direction simple a comprendre.

---

## Slide 2 - Fonctionnement utilisateur

**Mode de fonctionnement**

- Mode cap : affichage de la direction cardinale et de l'angle, par exemple `N045`.
- La lettre, les degres, les LEDs et le bip utilisent tous le cap du segment bleu.
- Les LEDs indiquent le secteur de direction.
- Le buzzer emet un son quand la direction affichee pointe vers le nord.
- Le bouton bleu affiche temporairement la valeur ADC normalisee du potentiometre/capteur PA0 sous la forme `Axxx`.

**Message oral :** l'utilisateur n'a pas besoin de lire une valeur brute ; l'interface combine affichage, LEDs et son.

---

## Slide 3 - Defi et problematique

**Mesurer une direction fiable sur une carte embarquee**

- Les donnees du magnetometre sont brutes et signees sur trois axes.
- Le champ magnetique peut etre perturbe par l'environnement et les composants proches.
- Une boussole devient moins fiable quand la carte est trop inclinee.
- Les ressources sont limitees : STM32L152 a 32 MHz, logique temps reel simple, pas d'OS.
- Les peripheriques doivent cohabiter avec des pins partagees : I2C logiciel, SPI logiciel, LEDs, buzzer, UART.

**Problematique :** comment produire une indication de cap claire, reactive et robuste avec des capteurs MEMS et peu de ressources ?

---

## Slide 4 - Solution technique argumentee

**Architecture retenue**

- Initialisation des composants : MAX7219, LIS2MDL, LIS2DW12, UART2.
- Lecture periodique toutes les 150 ms pour garder une interface reactive sans saturer le MCU.
- TIM6 genere une interruption periodique utilisee pour cadencer la lecture ADC.
- Le bouton bleu utilise une interruption EXTI15_10 avec anti-rebond logiciel.
- ADC1 lit PA0 pour respecter l'entree analogique du shield et fournir une valeur de diagnostic.
- I2C logiciel sur PB8/PB9 : controle direct du bus et integration sans dependance a un peripherique I2C CubeMX.
- SPI logiciel vers MAX7219 sur PA5/PA7/PA8 : affichage simple de 4 caracteres.
- Calcul du cap avec `Compass_HeadingFromMag(x, y)` : approximation entiere, rapide et adaptee au Cortex-M3.
- Transformation du cap utilisateur avec `Compass_UserHeading()` pour aligner lettre, degres et bip avec le segment bleu.
- Detection d'inclinaison avec LIS2DW12 : si la carte est trop penchee, affichage `TILT` et mesure ignoree.

**Argument :** la solution privilegie la simplicite, la robustesse et la lisibilite plutot qu'un algorithme lourd.

---

## Slide 5 - Resolution

**Traitement realise dans la boucle principale**

1. Attente de la periode de mise a jour de 150 ms.
2. Lecture du magnetometre LIS2MDL.
3. Calcul du cap 0-360 degres a partir des axes X/Y, avec correction de l'orientation mecanique du capteur et de l'afficheur.
4. Verification de l'inclinaison avec l'accelerometre.
5. Lecture ADC cadencee par TIM6 et gestion du bouton EXTI.
6. Mise a jour de l'afficheur, des LEDs, du buzzer et de l'UART.

**Gestion des cas d'erreur**

- Capteur absent ou lecture impossible : affichage `ERR`.
- Inclinaison trop forte : affichage `TILT`, LEDs extremes, buzzer coupe.
- Nord affiche detecte : feedback sonore progressif.

---

## Slide 6 - Conclusion

**Bilan du projet**

- Application fonctionnelle de boussole numerique embarquee.
- Integration de plusieurs interfaces : GPIO, UART, I2C logiciel, SPI logiciel.
- Interface utilisateur multi-sorties : angle, direction, LEDs, signal sonore.
- Gestion de robustesse : detection capteur, erreur, inclinaison.

**Ameliorations possibles**

- Ajouter une calibration magnetometre hard iron / soft iron.
- Utiliser une compensation d'inclinaison complete avec roll/pitch.
- Sauvegarder la calibration en EEPROM.
- Remplacer l'I2C logiciel par I2C materiel si la configuration de pins le permet.
