#ifndef __CAUTAFIS__RGX__H
#define __CAUTAFIS__RGX__H

#include "cautafis.h"

// Nr de înregistrări ale matricii de echivalențe joker - expresie Regex
#define NR_EXPR_REGEX_ECH   3
// Lungime maximă pentru expresia Regex
#define MAX_LUNG_REGEX 1024
// Nr maxim de nivele de nesting paranteze rotunde
#define MAX_NESTING_REGEX_ROT 4
// Nr maxim de nivele de nesting paranteze drepte
#define MAX_NESTING_REGEX_DREPT 1
// Nr maxim de nivele de nesting paranteze acolade
#define MAX_NESTING_REGEX_ACOLA 1

/* Obiectul tip sEchivalRegex este folosit în situația în care programul
 * primește opțiunea -n, iar numele fișierului conține caractere glob '?' sau '*'.
 * În acest caz căutarea nu se va mai face folosind funcția nFiltreazaDenumire,
 * din cautafis.c, ci numele de fișier va fi transformat în expresie regex,
 * iar căutarea se va face folosind funcția nFiltreazaDenumireRegex.
 * Transformarea în expresia regex presupune înlocuirea caracterelor '?' cu ".",
 * și '*' cu ".*".
 * Structura are două roluri:
 * - Primul, se face verificarea în funcția nVerificaExistJoker dacă vreunul
 * dintre caracterele numelui de fișier conține vreunul dintre caracterele
 * m_cJoker, cu condiția ca m_bVerificare să fie egal cu 1.
 * Sunt două caractere de verificat: '?' și '*'. Caracterul '.', al treilea
 * din obiectul tip sEchivalRegex, nu este considerat caracter glob,
 * de ex "fisier.txt"
 * - Al doilea rol, un obiect tip sEchivalRegex este scanat în căutarea m_cJoker
 * și furnizează m_pszExprEchRegex, expresia care va înlocui m_cJoker în
 * expresia finală Regex.
 *
 * Câmpul m_bVerificare:    dacă == 1, se verifică dacă apare în expresia glob;
 *                          dacă == 0, nu se verifică.
 *                          Ex: fisier.txt -> '.' nu e considerat caracter glob.
 */
struct sEchivalRegex{
    char m_cJoker;
    char* m_pszExprEchRegex;
    int m_bVerificare;
};
typedef struct sEchivalRegex EcRgx;

int nTransfNumeFisInExprRegex(const char*, char*);
int nVerificaExistJoker(const char*);
int nTransfGlobInRegex(const char*, char*);
char* pszStrCoresp(char);

int nRegexCaracterAdmis(char );
int nRegexValidareStricta(const char*);
int nRegexValidareExpresie(OptCLI*, const char*);
int nFiltreazaDenumireRegex(const char*);
void vDezalocaRegex(void);

#endif      //__CAUTAFIS__RGX__H
