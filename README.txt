Projekt: Rejestaror Parametrów Środowiskowych
Project: The Recorder of Environmental Parameters
Autor/Author: Piotr Knutel (piknut@linux.pl), 2020-2021

Rejestrator dokonuje pomiarów: temperatury, ciśnienia
i wilgotności względnej powietrza atmosferycznego.

Projekt oparto o mikrokontroler z rodziny STM32
(STM32F103RC), cyfrowy czujnik BME280 i kartę micro-SD.

Pomiary wykonywane są cyklicznie (co ustalony czas,
np. 5, 10, 15, 30 min) lub na żądanie użytkownika
(przez naciśnięcie przycisku). Wyniki zapisywane są
w plikach CSV, na karcie SD. Ustawienia urządzenia
wprowadza się i odczytuje poprzez opracowaną aplikację
komputerową, która komunikuje się z rejestratorem,
przez wirtualny port szeregowy i przewód USB.  

Rejestrator może być stosowane do długookresowych
badań środowisk – takich jak miejsca funkcjonowania
ludzi i maszyn, magazyny oraz kontenery towarowe
– lub jako domowa stacja pogodowa. Urządzenie może
być zasilane przewodowo lub bateryjnie. 

Projekt obejmuje:
- schemat i projekt PCB (w programie KiCAD);
- program mikroprocesora STM32F103RC (w języku C,
  w oparciu o STMCube);
- terminalową aplikację komputerową do wprowadzania
  i odczytu ustawień poprzez port szeregowy (w języku
  C#, dla Windows i Linux);
– projekt obudowy (w programie FreeCAD).
