# Symulator Układu Automatycznej Regulacji (UAR)

Projekt aplikacji okienkowej zrealizowany w języku C++ z wykorzystaniem frameworka Qt. Program umożliwia symulację, analizę oraz wizualizację działania układów automatycznej regulacji. Obejmuje m.in. symulację obiektu z wykorzystaniem modelu ARX, działanie regulatora PID, generację sygnałów wymuszających oraz komunikację sieciową opartą na architekturze klient-serwer.

## Główne funkcjonalności

* **Symulacja obiektu (Model ARX):** Implementacja dyskretnego modelu ARX do symulacji zachowania obiektów dynamicznych, uwzględniająca opóźnienia i zakłócenia.
* **Algorytm regulacji (Regulator PID):** Cyfrowy regulator proporcjonalno-całkująco-różniczkujący działający w pętli sprzężenia zwrotnego.
* **Generator sygnałów:** Moduł odpowiedzialny za generację wartości zadanych dla układu (np. skok jednostkowy, sygnał sinusoidalny, sygnał prostokątny).
* **Wizualizacja danych:** Rysowanie wykresów w czasie rzeczywistym przy użyciu biblioteki QCustomPlot. Narzędzie pozwala na monitorowanie sygnału zadanego, uchybu regulacji, sygnału sterującego oraz odpowiedzi obiektu.
* **Komunikacja sieciowa:** Wymiana danych symulacyjnych oraz parametrów układu za pomocą gniazd sieciowych (architektura klient-serwer).

## Technologie

* **Język programowania:** C++ 
* **Framework GUI:** Qt
* **Biblioteki zewnętrzne:** QCustomPlot (do wizualizacji danych)

## Struktura plików projektu

Poniżej znajduje się opis najważniejszych plików źródłowych i klas wchodzących w skład aplikacji:

* `mainwindow.cpp` / `.h` / `.ui` - definicja i implementacja głównego okna aplikacji oraz zarządzanie interfejsem użytkownika.
* `oknoarx.cpp` / `.h` / `.ui` - okno konfiguracyjne parametrów modelu ARX.
* `oknosiec.cpp` / `.h` / `.ui` - okno zarządzające konfiguracją połączenia sieciowego.
* `ModelARX.cpp` / `.h` - implementacja matematycznego modelu obiektu dynamicznego.
* `RegulatorPID.cpp` / `.h` - implementacja algorytmu regulatora.
* `ProstyUAR.cpp` / `.h` - klasa integrująca powyższe komponenty w działający układ automatycznej regulacji.
* `SignalGenerator.cpp` / `.h` - klasa odpowiedzialna za generowanie sygnałów wymuszających.
* `klient.cpp` / `.h`, `serwer.cpp` / `.h` - moduły obsługujące komunikację sieciową.
* `qcustomplot.cpp` / `.h` - zewnętrzna biblioteka graficzna do obsługi wykresów.
* `ProjektGUI.pro` - główny plik konfiguracyjny środowiska qmake.

## Kompilacja i uruchomienie

Do poprawnego zbudowania projektu wymagane jest środowisko programistyczne obsługujące framework Qt (np. Qt Creator) oraz odpowiedni kompilator (np. MinGW lub GCC).

1. Pobierz lub sklonuj repozytorium na dysk lokalny.
2. Uruchom środowisko Qt Creator.
3. Wybierz opcję "Otwórz plik lub projekt..." i wskaż plik `ProjektGUI.pro`.
4. Skonfiguruj projekt, wybierając odpowiedni zestaw narzędzi (Kit).
5. Skompiluj i uruchom aplikację (domyślny skrót: Ctrl + R).

## Informacje o projekcie

Projekt zrealizowany w ramach 4. semestru studiów jako praktyczna implementacja zagadnień z zakresu sieci komputerowych oraz podstaw automatyki.
