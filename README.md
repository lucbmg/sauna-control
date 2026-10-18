# Sauna Control

Contrôleur de sauna sur Arduino Mega.

## Matériel
- Arduino Mega (SPI : 50/51/52, CS : 53, Nextion sur Serial1 : 18/19)
- Sonde PT100 + Adafruit MAX31865 (3 fils, RREF 430 Ω)
- 2 SSR pour les éléments chauffants (pins 30, 31 — alternés toutes les 4 s pour répartir la charge)
- 1 relais pour la lumière (pin 32)
- Écran tactile Nextion (9600 bauds)

## Librairies Arduino requises
- Adafruit MAX31865
- SPI (incluse)

## Commandes Nextion → Arduino (via Serial1)
| Commande | Action |
|----------|--------|
| `START` | Démarre la session (30 min par défaut) |
| `STOP` | Arrête la session |
| `LTOGGLE` | Allume / éteint la lumière |
| `TUP` / `TDOWN` | Consigne +1 / −1 °C (30–95 °C) |
| `TIME<minutes>` | Définit la durée et démarre (ex. `TIME45`) |

## Affichage (Arduino → Nextion, toutes les 500 ms)
`tTemp` (température actuelle), `tSet` (consigne), `tTime` (minutes restantes),
`tStatus` (ACTIF/OFF), `tLight` (ON/OFF).

## Sécurité
- Tout est éteint au démarrage.
- Le chauffage coupe automatiquement à la fin de la minuterie.
- Le 120 V passe uniquement par les SSR dans une boîte électrique fermée.
