#ifndef __CAUTAFIS__H
#define __CAUTAFIS__H

#include <sys/stat.h>

//#define DEBUG
#ifdef DEBUG
    #define FISIER_HELP "cautafis_hlp.txt"
#else
    #define FISIER_HELP "/usr/local/bin/cautafis_hlp.txt"
#endif

// Toate argumentele și toți parametrii programului sunt ok.
#define OPT_OK      0
//  A fost plasată opțiunea -h
#define OPT_HELP    2
// Eroare în argumente sau / si parametri
#define OPT_ERR     1

// Tipuri de fișiere
#define TIPFISIER_TOT   0
#define TIPFISIER_FIS   1
#define TIPFISIER_DIR   2

// Tipuri de sortări
#define TIPSORTARE_NUME         0
#define TIPSORTARE_DIMENSIUNE   1
#define TIPSORTARE_DATA         2

// Cod de eroare la alocare memorie pe heap, pt dezalocare cu grație ;-)
#define ERR_ALOCARE     -1

// Cod lansare direct al nOptSetRegex cu argumentul -r din linia de comandă
#define REGEX_DIRECT    0
// Cod lansare indirect al nOptSetRegex cu argumentul -n din linia de comandă,
// dar numele de fișier conține jokere => devine expresie Regex
#define REGEX_INDIRECT  1

/* Structuri globale */
//-----------------------------------------------------------------------------

/* Structura pentru valorile argumentelor si parametrilor
 * primiți de program la lansare - CLI
*/
struct sOptiuniCLI
{
    char* m_pszDirector;        // Directorul de unde începe căutarea.
    char* m_pszNumeFis;         // numele fișierului de căutat.
    int m_nIgnoreCase;          // 0 cu Case; 1 fără Case.
    int m_nTipFisier;           // 0 = toate; 1 = doar fișiere; 2 = doar directaore.
    int m_nAdancime;            // Nr de niveluri de căutare; -1 = tot.
    // int m_nRegex;               // 0 nu se folosește regex; 1 se folosește.
    char* m_pszExprRegex;       // Expresia regex folosită la căutare.
    long m_lDimMin;             // Dimensiunea minimă de verificat, în MegaByes.
    long m_lDimMax;             // Dimensiunea maximă de verificat, în MegaByes.
    int m_nTipSortare;          // 0 = Ascii; 1 = după dimensiune; 2 = după data ultimei modificări.
    int m_nListareDesc;         // 0 = Ascendent; 1 = Descendent.
};
typedef struct sOptiuniCLI OptCLI;
//-----------------------------------------------------------------------------

/* Structura pentru căutare directoarelor sCautaFis_X3:
 * sCautaFis_X3 conține un dublu pointer la o structură tip sDirector - m_pDirCaut,
 * un număr de directoare găsite și adăugate - m_nNrDirGasite și un număr de
 * directoare alocate - m_nNrAlocate. Acest câmp este folosit în momentul în care
 * este nevoie să se adauge un director nou: se face comparație între el și
 * m_nNrDirGasite și dacă este egal sau mai mare, se dublează și se face realloc
 * pentru **m_pDirCaut cu valoarea m_nNrAlocate * sizeof(struct sDirector).
 * Structura sDirector conține un câmp care conține numele directorului adăugat -
 * m_pszNumeDir și un câmp care conține nivelul acestui director față de directorul
 * de start - m_nNivel. Acesta este folosit pentru a limita numărul de nivele de
 * căutare, dacă a fost folosită opțiunea -a la lansarea programului
 */
struct sDirector{
    char* m_pszNumeDir;     // Numele directorului găsit
    int m_nNivel;           // Nivelul fața de directorul de start, primit ca argument de program
};
typedef struct sDirector DirCaut;

struct sCautaFis{
    DirCaut ** m_pDirCaut;
    int m_nNrDirGasite;      // Nr directorului curent găsit și adăugat; la final = Nr total de directoare găsite și adăugate.
    int m_nNrAlocate;       // Nr de directoare alocate. Este comparat cu numărul la care s-a ajuns cu directoarele noi, pentru o eventuală realloc.
};
typedef struct sCautaFis CFis;
//-----------------------------------------------------------------------------

/* Structura pentru listarea obiectelor de tip fișier și / sau directoarelor
 * sListareFis:
 * sListareFis conține un dublu pointer la o structură tip sListareFis - m_pLstFis,
 * un număr de obiecte și adăugate - m_nNrDirObAdaugate și un număr de obiecte
 * alocate - m_nNrAlocate. Acest câmp este folosit în momentul în care este nevoie
 * să se adauge un obiect nou: se face comparație între el și m_nNrObAdaugate și,
 * dacă este egal sau mai mare, se dublează și se face realloc pentru **m_pLstFis
 * cu valoarea m_nNrAlocate * sizeof(struct sListareFis).
 * Structura sListareFis conține un câmpa care conține numele obiectului adăugat -
 * m_pszNumeFis, și un câmp care conține tipul acestui obiect - TIPFISIER_TOT
 * pentru orice tip de obiect (fișier sau director), TIPFISIER_FIS, doar pentru
 * obiect tip fișier, sau TIPFISIER_DIR, doar pentru obiect tip director.
 * Câmpul m_lDimFis conține dimensiunea obiectului, numai și numai daca tipul
 * este TIPFISIER_FIS, câmpul m_tDataFis care conține data ultimei modificări a
 * obiectului.
  */
struct sListareFis
{
    char *m_pszNumeFis;                 // Numele fișierului / directorului admis la filtrare.
    int m_nTipFisier;                   // 0 = toate; 1 = fișier normal; 2 = director.
    off_t m_lDimFis;                    // Dimensiunea fișierului - doar dacă e de tip 0 sau 1.
    time_t m_tDataFis;                  // Data ultimei modificări a fișierului.
};
typedef struct sListareFis LstFis;

struct sVectorListare{
    LstFis** m_pLstFis;
    int m_nNrObAdaugate;                // Nr fișierului / directorului admis la filtrare și plasat în vectorul de listare.
    int m_nNrAlocate;                   // Nr de fișiere / directoare alocate. Este comparat cu numărul la care s-a ajuns cu fișierele / directoarele adăugate, pentru o eventuală realloc.
};
typedef struct sVectorListare VLst;

//-----------------------------------------------------------------------------

/* Prototipuri funcții */
int nAlocaTot(OptCLI**, CFis**, VLst**);
void vDezalocaTot(OptCLI**, CFis**, VLst**);

int nInitStructCLI(OptCLI**);
void vDezalocStructCLI(OptCLI**);

int nInitDirectoare(CFis**);
void vDezalocaDirectoare(CFis **);

int nCautaFis(OptCLI*, CFis*, VLst*);
int nAdaugaDirector(CFis*, char*, int);
void vListeazaVectorDirectoareDebug(CFis*);
int nParseazaLinieComanda(OptCLI*, int, char**);
int nValideazaOptiuni(int, int, int, int, int, int, int, int);
int nOptSetDirector(OptCLI*,char*);
int nOptSetNumeFis(OptCLI*,char*);
int nValidareNumeFis(char*);
void vOptSetTipFisier(OptCLI*, int);
int nOptSetAdancime(OptCLI*,char*);
int nOptSetDimensMinMax(OptCLI*, char*, int);
void vOptSetCase(OptCLI*);
int nOptSetRegex(OptCLI*, char*, int);
int nOptSetTipSortare(OptCLI*,char*);
void vOptSetListareDesc(OptCLI*);

void vAfisHelpMinimal(void);
int nAfisHelpFisier(void);

void vAfiseazaStructuraCLI(OptCLI*);

int nFiltreazaDenumire(OptCLI*, const char*, char*(*)(const char*, const char*));
int nFiltreazaTipFisier(OptCLI*, int);
int nFiltreazaDimensiune(OptCLI*, struct stat*, int);
int nFiltreazaFisier(OptCLI*, struct stat*, char*, int, char*(*)(const char*, const char*));

/* Partea de listare */
int nInitListare(VLst**);
void vDezalocListare(VLst**);
int nAdaugareFisier(VLst*, struct stat*, char*, int);
void vSorteazaVectorListare(OptCLI*, VLst*);
int nListSortNumeAsc(const void*, const void*);
int nListSortDimAsc(const void*, const void*);
int nListSortDataAsc(const void*, const void*);
int nListSortNumeDesc(const void*, const void*);
int nListSortDimDesc(const void*, const void*);
int nListSortDataDesc(const void*, const void*);
int nListeazaVectorListare(VLst*);

int main(int argc, char**argv);

#endif      //__CAUTAFIS__H
