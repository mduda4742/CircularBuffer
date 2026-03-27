# Zadanie 2: Bufor Kołowy – Producent i Konsumenci

Implementacja systemu zapisywania i analizy danych pomiarowych w czasie zbliżonym do rzeczywistego. Program wykorzystuje mechanizm pamięci współdzielonej (Shared Memory) oraz strukturę bufora kołowego, aby umożliwić wielu konsumentom dostęp do próbek sygnału sinusoidalnego generowanego przez producenta (vSensor).

### Opis zadania
* **vSensor (Producent):** Generuje sygnał sinusoidalny o częstotliwości f1, próbkowany z częstotliwością fs, i umieszcza dane w buforze kołowym o rozmiarze N.
* **monitor_avg (Konsument):** Oblicza prostą średnią ruchomą (SMA) dla ostatnich np próbek.
* **monitor_f (Konsument):** Estymuje częstotliwość sygnału poprzez zliczanie przejść przez zero (Zero Crossing Detection).

