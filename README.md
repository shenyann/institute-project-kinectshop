# IP4 - Kinectshop

In diesem Projekt wir ein interaktiver Shop mit der Microsoft Kinect programmiert. Die Aufgabe wird in drei Projektgruppen aufgeteils. Gruppe 1 beschäftigt sich mit der Lokalisation
eines Produktes im Bild (z.B. durch Verwedung des Tiefenbildes und Bewegungsinformation). Gruppe 2 hat die Aufgabe eine Region-of-Interest zu klassifizieren, dh. das in einem Bildausschnitt gezeigte Produkt zu 
identifizieren (Es müssen geeignete Merkmale gefudnen und ein Klassifikator trainiert werden). Gruppe 3 erstellt das User-Interaktionsschema, dh. den Ablauf eines Kaufvorgangs und die zugehörigen Gesten/Sprachsignale.
Damit das System als ganzes funktioniert, müssen die Interfaces zwischen den Gruppen abgesprochen werden.

Der Programmcode ist stark entkoppelt. Alle Programmteile werden als eigenständige exe-Dateien kompiliert und kommunizieren über die Softwareschnittstelle <a href="http://zeromq.org/">ZeroMQ</a>.
Der Zugang zu den Bilddaten der Kinect ist im ersten Baustein realisiert. Für alle anderen Programmteile steht ein Programmskelett zur Verfügung, dass die Daten vom Treiberbaustein abrufen kann und zudem mit einer Minimallösung
der Aufgabe versehen ist - sodass jede Gruppe mit ihrem Aufgabenteil starten kann. Um neuere Lösungen der anderen Gruppen verwenden zu können, muss unter Windows dann (idealer Weise) nur die neue executable weitergegeben werden.


* Für Bildverarbeitungsaufgaben lohnt ein Blick in die Umfangreiche [OpenCV](http://opencv.org/) Bibliothek.
* Als Produktdatenbank für das Shopsystem steht eine MySQL Datenbank zur Verfügung. Den Server erreichst du [hier](kinectsrv.lfb.rwth-aachen.de). Weiter unten steht eine Anleitung, wie du die Datenbank unter Windows einbinden musst.
* Die Versionsverwaltung für dieses Projekt ist [git](https://git-scm.com/). Eine README.md wie diese, kannst du in [Markdown](https://de.wikipedia.org/wiki/Markdown) geschrieben


## Einrichten einer ODBC Datenbank
* Installieren des MySQL connectors für ODBC Datenbanken: <a href="https://dev.mysql.com/downloads/connector/odbc/">hier</a>
* ODBC Datenbankverbindung erstellen unter Systemsteuerung -> Verwaltung -> Datenquellen (ODBC) einrichten
* Data Source Name: kinectshop2016
* TCP/IP Server: kinectsrv.lfb.rwth-aachen.de
* Port: 3306
* User: kinectshopClient
* Passwd: lfb-student2016
* Database: kinectshop2016
* Einstellungen mit "Test" checken und dann mit "Ok" speichern.

## Einrichten von C++ Projekten mit Microsoft Visual Studio und anderen IDEs
Leider verändern sich die Versionen der IDEs sehr schnell, sodass wir hier nicht für alle Versionen eine ausführliche Anleitung angeben können. Aber ein paar grundlegende Dinge, gibt es bei jeder IDE zu beachten.

In umfangreicheren Projekten wir dein Code niemals alleine stehen, sondern viele gute und etablierte Bibliotheken benutzen und sie zu einem funktionierenden Programm zusammenfügen. In der Programmiersprache C++ besteht
die Erzeugung eines Programms aus den beiden Schritten "Kompilieren" und "Linken". Im ersten Schritt wird dein Programmcode syntaktisch geprüft und jede Datei in eine .obj Datei übersetzt. Damit das funktioniert müssen dem
Projekt alle Pfade bekannt sein unter denen Sytax zu finden ist, die mit deinem Programm zu tun hat. Konkret bedeutet das, dass in den Projekteinstellungen für den Compile-Vorgang alle Pfade zu .c .cpp .h und .hpp Dateien
anzugeben sind, die zu verwendeten Bibliotheken (z.B. OpenCV, ZeroMQ, etc.) führen. Die häufigsten Fehler sind hier vergessene Semikolons, fehlende Header und falsch geschriebene Variablen/Funktionsnamen.

Der Linker fügt deine eigenen .obj Dateien mit den vorkompilierten Funktionen aus Bibiliotheken zusammen und erzeugt schließlich eine ausführbare Datei.
Auch hier müssen mitunter viele Pfade gesetzt werden, diesmal zu den .lib Dateien der jeweiligen Bibliotheken. Je nach dem wie der Kompiler eingestellt ist, wird er den Code aus diesen Bibliotheken direkt in die exe-Datei einbinden 
(statisches kompilieren), oder er setzt nur Links zu den Funktionen innerhalb einer Dynamischen Bibliothek (.dll). Bei dynamischer Verlinkung muss der Pfad zu der .dll dem System bekannt sein, wenn die exe-Datei gestartet wird.
Eine einfache Methode das sicherzustellen ist es, die .dll in das Verzeichnis der exe-Datei zu kopieren oder (bei großen Bibliotheken) eine Verknüpfung anzulegen. Die typische Fehlermeldung bei defekten Pfaden ist eine nichtaufgelöste Referenz.

Abschließend sei noch angemerkt, dass es leider keinen einheitlichen  Standart gibt, wie die Dateien innerhalb eines Projektes organisiert sind, dennoch als rule-of-thumb: oftmals finden sich die header Dateien .h .hpp in einem "include" Verzeichnis, die statischen Bibliotheken (sofern vorhanden) in einem "lib" Verzeichnis und die
dynamischen Bibliotheken in einem "bin" Verzeichnis. Für ältere OpenCV Versionen gab es aber z.B. mehrere verschachtelte include Verzeichnisse, was bei einigen IDEs exzessives ausprobieren zur Folge hatte, wie man die includes alle
konfigurieren muss. In der Regel hilft einem googeln weiter!

## Hilfreiche Links
* [MySQL Tutorial](http://dev.mysql.com/doc/refman/5.6/en/tutorial.html)
* [OpenCV Reference Manual](http://docs.opencv.org/3.1.0/#gsc.tab=0)
* [MS Kinect SDK](https://msdn.microsoft.com/en-us/library/hh855370.aspx) (für die Detailverliebten)
* Für Booking Gruppe:
* [MS Speech Recognition Interfaces](https://msdn.microsoft.com/en-us/library/ms720424%28v=vs.85%29.aspx)
* [MS Basic Speech Grammars](https://msdn.microsoft.com/en-us/library/hh378522%28v=office.14%29.aspx)
* [A short introduction on Gestures](https://svn.lfb.rwth-aachen.de/kinectshop2015/trac/attachment/wiki/ProductBooking/gestures.pdf)
* [X Stroke: Gesture Recognition Examples](https://cworth.org/~cworth/papers/xstroke/html/)
* Für Classification Gruppe:
* [Wiki zu kNN Klassifikator](https://en.wikipedia.org/wiki/K-nearest_neighbors_algorithm)
* [Farbräume](https://en.wikipedia.org/wiki/HSL_and_HSV)
* [Histogram Comparison](http://docs.opencv.org/3.1.0/d6/dc7/group__imgproc__hist.html#gaf4190090efa5c47cb367cf97a9a519bd&gsc.tab=0)

