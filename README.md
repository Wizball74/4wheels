# 4wheels - Spielregeln (v1.1)

## Kurzbeschreibung
Zwei Spieler spielen abwechselnd. Durch das Drehen der vier Scheiben entstehen
an den vier Schnittpunkten Kartenkombinationen (Farbe + Wert). Gültig sind nur
diagonal gegenüberliegende Paare. Ziel ist, am Ende mit 5 Karten pro Spieler
die beste Pokerhand zu haben.

## Vorgaben
- Es gibt 2 Gemeinschaftskarten (wie bei Hold’em), die für beide Spieler gelten.
- Bereits genommene Karten sind aus dem Spiel und dürfen nicht erneut gewählt werden.
- Es muss pro Zug mindestens 1 diagonales Paar geclaimt werden.
- Ein Paar entspricht genau 1 Karte (Farbe + Wert).

## Hardware-Logik
- Linke 4 Schalter: Scheiben an/aus (zeigen immer IST-Stand).
- Rechte 4 Schalter: Drehrichtung aller 4 Scheiben (zeigen IST-Stand).
- LEDs: zeigen Claims und ggf. Fehlstellungen an.

## Vorbereitung
- Zwei Gemeinschaftskarten zufaellig aus dem 32er Skat-Deck ziehen.
- Runde 1 Startzustand: alle Scheiben aus, alle Drehrichtungen im Uhrzeigersinn.
- Ab Runde 2: Startzustand = letzter gespeicherter Zustand.

## Initialisierung
- Vor Zugbeginn wird geprueft, ob der IST-Stand der Schalter dem erwarteten
  Zustand entspricht. Falls nicht: Korrektur einfordern.
- LEDs koennen eine falsche Richtung anzeigen (Details noch festzulegen).

## Phase A: Setup (Schalterwahl)
- Aktiver Spieler muss genau zwei Schalter umlegen:
  - einen linken (Scheibe an/aus)
  - einen rechten (Drehrichtung)
- Diese Seiten gelten bis zum naechsten Zug als "gesperrt" (Vertrauensregel).

## Phase B: Aktivierung
- Mittel-Button startet einen 5-Sekunden-Countdown.
- Countdown-Logik: Anzeige zaehlt 1..5 hoch und dann 5..1 herunter.
- In dieser Zeit ist der Joystick aktiv, nur aktivierte Scheiben drehen.

## Phase C: Claim
- Kurze Sperrzeit nach dem Countdown, um versehentliches Druecken zu verhindern.
- Claim erfolgt ueber den Joystick:
  - Joystick-Richtung entspricht einer Diagonale.
  - Umschalten der Richtung schaltet die LED dieser Diagonale an/aus.
  - Sichtbar, ob 0/1/2 Paare gewaehlt sind.
- Es duerfen beide diagonalen Paare geclaimt werden.
- Es muss mindestens 1 Paar geclaimt werden.

## Phase D: Kartennehmen
- Ein geclaimtes Paar ergibt genau 1 Karte (Farbe + Wert).
- Spieler nimmt die geclaimten Karten und entfernt sie aus dem Spiel.
- Naechster Spieler ist dran.

## Spielende und Auswertung
- Spiel endet, wenn beide Spieler 5 Karten haben.
- Auswertung nach Texas Hold’em-Rangfolge mit Skat-Deck.
- Straights nur bis 7, A ist nur hoch.
