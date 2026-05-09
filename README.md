# cautafis
### Aplicație utilitar pentru căutarea obiectelor de tip fișier și / sau director 
### Conținut
1. [Descriere](#descriere)
2. [Mod de folosire](#mod-de-folosire)
3. [Opțiunile de căutare](#opțiunile-de-căutare)
4. [Opțiunile de rafinare a căutarii](#opțiunile-de-rafinare-a-căutării)
5. [Opțiunile de listare](#opțiunile-de-listare)
4. [Exemple de folosire](#exemple-de-folosire)

<!-- toc -->
## Descriere
Programul caută într-un director trimis parametru fișerele și / sau directoarele care se aseamănă ca denumire cu numele trimis parametru, sau care respectă o regulă regex trimisă parametru.<br>
Fișierele sau directoarele pot fi afișate în mod sortat alfabetic, sau în
funcție de dimensiune, sau în funcție de data ultimei modificări. Listarea se
poate face crescător sau descrescător.<br>
Dacă sunt căutate doar fișiere, atunci va fi afișată calea completă, numele și dimensiunea fișierului, pentru fiecare fișier care îndeplinește condițiile
de căutare.<br>
Dacă sunt căutate directoare, atunci va fi afișată calea compleată și numele
directorului, pentru fiecare director care îndeplinește condițiile de căutare.<br>
Se poate face o rafinare a căutării, în funcție de opțunile trimise la lansarea programului.<br>

## Mod de folosire

### Opțiuni minime la start:
	cautafis -c <director_start> -n <nume_fișier>
	sau
	cautafis -c <director_start> -r <expresie_regex>
	sau
	cautafis -h
	sau
	cautafis -v

### Opțiuni de rafinare a căutării:
- **-i** sau **--ignore-case**
- **-f** sau **--fisiere**
- **-d** sau **--directoare**
- **-a** sau **--adancime <număr>**
- **-m** sau **--min-size <număr>**
- **-M** sau **--max-size <număr>**
- **-s** sau **--sortare**
- **-l** sau **--list-desc**
- **-h** sau **--help**
- **-v** sau **--versiune**
 
## Opțiunile de căutare

Pentru căutarea folosind **numele fișierului**, inclusiv utilizând wildcard-urile '?' și / sau '*', sunt obligatorii opțiunile: 
1. **-c <nume_director>** sau **--cale <nume_director>**, unde <nume_director> este numele directorului din care se pornește căutarea,<br>
2. **-n <nume_fișier>** sau **--nume <nume_fișier>**, unde <nume_fișier>     este numele fișierului care se dorește a fi găsit.  

Pentru căutarea folosind o **expresie regex** sunt obligatorii opțiunile:
1. **-c <nume_director>** sau **--cale <nume_director>**, unde <nume_director> este numele directorului din care se pornește căutarea,<br>
2. **-r \<expresie_regex>** sau **--regex \<expresie_regex>**, unde \<expresie_regex> este o expresie regex validă folosită la căutarea fișierului.<br>

Pentru obținerea de **help**, unde sunt prezentate toate opțiunile de lansare:<br>

1. O singură opțiune, **-h** sau **--help**, caz în care este afișat un text cu toate opțiunile de căutare și listare.<br>

## Opțiunile de rafinare a căutării
- **-i** sau **--ignore-case**:<br>
        Dacă este prezentă opțiunea, nu se va face distincția între literele
        mari și literele mici la căutare.
- **-f** sau **--fisiere**:<br>
        Vor fi căutate doar fișiere normale.
- **-d** sau **--directoare**:<br>
        Vor fi căutate doar directoare.
        Dacă nu sunt prezente, simultan, nici **-f** nici **-D**, vor fi căutate și directoare și fișiere.<br>
- **-a** sau **--adancime**, urmată obligatoriu de un **număr**:<br>
        Opțiunea indică numărul de nivele la care se coboară în structura de
        directoare de sub directorul de start în care se face căutarea.
        Dacă nu este folosită această opțiune, căutarea se face în toate
        subdirectoarele directorului de start.<br>
- **-r** sau **--regex \<expresie_regex>**:<br>
        Dacă este prezentă opțiunea, se vor folosi regulile regex la căutarea
        fișierelor sau directoarelor. Altfel, căutarea se face după nume.<br>
- **-m** sau **--min-size**, urmată obligatoriu de **\<număr>**, în Megabytes:<br>
        Vor fi afișate doar fișierele care au dimensiunea mai mare decât **<număr>**;<br>
 - **-M** sau **--max-size**, urmată obligatoriu de **<număr>**, în Megabytes: <br>
        Vor fi afișate doar fișierele care au dimensiunea mai mică decât **<număr>**;<br>
## Opțiunile de listare      
- **-s** sau **--sortare**:<br>
        Opțiunea indică tipul de sortare folosit pentru listarea fișierelor sau / și directoarelor găsite. Parametrii care trebuie folosiți pentru această opțiune sunt:<br>
	- **a** = sortare după nume, <br>
	- **s** = sortare după dimensiune,<br>
    - **d** = sortare după data fișierului.  
	Dacă nu este prezentă opțiunea, sortarea se va face după numele fișierului.<br>
- **-l** sau **--list-desc**:<br>
        Dacă este prezentă opțiunea va face ca listarea să fie facută în mod
        descendent, ținând cont de modul de sortare de la opțiunea **-s**.  
        Dacă nu este prezentă, sortarea se va face ascendent.<br>
- **-h** sau **--help**:<br>
        Este afișat acest text, sau un alt text cu opțiunile de start.<br>

Notă:
----------
- Opțiunile **-m** și **-d** (echivalent **--min-size** și **--directoare**),<br>
sau<br>
**-M** și **-d** (echivalent **--max-size** și **--directoare**)<br>
**nu sunt compatibile și nu pot apare simultan**.
Dacă sunt prezente la lansare, va fi afișat un mesaj de eroare.<br>
- **-s** și **-d** (echivalent cu **--sortare** și **--directoare**) **nu sunt compatibile și nu pot apare simultan**.<br>
 Directoarele vor fi întotdeauna sortate doar după nume - ascendent sau descendent, depinzând de existența sau nu a opțiunii **-l** (echivalent **--list-desc**).<br>
- **-n** și **-r** (echivalent **--nume** și **--regex**) **nu sunt compatibile și nu pot apare simultan**.<br>
- Opțiunea **-n** (folosită pentru a căuta fișiere și / sau directoare după nume) poate include caracterele:<br>
      - '?' care caută orice caracter (doar de pe poziția respectivă);<br>
      - '\*' care caută orice caracter (începând cu poziția respectivă).<br>
Ex. pt d?nsa.txt: dansa.txt, dinsa.txt, densa.txt.<br>
Ex. pt d\*.txt:  toate fișierele care încep cu litera 'd' și au extensia 'txt'.<br>

## Exemple de folosire:

    cautafis -c /home/user -n tehnic
    cautafis --cale /home/user --nume tehnic
Vor fi căutate toate fișierele și directoarele care conțin șirul 'tehnic'
în nume, în directorul /home/user și în toate subdirectoarele acestuia.<br>

    cautafis -h
    cautafis --help
Este afișat un text cu opțiunile de start și caracteristicile programului.<br>

	cautafis -v
	cautafis --versiune
Este afișat un text cuprinzând numele și versiunea programului.

    cautafis -c /home/user -n tehnic -m 50
    cautafis --cale /home/user --nume tehnic --min-size 50
Vor fi căutate toate fișierele și directoarele care conțin șirul 'tehnic' în
nume, în directorul /home/user și în toate subdirectoarele acestuia și vor
fi afișate cele care au dimensiunea minimă de 50 Megabytes.<br>

    cautafis -c /home/user -n tehnic -M 50
    cautafis --cale /home/user --nume tehnic --max-size 50
Vor fi căutate toate fișierele și directoarele care conțin șirul 'tehnic' în
nume, în directorul /home/user și în toate subdirectoarele acestuia și vor
fi afișate cele care au dimensiunea maximă de 50 Megabytes.<br>

    cautafis -c /home/user -n tehnic -m 50 -M 51
    cautafis --cale /home/user --nume tehnic --min-size 50 --max-size 51
Vor fi căutate toate fișierele care conțin șirul 'tehnic' în nume, în directorul /home/user și în toate subdirectoarele acestuia și vor fi afișate cele care au dimensiunea minimă de 50 Megabytes și dimensiunea maximă de 51 Megabytes.<br>

    cautafis -c /home/user -n tehnic -d
    cautafis --cale /home/user --nume tehnic --directoare
Vor fi căutate toate directoarele care conțin șirul 'tehnic' în nume, în directorul /home/user și în toate subdirectoarele acestuia.<br>

    cautafis -c /home/user -n tehnic -d -a 5
    cautafis --cale /home/user --nume tehnic --directoare --adancime 5
Vor fi căutate toate directoarele care conțin șirul 'tehnic' în nume, în directorul /home/user și în toate subdirectoarele acestuia, pe o adăncime de 5 niveluri sub directorul /home/user.<br>

    cautafis -c /home/user -r d?nsa.t?t
    cautafis --cale /home/user --regex d?nsa.t?t
Vor fi căutate toate fișierele și directoarele al căror nume conține litera 't"pe prima poziție, pe a doua poziție poate conține orice caracter, iar restul de caractere din nume este 'ehnic', apoi extensia conține litera 't' pe prima și pe a treia poziție, iar pe a doua poziție poate fi orice caracter.<br>
Ex: dansa.txt, dinsa.txt, densa.txt, dansa.tmt.<br>
Căutarea se face în directorul /home/user și în toate subdirectoarele acestuia, respectând regulile regex.<br>

    cautafis -c /home/user -n tehnic -s s -l
    cautafis --cale /home/user --nume tehnic --sortare s --list-desc
Vor fi căutate toate fișierele și directoarele care conțin șirul 'tehnic' în nume, în directorul /home/user și în toate subdirectoarele acestuia, iar fișierele găsite vor fi sortate în funcție de dimensiune, descendent.<br>

